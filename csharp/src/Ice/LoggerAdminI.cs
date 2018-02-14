// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace IceInternal
{
sealed class LoggerAdminI : Ice.LoggerAdminDisp_
{
    public override void
    attachRemoteLogger(Ice.RemoteLoggerPrx prx, Ice.LogMessageType[] messageTypes, string[] categories,
                       int messageMax, Ice.Current current)
    {
        if(prx == null)
        {
            return; // can't send this null RemoteLogger anything!
        }

        Ice.RemoteLoggerPrx remoteLogger = Ice.RemoteLoggerPrxHelper.uncheckedCast(prx.ice_twoway());

        Filters filters = new Filters(messageTypes, categories);
        LinkedList<Ice.LogMessage> initLogMessages = null;

        lock(this)
        {
            if(_sendLogCommunicator == null)
            {
                if(_destroyed)
                {
                    throw new Ice.ObjectNotExistException();
                }

                _sendLogCommunicator =
                    createSendLogCommunicator(current.adapter.getCommunicator(), _logger.getLocalLogger());
            }

            Ice.Identity remoteLoggerId = remoteLogger.ice_getIdentity();

            if(_remoteLoggerMap.ContainsKey(remoteLoggerId))
            {
                if(_traceLevel > 0)
                {
                    _logger.trace(_traceCategory, "rejecting `" + remoteLogger.ToString() +
                                 "' with RemoteLoggerAlreadyAttachedException");
                }

                throw new Ice.RemoteLoggerAlreadyAttachedException();
            }

            _remoteLoggerMap.Add(remoteLoggerId,
                                 new RemoteLoggerData(changeCommunicator(remoteLogger, _sendLogCommunicator), filters));

            if(messageMax != 0)
            {
                initLogMessages = new LinkedList<Ice.LogMessage>(_queue); // copy
            }
            else
            {
                initLogMessages = new LinkedList<Ice.LogMessage>();
            }
        }

        if(_traceLevel > 0)
        {
            _logger.trace(_traceCategory, "attached `" + remoteLogger.ToString() + "'");
        }

        if(initLogMessages.Count > 0)
        {
            filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }

        try
        {
            remoteLogger.initAsync(_logger.getPrefix(), initLogMessages.ToArray()).ContinueWith(
                (t) =>
                {
                    try
                    {
                        t.Wait();
                        if(_traceLevel > 1)
                        {
                            _logger.trace(_traceCategory,"init on `" + remoteLogger.ToString()
                                          + "' completed successfully");
                        }
                    }
                    catch(System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is Ice.LocalException);
                        deadRemoteLogger(remoteLogger, _logger, (Ice.LocalException)ae.InnerException, "init");
                    }
                });
        }
        catch(Ice.LocalException ex)
        {
            deadRemoteLogger(remoteLogger, _logger, ex, "init");
            throw;
        }
    }

    public override bool
    detachRemoteLogger(Ice.RemoteLoggerPrx remoteLogger, Ice.Current current)
    {
        if(remoteLogger == null)
        {
            return false;
        }

        //
        // No need to convert the proxy as we only use its identity
        //
        bool found = removeRemoteLogger(remoteLogger);

        if(_traceLevel > 0)
        {
            if(found)
            {
                _logger.trace(_traceCategory,  "detached `" + remoteLogger.ToString() + "'");
            }
            else
            {
                _logger.trace(_traceCategory, "cannot detach `" + remoteLogger.ToString() + "': not found");
            }
        }

        return found;
    }

    public override Ice.LogMessage[]
    getLog(Ice.LogMessageType[] messageTypes, string[] categories, int messageMax, out string prefix,
           Ice.Current current)
    {
        LinkedList<Ice.LogMessage> logMessages = null;
        lock(this)
        {
            if(messageMax != 0)
            {
                logMessages = new LinkedList<Ice.LogMessage>(_queue);
            }
            else
            {
                logMessages = new LinkedList<Ice.LogMessage>();
            }
        }

        prefix = _logger.getPrefix();

        if(logMessages.Count > 0)
        {
            Filters filters = new Filters(messageTypes, categories);
            filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }
        return logMessages.ToArray();
    }

    internal LoggerAdminI(Ice.Properties props, LoggerAdminLoggerI logger)
    {
        _maxLogCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepLogs", 100);
        _maxTraceCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepTraces", 100);
        _traceLevel = props.getPropertyAsInt("Ice.Trace.Admin.Logger");
        _logger = logger;
    }

    internal void destroy()
    {
        Ice.Communicator sendLogCommunicator = null;

        lock(this)
        {
            if(!_destroyed)
            {
                _destroyed = true;
                sendLogCommunicator = _sendLogCommunicator;
                _sendLogCommunicator = null;
            }
        }

        //
        // Destroy outside lock to avoid deadlock when there are outstanding two-way log calls sent to
        // remote logggers
        //
        if(sendLogCommunicator != null)
        {
            sendLogCommunicator.destroy();
        }
    }

    internal List<Ice.RemoteLoggerPrx> log(Ice.LogMessage logMessage)
    {
        lock(this)
        {
            List<Ice.RemoteLoggerPrx> remoteLoggers = null;

            //
            // Put message in _queue
            //
            if((logMessage.type != Ice.LogMessageType.TraceMessage && _maxLogCount > 0) ||
               (logMessage.type == Ice.LogMessageType.TraceMessage && _maxTraceCount > 0))
            {
                _queue.AddLast(logMessage);

                if(logMessage.type != Ice.LogMessageType.TraceMessage)
                {
                    Debug.Assert(_maxLogCount > 0);
                    if(_logCount == _maxLogCount)
                    {
                        //
                        // Need to remove the oldest log from the queue
                        //
                        Debug.Assert(_oldestLog != null);
                        var next = _oldestLog.Next;
                        _queue.Remove(_oldestLog);
                        _oldestLog = next;

                        while(_oldestLog != null && _oldestLog.Value.type == Ice.LogMessageType.TraceMessage)
                        {
                            _oldestLog = _oldestLog.Next;
                        }
                        Debug.Assert(_oldestLog != null); // remember: we just added a Log at the end
                    }
                    else
                    {
                        Debug.Assert(_logCount < _maxLogCount);
                        _logCount++;
                        if(_oldestLog == null)
                        {
                            _oldestLog = _queue.Last;
                        }
                    }
                }
                else
                {
                    Debug.Assert(_maxTraceCount > 0);
                    if(_traceCount == _maxTraceCount)
                    {
                        //
                        // Need to remove the oldest trace from the queue
                        //
                        Debug.Assert(_oldestTrace != null);
                        var next = _oldestTrace.Next;
                        _queue.Remove(_oldestTrace);
                        _oldestTrace = next;

                        while(_oldestTrace != null && _oldestTrace.Value.type != Ice.LogMessageType.TraceMessage)
                        {
                            _oldestTrace = _oldestTrace.Next;
                        }
                        Debug.Assert(_oldestTrace != null);  // remember: we just added a Log at the end
                    }
                    else
                    {
                        Debug.Assert(_traceCount < _maxTraceCount);
                        _traceCount++;
                        if(_oldestTrace == null)
                        {
                            _oldestTrace = _queue.Last;
                        }
                    }
                }
            }

            //
            // Queue updated, now find which remote loggers want this message
            //
            foreach(RemoteLoggerData p in _remoteLoggerMap.Values)
            {
                Filters filters = p.filters;

                if(filters.messageTypes.Count == 0 || filters.messageTypes.Contains(logMessage.type))
                {
                    if(logMessage.type != Ice.LogMessageType.TraceMessage || filters.traceCategories.Count == 0 ||
                       filters.traceCategories.Contains(logMessage.traceCategory))
                    {
                        if(remoteLoggers == null)
                        {
                            remoteLoggers = new List<Ice.RemoteLoggerPrx>();
                        }
                        remoteLoggers.Add(p.remoteLogger);
                    }
                }
            }

            return remoteLoggers;
        }
    }

    internal void deadRemoteLogger(Ice.RemoteLoggerPrx remoteLogger, Ice.Logger logger, Ice.LocalException ex,
                                   string operation)
    {
        //
        // No need to convert remoteLogger as we only use its identity
        //
        if(removeRemoteLogger(remoteLogger))
        {
            if(_traceLevel > 0)
            {
                logger.trace(_traceCategory,  "detached `" + remoteLogger.ToString() + "' because "
                             + operation + " raised:\n" + ex.ToString());
            }
        }
    }

    internal int getTraceLevel()
    {
        return _traceLevel;
    }

    private bool removeRemoteLogger(Ice.RemoteLoggerPrx remoteLogger)
    {
        lock(this)
        {
            return _remoteLoggerMap.Remove(remoteLogger.ice_getIdentity());
        }
    }

    private static void filterLogMessages(LinkedList<Ice.LogMessage> logMessages,
                                          HashSet<Ice.LogMessageType> messageTypes,
                                          HashSet<string> traceCategories, int messageMax)
    {
        Debug.Assert(logMessages.Count > 0 && messageMax != 0);

        //
        // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
        // that match the other filters, if any.
        //
        if(messageTypes.Count > 0 || traceCategories.Count > 0 || messageMax > 0)
        {
            int count = 0;
            var p = logMessages.Last;
            while(p != null)
            {
                bool keepIt = false;
                Ice.LogMessage msg = p.Value;
                if(messageTypes.Count == 0 || messageTypes.Contains(msg.type))
                {
                    if(msg.type != Ice.LogMessageType.TraceMessage || traceCategories.Count == 0 ||
                       traceCategories.Contains(msg.traceCategory))
                    {
                        keepIt = true;
                    }
                }

                if(keepIt)
                {
                    ++count;
                    if(messageMax > 0 && count >= messageMax)
                    {
                        // Remove all older messages
                        p = p.Previous;
                        while(p != null)
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
    private static Ice.RemoteLoggerPrx changeCommunicator(Ice.RemoteLoggerPrx prx, Ice.Communicator communicator)
    {
        if(prx == null)
        {
            return null;
        }

        Ice.ObjectPrx result = communicator.stringToProxy(prx.ToString());
        return Ice.RemoteLoggerPrxHelper.uncheckedCast(result.ice_invocationTimeout(prx.ice_getInvocationTimeout()));
    }

    private static void copyProperties(string prefix, Ice.Properties from, Ice.Properties to)
    {
        foreach(var p in from.getPropertiesForPrefix(prefix))
        {
            to.setProperty(p.Key, p.Value);
        }
    }

    private static Ice.Communicator createSendLogCommunicator(Ice.Communicator communicator, Ice.Logger logger)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.logger = logger;
        initData.properties = Ice.Util.createProperties();

        Ice.Properties mainProps = communicator.getProperties();

        copyProperties("Ice.Default.Locator", mainProps, initData.properties);
        copyProperties("Ice.Plugin.IceSSL", mainProps, initData.properties);
        copyProperties("IceSSL.", mainProps, initData.properties);

        string[] extraProps = mainProps.getPropertyAsList("Ice.Admin.Logger.Properties");

        if(extraProps.Length > 0)
        {
            for(int i = 0; i < extraProps.Length; ++i)
            {
                string p = extraProps[i];
                if(!p.StartsWith("--"))
                {
                    extraProps[i] = "--" + p;
                }
            }
            initData.properties.parseCommandLineOptions("", extraProps);
        }
        return Ice.Util.initialize(initData);
    }

    private readonly LinkedList<Ice.LogMessage> _queue = new LinkedList<Ice.LogMessage>();
    private int _logCount = 0; // non-trace messages
    private readonly int _maxLogCount;
    private int _traceCount = 0;
    private readonly int _maxTraceCount;
    private readonly int _traceLevel;

    private LinkedListNode<Ice.LogMessage> _oldestTrace = null;
    private LinkedListNode<Ice.LogMessage> _oldestLog = null;

    private class Filters
    {
        internal Filters(Ice.LogMessageType[] m, string[] c)
        {
            messageTypes = new HashSet<Ice.LogMessageType>(m);
            traceCategories = new HashSet<string>(c);
        }

        internal readonly HashSet<Ice.LogMessageType> messageTypes;
        internal readonly HashSet<string> traceCategories;
    }

    private class RemoteLoggerData
    {
        internal RemoteLoggerData(Ice.RemoteLoggerPrx prx, Filters f)
        {
            remoteLogger = prx;
            filters = f;
        }

        internal readonly Ice.RemoteLoggerPrx remoteLogger;
        internal readonly Filters filters;
    }

    private readonly Dictionary<Ice.Identity, RemoteLoggerData> _remoteLoggerMap
        = new Dictionary<Ice.Identity, RemoteLoggerData>();

    private readonly LoggerAdminLoggerI _logger;

    private Ice.Communicator _sendLogCommunicator = null;
    private bool _destroyed = false;
    static private readonly string _traceCategory = "Admin.Logger";
}

}
