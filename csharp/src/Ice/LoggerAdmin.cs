//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Linq;

namespace ZeroC.Ice
{
    internal sealed class RemoteLoggerQueue
    {
        private readonly LoggerAdmin _loggerAdmin;
        private readonly HashSet<LogMessageType> _messageTypes;
        private readonly IRemoteLoggerPrx _remoteLoggerPrx;
        private ValueTask _task;
        private readonly HashSet<string> _traceCategories;

        internal RemoteLoggerQueue(LoggerAdmin loggerAdmin, IRemoteLoggerPrx prx, HashSet<LogMessageType> messageTypes,
            HashSet<string> traceCategories)
        {
            // Use oneway proxy to send the remote logger invocations.
            _remoteLoggerPrx = prx.Clone(oneway: true);
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

        internal void Queue(Func<IRemoteLoggerPrx, ValueTask> func, ILogger logger, string op)
        {
            lock (this)
            {
                _task = PerformAsync(func, logger, op);
            }
        }

        private async ValueTask PerformAsync(Func<IRemoteLoggerPrx, ValueTask> func, ILogger logger, string op)
        {
            try
            {
                // Wait for the previous invocation to be sent.
                await _task;

                // Now send the given invocation.
                await func(_remoteLoggerPrx);
            }
            catch (Exception ex)
            {
                _loggerAdmin.DeadRemoteLogger(_remoteLoggerPrx, logger, ex, op);
                throw;
            }
        }
    }

    internal sealed class LoggerAdmin : ILoggerAdmin
    {
        private const string TraceCategory = "Admin.Logger";
        private bool _destroyed = false;
        private int _logCount = 0; // non-trace messages
        private readonly LoggerAdminLogger _logger;
        private readonly int _maxLogCount;
        private readonly int _maxTraceCount;
        private LinkedListNode<LogMessage>? _oldestTrace = null;
        private LinkedListNode<LogMessage>? _oldestLog = null;
        private readonly LinkedList<LogMessage> _queue = new LinkedList<LogMessage>();
        private readonly Dictionary<Identity, RemoteLoggerQueue> _remoteLoggerMap
            = new Dictionary<Identity, RemoteLoggerQueue>();
        private Communicator? _sendLogCommunicator = null;
        private int _traceCount = 0;
        private readonly int _traceLevel;

        public void AttachRemoteLogger(IRemoteLoggerPrx? prx, LogMessageType[] types, string[] categories,
            int messageMax, Current current)
        {
            if (prx == null)
            {
                return; // can't send this null RemoteLogger anything!
            }

            var messageTypes = new HashSet<LogMessageType>(types);
            var traceCategories = new HashSet<string>(categories);

            LinkedList<LogMessage>? initLogMessages = null;
            RemoteLoggerQueue remoteLogger;
            lock (this)
            {
                if (_sendLogCommunicator == null)
                {
                    if (_destroyed)
                    {
                        throw new ObjectNotExistException(current);
                    }

                    _sendLogCommunicator =
                        CreateSendLogCommunicator(current.Adapter.Communicator, _logger.GetLocalLogger());
                }

                Identity remoteLoggerId = prx.Identity;

                if (_remoteLoggerMap.ContainsKey(remoteLoggerId))
                {
                    if (_traceLevel > 0)
                    {
                        _logger.Trace(TraceCategory, @$"rejecting `{prx}' with RemoteLoggerAlreadyAttachedException");
                    }

                    throw new RemoteLoggerAlreadyAttachedException();
                }

                remoteLogger = new RemoteLoggerQueue(this, ChangeCommunicator(prx, _sendLogCommunicator), messageTypes,
                    traceCategories);
                _remoteLoggerMap.Add(remoteLoggerId, remoteLogger);

                if (messageMax != 0)
                {
                    initLogMessages = new LinkedList<LogMessage>(_queue); // copy
                }
                else
                {
                    initLogMessages = new LinkedList<LogMessage>();
                }
            }

            if (_traceLevel > 0)
            {
                _logger.Trace(TraceCategory, $"attached `{remoteLogger}'");
            }

            if (initLogMessages.Count > 0)
            {
                FilterLogMessages(initLogMessages, messageTypes, traceCategories, messageMax);
            }

            remoteLogger.Queue(async prx =>
            {
                await prx.InitAsync(_logger.GetPrefix(), initLogMessages.ToArray());
                if (_traceLevel > 1)
                {
                    _logger.Trace(TraceCategory, $"init on `{remoteLogger}' completed successfully");
                }
            }, _logger, "init");
        }

