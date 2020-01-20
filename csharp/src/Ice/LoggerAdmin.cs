//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Ice;

namespace IceInternal
{
    internal sealed class LoggerAdmin : ILoggerAdmin
    {
        public void
        AttachRemoteLogger(IRemoteLoggerPrx prx, LogMessageType[] messageTypes, string[] categories,
                           int messageMax, Current current)
        {
            if (prx == null)
            {
                return; // can't send this null RemoteLogger anything!
            }

            var remoteLogger = prx.Clone(oneway: false);

            Filters filters = new Filters(messageTypes, categories);
            LinkedList<LogMessage>? initLogMessages = null;

            lock (this)
            {
                if (_sendLogCommunicator == null)
                {
                    if (_destroyed)
                    {
                        throw new ObjectNotExistException();
                    }

                    _sendLogCommunicator =
                        createSendLogCommunicator(current.Adapter.Communicator, _logger.getLocalLogger());
                }

                Identity remoteLoggerId = remoteLogger.Identity;

                if (_remoteLoggerMap.ContainsKey(remoteLoggerId))
                {
                    if (_traceLevel > 0)
                    {
                        _logger.trace(_traceCategory, "rejecting `" + remoteLogger.ToString() +
                                     "' with RemoteLoggerAlreadyAttachedException");
                    }

                    throw new RemoteLoggerAlreadyAttachedException();
                }

                _remoteLoggerMap.Add(remoteLoggerId,
                                     new RemoteLoggerData(changeCommunicator(remoteLogger, _sendLogCommunicator), filters));

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
                _logger.trace(_traceCategory, "attached `" + remoteLogger.ToString() + "'");
            }

            if (initLogMessages.Count > 0)
            {
                filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
            }

            try
            {
                remoteLogger.InitAsync(_logger.getPrefix(), initLogMessages.ToArray()).ContinueWith(
                    (t) =>
                    {
                        try
                        {
                            t.Wait();
                            if (_traceLevel > 1)
                            {
                                _logger.trace(_traceCategory, "init on `" + remoteLogger.ToString()
                                              + "' completed successfully");
                            }
                        }
                        catch (AggregateException ae)
                        {
                            Debug.Assert(ae.InnerException is LocalException);
                            deadRemoteLogger(remoteLogger, _logger, (LocalException)ae.InnerException, "init");
                        }
                    },
                    System.Threading.Tasks.TaskScheduler.Current);
            }
            catch (LocalException ex)
            {
                deadRemoteLogger(remoteLogger, _logger, ex, "init");
                throw;
            }
        }

        public bool
        DetachRemoteLogger(IRemoteLoggerPrx remoteLogger, Current current)
        {
            if (remoteLogger == null)
            {
                return false;
            }

            //
            // No need to convert the proxy as we only use its identity
            //
            bool found = removeRemoteLogger(remoteLogger);

            if (_traceLevel > 0)
            {
                if (found)
                {
                    _logger.trace(_traceCategory, "detached `" + remoteLogger.ToString() + "'");
                }
                else
                {
                    _logger.trace(_traceCategory, "cannot detach `" + remoteLogger.ToString() + "': not found");
                }
            }

            return found;
        }

        public ILoggerAdmin.GetLogReturnValue
        GetLog(LogMessageType[] messageTypes, string[] categories, int messageMax, Current current)
        {
            LinkedList<Ice.LogMessage> logMessages;
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

            var prefix = _logger.getPrefix();

            if (logMessages.Count > 0)
            {
                Filters filters = new Filters(messageTypes, categories);
                filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
            }
            return new ILoggerAdmin.GetLogReturnValue(logMessages.ToArray(), prefix);
        }

        internal LoggerAdmin(Communicator communicator, LoggerAdminLogger logger)
        {
            _maxLogCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepLogs") ?? 100;
            _maxTraceCount = communicator.GetPropertyAsInt("Ice.Admin.Logger.KeepTraces") ?? 100;
            _traceLevel = communicator.GetPropertyAsInt("Ice.Trace.Admin.Logger") ?? 0;
            _logger = logger;
        }

        internal void destroy()
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

            //
            // Destroy outside lock to avoid deadlock when there are outstanding two-way log calls sent to
            // remote loggers
            //
            if (sendLogCommunicator != null)
            {
                sendLogCommunicator.destroy();
            }
        }

