// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class LogForwarder
    {
        private readonly LoggerAdmin _loggerAdmin;
        private readonly HashSet<LogMessageType> _messageTypes;
        private readonly object _mutex = new object();
        private readonly IRemoteLoggerPrx _remoteLoggerPrx;
        private Task _lastRequestTask = Task.CompletedTask;
        private readonly HashSet<string> _traceCategories;

        internal LogForwarder(
            LoggerAdmin loggerAdmin,
            IRemoteLoggerPrx prx,
            HashSet<LogMessageType> messageTypes,
            HashSet<string> traceCategories)
        {
            _remoteLoggerPrx = prx;
            _loggerAdmin = loggerAdmin;
            _messageTypes = messageTypes;
            _traceCategories = traceCategories;
        }

        internal bool IsAccepted(LogMessage logMessage)
        {
            if (_messageTypes.Count == 0 || _messageTypes.Contains(logMessage.Type))
            {
                if (logMessage.Type != LogMessageType.TraceMessage || _traceCategories.Count == 0 ||
                   _traceCategories.Contains(logMessage.TraceCategory))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>Queues an asynchronous request to the remote logger, and ensures this request is sent in the order
        /// it is enqueued.</summary>
        /// <param name="operation">The name of the operation called on the remote logger.</param>
        /// <param name="localLogger">The local logger.</param>
        /// <param name="request">A delegate that represents an asynchronous request to be called on the remote
        /// logger.</param>
        internal void Queue(string operation, ILogger localLogger, Func<IRemoteLoggerPrx, Task> request)
        {
            lock (_mutex)
            {
                // Creates a new task that waits for the last request task to complete and to send the given
                // request. Save this new task as the last request task.
                _lastRequestTask = QueueAsync(_lastRequestTask, operation, localLogger, request);
            }

            async Task QueueAsync(
                Task previousRequestTask,
                string operation,
                ILogger localLogger,
                Func<IRemoteLoggerPrx, Task> request)
            {
                try
                {
                    // Wait for the previous request task to complete.
                    await previousRequestTask.ConfigureAwait(false);

                    if (_loggerAdmin.TraceLevel > 1)
                    {
                        localLogger.Trace(LoggerAdmin.TraceCategory,
                            $"sending {operation} message to `{_remoteLoggerPrx}'");
                    }

                    // Now send the given request. The IRemoteLogger requests are marked as [oneway] in the Slice so
                    // this should return once the request has been sent.
                    await request(_remoteLoggerPrx).ConfigureAwait(false);

                    if (_loggerAdmin.TraceLevel > 1)
                    {
                        localLogger.Trace(LoggerAdmin.TraceCategory,
                            $"{operation} on `{_remoteLoggerPrx}' sent successfully");
                    }
                }
                catch (Exception exception)
                {
                    _loggerAdmin.DeadRemoteLogger(_remoteLoggerPrx, localLogger, exception, operation);
                    throw;
                }
            }
        }
    }

    internal sealed class LoggerAdmin : IAsyncLoggerAdmin, IAsyncDisposable
    {
        internal int TraceLevel { get; }
        internal const string TraceCategory = "Admin.Logger";
        private bool _destroyed;
        private int _logCount; // non-trace messages
        private readonly LoggerAdminLogger _logger;
        private readonly int _maxLogCount;
        private readonly int _maxTraceCount;
        private readonly object _mutex = new object();
        private LinkedListNode<LogMessage>? _oldestTrace;
        private LinkedListNode<LogMessage>? _oldestLog;
        private readonly LinkedList<LogMessage> _queue = new LinkedList<LogMessage>();
        private readonly Dictionary<Identity, LogForwarder> _logForwarderMap = new Dictionary<Identity, LogForwarder>();
        private Communicator? _sendLogCommunicator;
        private int _traceCount;

        public ValueTask AttachRemoteLoggerAsync(
            IRemoteLoggerPrx? prx,
            LogMessageType[] types,
            string[] categories,
            int messageMax,
            Current current,
            CancellationToken cancel)
        {
            if (prx == null)
            {
                return default; // can't send this null RemoteLogger anything!
            }

            var messageTypes = new HashSet<LogMessageType>(types);
            var traceCategories = new HashSet<string>(categories);

            LinkedList<LogMessage>? initLogMessages = null;
            LogForwarder logForwarder;
            lock (_mutex)
            {
                if (_destroyed)
                {
                    throw new ObjectNotExistException();
                }

                _sendLogCommunicator ??= CreateSendLogCommunicator(current.Communicator, _logger.LocalLogger);

                Identity remoteLoggerId = prx.Identity;

                if (_logForwarderMap.ContainsKey(remoteLoggerId))
                {
                    if (TraceLevel > 0)
                    {
                        _logger.Trace(TraceCategory, @$"rejecting `{prx}' with RemoteLoggerAlreadyAttachedException");
                    }

                    throw new RemoteLoggerAlreadyAttachedException();
                }

                logForwarder = new LogForwarder(
                    this,
                    ChangeCommunicator(prx, _sendLogCommunicator),
                    messageTypes,
                    traceCategories);
                _logForwarderMap.Add(remoteLoggerId, logForwarder);

                if (messageMax != 0)
                {
                    initLogMessages = new LinkedList<LogMessage>(_queue); // copy
                }
                else
                {
                    initLogMessages = new LinkedList<LogMessage>();
                }
            }

            if (TraceLevel > 0)
            {
                _logger.Trace(TraceCategory, $"attached `{prx}'");
            }

            if (initLogMessages.Count > 0)
            {
                FilterLogMessages(initLogMessages, messageTypes, traceCategories, messageMax);
            }

            logForwarder.Queue("init", _logger, prx => prx.InitAsync(_logger.Prefix, initLogMessages.ToArray()));
            return default;
        }

        public ValueTask<bool> DetachRemoteLoggerAsync(
            IRemoteLoggerPrx? remoteLogger,
            Current current,
            CancellationToken cancel)
        {
            if (remoteLogger == null)
            {
                return new(false);
            }

            bool found = RemoveLogForwarder(remoteLogger.Identity);

            if (TraceLevel > 0)
            {
                if (found)
                {
                    _logger.Trace(TraceCategory, $"detached `{remoteLogger}'");
                }
                else
                {
                    _logger.Trace(TraceCategory, $"cannot detach `{remoteLogger}': not found");
                }
            }

            return new(found);
        }

        public ValueTask<(IEnumerable<LogMessage>, string)> GetLogAsync(
            LogMessageType[] types,
            string[] categories,
            int messageMax,
            Current current,
            CancellationToken cancel)
        {
            LinkedList<LogMessage> logMessages;
            lock (_mutex)
            {
                if (messageMax != 0)
                {
                    logMessages = new LinkedList<LogMessage>(_queue);
                }
                else
                {
                    logMessages = new LinkedList<LogMessage>();
                }
            }

            if (logMessages.Count > 0)
            {
                var messageTypes = new HashSet<LogMessageType>(types);
                var traceCategories = new HashSet<string>(categories);
                FilterLogMessages(logMessages, messageTypes, traceCategories, messageMax);
            }
            return new((logMessages.ToArray(), _logger.Prefix));
        }

        internal LoggerAdmin(Communicator communicator, LoggerAdminLogger logger)
        {
            _maxLogCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepLogs") ?? 100;
            _maxTraceCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepTraces") ?? 100;
            TraceLevel = communicator.GetPropertyAsInt("Ice.Trace.Admin.Logger") ?? 0;
            _logger = logger;
        }

        public async ValueTask DisposeAsync()
        {
            lock (_mutex)
            {
                if (_destroyed)
                {
                    return;
                }
                _destroyed = true;
            }

            // Destroy outside lock to avoid deadlock when there are outstanding calls sent to remote loggers
            if (_sendLogCommunicator != null)
            {
                await _sendLogCommunicator.DisposeAsync().ConfigureAwait(false);
            }
        }

        internal void DeadRemoteLogger(IRemoteLoggerPrx remoteLogger, ILogger logger, Exception ex, string operation)
        {
            if (RemoveLogForwarder(remoteLogger.Identity))
            {
                if (!(ex is CommunicatorDisposedException))
                {
                    if (TraceLevel > 0)
                    {
                        logger.Trace(TraceCategory, $"detached `{remoteLogger}' because {operation} raised:\n{ex}");
                    }
                }
            }
        }

        internal List<LogForwarder>? Log(LogMessage logMessage)
        {
            lock (_mutex)
            {
                List<LogForwarder>? logForwarderList = null;

                // Put message in _queue
                if ((logMessage.Type != LogMessageType.TraceMessage && _maxLogCount > 0) ||
                    (logMessage.Type == LogMessageType.TraceMessage && _maxTraceCount > 0))
                {
                    _queue.AddLast(logMessage);

                    if (logMessage.Type != LogMessageType.TraceMessage)
                    {
                        Debug.Assert(_maxLogCount > 0);
                        if (_logCount == _maxLogCount)
                        {
                            // Need to remove the oldest log from the queue
                            Debug.Assert(_oldestLog != null);
                            LinkedListNode<LogMessage>? next = _oldestLog.Next;
                            _queue.Remove(_oldestLog);
                            _oldestLog = next;

                            while (_oldestLog != null && _oldestLog.Value.Type == LogMessageType.TraceMessage)
                            {
                                _oldestLog = _oldestLog.Next;
                            }
                            Debug.Assert(_oldestLog != null); // remember: we just added a Log at the end
                        }
                        else
                        {
                            Debug.Assert(_logCount < _maxLogCount);
                            _logCount++;
                            if (_oldestLog == null)
                            {
                                _oldestLog = _queue.Last;
                            }
                        }
                    }
                    else
                    {
                        Debug.Assert(_maxTraceCount > 0);
                        if (_traceCount == _maxTraceCount)
                        {
                            // Need to remove the oldest trace from the queue
                            Debug.Assert(_oldestTrace != null);
                            LinkedListNode<LogMessage>? next = _oldestTrace.Next;
                            _queue.Remove(_oldestTrace);
                            _oldestTrace = next;

                            while (_oldestTrace != null && _oldestTrace.Value.Type != LogMessageType.TraceMessage)
                            {
                                _oldestTrace = _oldestTrace.Next;
                            }
                            Debug.Assert(_oldestTrace != null);  // remember: we just added a Log at the end
                        }
                        else
                        {
                            Debug.Assert(_traceCount < _maxTraceCount);
                            _traceCount++;
                            if (_oldestTrace == null)
                            {
                                _oldestTrace = _queue.Last;
                            }
                        }
                    }
                }

                // Queue updated, now find which remote loggers want this message
                foreach (LogForwarder p in _logForwarderMap.Values)
                {
                    if (p.IsAccepted(logMessage))
                    {
                        logForwarderList ??= new List<LogForwarder>();
                        logForwarderList.Add(p);
                    }
                }

                return logForwarderList;
            }
        }

        // Change this proxy's communicator
        private static IRemoteLoggerPrx ChangeCommunicator(IRemoteLoggerPrx prx, Communicator communicator) =>
            IRemoteLoggerPrx.Parse(prx.ToString()!, communicator);

        private static Communicator CreateSendLogCommunicator(Communicator communicator, ILogger logger)
        {
            var properties = communicator.GetProperties().Where(p =>
                p.Key == "Ice.Default.Locator" ||
                p.Key.StartsWith("IceSSL.", StringComparison.InvariantCulture)).ToDictionary(p => p.Key, p => p.Value);

            string[] args = communicator.GetPropertyAsList("Ice.Admin.Logger.Properties")?.Select(
                v => v.StartsWith("--", StringComparison.InvariantCulture) ?
                    v : $"--{v}").ToArray() ?? Array.Empty<string>();
            return new Communicator(ref args, properties, logger: logger);
        }

        private static void FilterLogMessages(
            LinkedList<LogMessage> logMessages,
            HashSet<LogMessageType> messageTypes,
            HashSet<string> traceCategories,
            int messageMax)
        {
            Debug.Assert(logMessages.Count > 0 && messageMax != 0);

            // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
            // that match the other filters, if any.
            if (messageTypes.Count > 0 || traceCategories.Count > 0 || messageMax > 0)
            {
                int count = 0;
                LinkedListNode<LogMessage>? p = logMessages.Last;
                while (p != null)
                {
                    bool keepIt = false;
                    LogMessage msg = p.Value;
                    if (messageTypes.Count == 0 || messageTypes.Contains(msg.Type))
                    {
                        if (msg.Type != LogMessageType.TraceMessage || traceCategories.Count == 0 ||
                           traceCategories.Contains(msg.TraceCategory))
                        {
                            keepIt = true;
                        }
                    }

                    if (keepIt)
                    {
                        ++count;
                        if (messageMax > 0 && count >= messageMax)
                        {
                            // Remove all older messages
                            p = p.Previous;
                            while (p != null)
                            {
                                LinkedListNode<LogMessage>? previous = p.Previous;
                                logMessages.Remove(p);
                                p = previous;
                            }
                            break; // while
                        }
                        else
                        {
                            p = p.Previous;
                        }
                    }
                    else
                    {
                        LinkedListNode<LogMessage>? previous = p.Previous;
                        logMessages.Remove(p);
                        p = previous;
                    }
                }
            }
            // else, don't need any filtering
        }

        private bool RemoveLogForwarder(Identity remoteLogger)
        {
            lock (_mutex)
            {
                return _logForwarderMap.Remove(remoteLogger);
            }
        }
    }
}