        public bool DetachRemoteLogger(IRemoteLoggerPrx? remoteLogger, Current current)
        {
            if (remoteLogger == null)
            {
                return false;
            }

            //
            // No need to convert the proxy as we only use its identity
            //
            bool found = RemoveRemoteLogger(remoteLogger);

            if (_traceLevel > 0)
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

            return found;
        }

        public (IEnumerable<LogMessage>, string) GetLog(LogMessageType[] types, string[] categories, int messageMax,
            Current current)
        {
            LinkedList<LogMessage> logMessages;
            lock (this)
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

            string prefix = _logger.GetPrefix();

            if (logMessages.Count > 0)
            {
                var messageTypes = new HashSet<LogMessageType>(types);
                var traceCategories = new HashSet<string>(categories);
                FilterLogMessages(logMessages, messageTypes, traceCategories, messageMax);
            }
            return (logMessages.ToArray(), prefix);
        }

        internal LoggerAdmin(Communicator communicator, LoggerAdminLogger logger)
        {
            _maxLogCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepLogs") ?? 100;
            _maxTraceCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepTraces") ?? 100;
            _traceLevel = communicator.GetPropertyAsInt("Ice.Trace.Admin.Logger") ?? 0;
            _logger = logger;
        }

        internal void Destroy()
        {
            Communicator? sendLogCommunicator = null;

            lock (this)
            {
                if (!_destroyed)
                {
                    _destroyed = true;
                    sendLogCommunicator = _sendLogCommunicator;
                    _sendLogCommunicator = null;
                }
            }

            // Destroy outside lock to avoid deadlock when there are outstanding calls sent to remote loggers
            if (sendLogCommunicator != null)
            {
                sendLogCommunicator.Destroy();
            }
        }

        internal void DeadRemoteLogger(IRemoteLoggerPrx remoteLogger, ILogger logger, Exception ex, string operation)
        {
            // No need to convert remoteLogger as we only use its identity
            if (RemoveRemoteLogger(remoteLogger))
            {
                if (!(ex is CommunicatorDestroyedException))
                {
                    if (_traceLevel > 0)
                    {
                        logger.Trace(TraceCategory, $"detached `{remoteLogger}' because {operation} raised:\n{ex}");
                    }
                }
            }
        }

        internal int GetTraceLevel() => _traceLevel;

        internal List<RemoteLoggerQueue>? Log(LogMessage logMessage)
        {
            lock (this)
            {
                List<RemoteLoggerQueue>? remoteLoggers = null;

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
                foreach (RemoteLoggerQueue p in _remoteLoggerMap.Values)
                {
                    if (p.IsAccepted(logMessage))
                    {
                        remoteLoggers ??= new List<RemoteLoggerQueue>();
                        remoteLoggers.Add(p);
                    }
                }

                return remoteLoggers;
            }
        }

        // Change this proxy's communicator
        private static IRemoteLoggerPrx ChangeCommunicator(IRemoteLoggerPrx prx, Communicator communicator) =>
            IRemoteLoggerPrx.Parse(prx.ToString()!, communicator);

        private static Communicator CreateSendLogCommunicator(Communicator communicator, ILogger logger)
        {
            var properties = communicator.GetProperties().Where(
                p => p.Key == "Ice.Default.Locator" ||
                p.Key == "Ice.Plugin.IceSSL" ||
                p.Key.StartsWith("IceSSL.")).ToDictionary(p => p.Key, p => p.Value);

            string[] args = communicator.GetPropertyAsList("Ice.Admin.Logger.Properties")?.Select(
                v => v.StartsWith("--") ? v : $"--{v}").ToArray() ?? Array.Empty<string>();
            return new Communicator(ref args, properties, logger: logger);
        }

        private static void FilterLogMessages(LinkedList<LogMessage> logMessages,
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

        private bool RemoveRemoteLogger(IRemoteLoggerPrx remoteLogger)
        {
            lock (this)
            {
                return _remoteLoggerMap.Remove(remoteLogger.Identity);
            }
        }
    }
}