        internal List<IRemoteLoggerPrx>? log(LogMessage logMessage)
        {
            lock (this)
            {
                List<IRemoteLoggerPrx>? remoteLoggers = null;

                //
                // Put message in _queue
                //
                if ((logMessage.Type != LogMessageType.TraceMessage && _maxLogCount > 0) ||
                    (logMessage.Type == LogMessageType.TraceMessage && _maxTraceCount > 0))
                {
                    _queue.AddLast(logMessage);

                    if (logMessage.Type != LogMessageType.TraceMessage)
                    {
                        Debug.Assert(_maxLogCount > 0);
                        if (_logCount == _maxLogCount)
                        {
                            //
                            // Need to remove the oldest log from the queue
                            //
                            Debug.Assert(_oldestLog != null);
                            var next = _oldestLog.Next;
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
                            //
                            // Need to remove the oldest trace from the queue
                            //
                            Debug.Assert(_oldestTrace != null);
                            var next = _oldestTrace.Next;
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

                //
                // Queue updated, now find which remote loggers want this message
                //
                foreach (RemoteLoggerData p in _remoteLoggerMap.Values)
                {
                    Filters filters = p.filters;

                    if (filters.messageTypes.Count == 0 || filters.messageTypes.Contains(logMessage.Type))
                    {
                        if (logMessage.Type != LogMessageType.TraceMessage || filters.traceCategories.Count == 0 ||
                           filters.traceCategories.Contains(logMessage.TraceCategory))
                        {
                            if (remoteLoggers == null)
                            {
                                remoteLoggers = new List<IRemoteLoggerPrx>();
                            }
                            remoteLoggers.Add(p.remoteLogger);
                        }
                    }
                }

                return remoteLoggers;
            }
        }

        internal void deadRemoteLogger(IRemoteLoggerPrx remoteLogger, ILogger logger, LocalException ex,
                                       string operation)
        {
            //
            // No need to convert remoteLogger as we only use its identity
            //
            if (removeRemoteLogger(remoteLogger))
            {
                if (_traceLevel > 0)
                {
                    logger.trace(_traceCategory, "detached `" + remoteLogger.ToString() + "' because "
                                 + operation + " raised:\n" + ex.ToString());
                }
            }
        }

        internal int getTraceLevel()
        {
            return _traceLevel;
        }

        private bool removeRemoteLogger(IRemoteLoggerPrx remoteLogger)
        {
            lock (this)
            {
                return _remoteLoggerMap.Remove(remoteLogger.Identity);
            }
        }

        private static void filterLogMessages(LinkedList<LogMessage> logMessages,
                                              HashSet<LogMessageType> messageTypes,
                                              HashSet<string> traceCategories, int messageMax)
        {
            Debug.Assert(logMessages.Count > 0 && messageMax != 0);

            //
            // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
            // that match the other filters, if any.
            //
            if (messageTypes.Count > 0 || traceCategories.Count > 0 || messageMax > 0)
            {
                int count = 0;
                var p = logMessages.Last;
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
                                var previous = p.Previous;
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
                        var previous = p.Previous;
                        logMessages.Remove(p);
                        p = previous;
                    }
                }
            }
            // else, don't need any filtering
        }

        //
        // Change this proxy's communicator, while keeping its invocation timeout
        //
        private static IRemoteLoggerPrx changeCommunicator(IRemoteLoggerPrx prx, Communicator communicator)
        {
            return IRemoteLoggerPrx.Parse(prx.ToString(), communicator).Clone(invocationTimeout: prx.InvocationTimeout);
        }

        private static Communicator createSendLogCommunicator(Communicator communicator, ILogger logger)
        {
            Dictionary<string, string> properties = communicator.GetProperties().Where(
                p => p.Key == "Ice.Default.Locator" || p.Key == "Ice.Plugin.IceSSL" || p.Key.StartsWith("IceSSL.")
            ).ToDictionary(p => p.Key, p => p.Value);

            string[] args = communicator.GetPropertyAsList("Ice.Admin.Logger.Properties")?.Select(
                v => v.StartsWith("--") ? v : $"--{v}").ToArray() ?? Array.Empty<string>();
            return new Communicator(ref args, properties, logger: logger);
        }

        private readonly LinkedList<LogMessage> _queue = new LinkedList<LogMessage>();
        private int _logCount = 0; // non-trace messages
        private readonly int _maxLogCount;
        private int _traceCount = 0;
        private readonly int _maxTraceCount;
        private readonly int _traceLevel;

        private LinkedListNode<LogMessage>? _oldestTrace = null;
        private LinkedListNode<LogMessage>? _oldestLog = null;

        private class Filters
        {
            internal Filters(LogMessageType[] m, string[] c)
            {
                messageTypes = new HashSet<LogMessageType>(m);
                traceCategories = new HashSet<string>(c);
            }

            internal readonly HashSet<LogMessageType> messageTypes;
            internal readonly HashSet<string> traceCategories;
        }

        private class RemoteLoggerData
        {
            internal RemoteLoggerData(IRemoteLoggerPrx prx, Filters f)
            {
                remoteLogger = prx;
                filters = f;
            }

            internal readonly IRemoteLoggerPrx remoteLogger;
            internal readonly Filters filters;
        }

        private readonly Dictionary<Identity, RemoteLoggerData> _remoteLoggerMap
            = new Dictionary<Identity, RemoteLoggerData>();

        private readonly LoggerAdminLogger _logger;

        private Communicator? _sendLogCommunicator = null;
        private bool _destroyed = false;
        private const string _traceCategory = "Admin.Logger";
    }

}
