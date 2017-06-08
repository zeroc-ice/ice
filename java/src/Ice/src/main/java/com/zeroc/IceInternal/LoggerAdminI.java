// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.LogMessage;
import com.zeroc.Ice.LogMessageType;
import com.zeroc.Ice.RemoteLoggerPrx;

final class LoggerAdminI implements com.zeroc.Ice.LoggerAdmin
{
    @Override
    public void attachRemoteLogger(RemoteLoggerPrx prx, LogMessageType[] messageTypes,
                                   String[] categories, int messageMax, com.zeroc.Ice.Current current)
        throws com.zeroc.Ice.RemoteLoggerAlreadyAttachedException
    {
        if(prx == null)
        {
            return; // can't send this null RemoteLogger anything!
        }

        RemoteLoggerPrx remoteLogger = RemoteLoggerPrx.uncheckedCast(prx.ice_twoway());

        Filters filters = new Filters(messageTypes, categories);
        java.util.List<LogMessage> initLogMessages = null;

        synchronized(this)
        {
            if(_sendLogCommunicator == null)
            {
                if(_destroyed)
                {
                    throw new com.zeroc.Ice.ObjectNotExistException();
                }

                _sendLogCommunicator =
                    createSendLogCommunicator(current.adapter.getCommunicator(), _logger.getLocalLogger());
            }

            com.zeroc.Ice.Identity remoteLoggerId = remoteLogger.ice_getIdentity();

            if(_remoteLoggerMap.containsKey(remoteLoggerId))
            {
                if(_traceLevel > 0)
                {
                    _logger.trace(_traceCategory, "rejecting `" + remoteLogger.toString() +
                                                  "' with RemoteLoggerAlreadyAttachedException");
                }

                throw new com.zeroc.Ice.RemoteLoggerAlreadyAttachedException();
            }

            _remoteLoggerMap.put(remoteLoggerId,
                                 new RemoteLoggerData(changeCommunicator(remoteLogger, _sendLogCommunicator), filters));

            if(messageMax != 0)
            {
                initLogMessages = new java.util.LinkedList<>(_queue); // copy
            }
            else
            {
                initLogMessages = new java.util.LinkedList<>();
            }
        }

        if(_traceLevel > 0)
        {
            _logger.trace(_traceCategory, "attached `" + remoteLogger.toString() + "'");
        }

        if(!initLogMessages.isEmpty())
        {
            filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }

        try
        {
            remoteLogger.initAsync(_logger.getPrefix(), initLogMessages.toArray(new LogMessage[0])).whenComplete(
                (Void v, Throwable ex) ->
                {
                    if(ex != null)
                    {
                        if(ex instanceof com.zeroc.Ice.LocalException)
                        {
                            deadRemoteLogger(remoteLogger, _logger, (com.zeroc.Ice.LocalException)ex, "init");
                        }
                        else
                        {
                            deadRemoteLogger(remoteLogger, _logger, new com.zeroc.Ice.UnknownException(ex), "init");
                        }
                    }
                    else
                    {
                        if(_traceLevel > 1)
                        {
                            _logger.trace(_traceCategory, "init on `" + remoteLogger.toString() +
                                          "' completed successfully");
                        }
                    }
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            deadRemoteLogger(remoteLogger, _logger, ex, "init");
            throw ex;
        }
    }

    @Override
    public boolean detachRemoteLogger(RemoteLoggerPrx remoteLogger, com.zeroc.Ice.Current current)
    {
        if(remoteLogger == null)
        {
            return false;
        }

        //
        // No need to convert the proxy as we only use its identity
        //
        boolean found = removeRemoteLogger(remoteLogger);

        if(_traceLevel > 0)
        {
            if(found)
            {
                _logger.trace(_traceCategory,  "detached `" + remoteLogger.toString() + "'");
            }
            else
            {
                _logger.trace(_traceCategory, "cannot detach `" + remoteLogger.toString() + "': not found");
            }
        }

        return found;
    }

    @Override
    public com.zeroc.Ice.LoggerAdmin.GetLogResult getLog(
        LogMessageType[] messageTypes,
        String[] categories,
        int messageMax,
        com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.LoggerAdmin.GetLogResult r = new com.zeroc.Ice.LoggerAdmin.GetLogResult();

        java.util.List<LogMessage> logMessages = null;
        synchronized(this)
        {
            if(messageMax != 0)
            {
                logMessages = new java.util.LinkedList<>(_queue);
            }
            else
            {
                logMessages = new java.util.LinkedList<>();
            }
        }

        r.prefix = _logger.getPrefix();

        if(!logMessages.isEmpty())
        {
            Filters filters = new Filters(messageTypes, categories);
            filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }
        r.returnValue = logMessages.toArray(new LogMessage[0]);
        return r;
    }

    LoggerAdminI(com.zeroc.Ice.Properties props, LoggerAdminLoggerI logger)
    {
        _maxLogCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepLogs", 100);
        _maxTraceCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepTraces", 100);
        _traceLevel = props.getPropertyAsInt("Ice.Trace.Admin.Logger");
        _logger = logger;
    }

    void destroy()
    {
        com.zeroc.Ice.Communicator sendLogCommunicator = null;

        synchronized(this)
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

    synchronized java.util.List<RemoteLoggerPrx> log(LogMessage logMessage)
    {
        java.util.List<RemoteLoggerPrx> remoteLoggers = null;

        //
        // Put message in _queue
        //
        if((logMessage.type != LogMessageType.TraceMessage && _maxLogCount > 0) ||
           (logMessage.type == LogMessageType.TraceMessage && _maxTraceCount > 0))
        {
            _queue.add(logMessage); // add at the end

            if(logMessage.type != LogMessageType.TraceMessage)
            {
                assert(_maxLogCount > 0);
                if(_logCount == _maxLogCount)
                {
                    //
                    // Need to remove the oldest log from the queue
                    //
                    assert(_oldestLog != -1);
                    _queue.remove(_oldestLog);
                    int qs = _queue.size();

                    while(_oldestLog < qs && _queue.get(_oldestLog).type == LogMessageType.TraceMessage)
                    {
                        _oldestLog++;
                    }
                    assert(_oldestLog < qs); // remember: we just added a log message at end
                }
                else
                {
                    assert(_logCount < _maxLogCount);
                    _logCount++;
                    if(_oldestLog == -1)
                    {
                        _oldestLog = _queue.size() - 1;
                    }
                }
            }
            else
            {
                assert(_maxTraceCount > 0);
                if(_traceCount == _maxTraceCount)
                {
                    //
                    // Need to remove the oldest trace from the queue
                    //
                    assert(_oldestTrace != -1);
                    _queue.remove(_oldestTrace);
                    int qs = _queue.size();
                    while(_oldestTrace < qs && _queue.get(_oldestTrace).type != LogMessageType.TraceMessage)
                    {
                        _oldestTrace++;
                    }
                    assert(_oldestTrace < qs); // remember: we just added a trace message at end
                }
                else
                {
                    assert(_traceCount < _maxTraceCount);
                    _traceCount++;
                    if(_oldestTrace == -1)
                    {
                        _oldestTrace = _queue.size() - 1;
                    }
                }
            }

            //
            // Queue updated, now find which remote loggers want this message
            //
            for(RemoteLoggerData p : _remoteLoggerMap.values())
            {
                Filters filters = p.filters;

                if(filters.messageTypes.isEmpty() || filters.messageTypes.contains(logMessage.type))
                {
                    if(logMessage.type != LogMessageType.TraceMessage || filters.traceCategories.isEmpty() ||
                       filters.traceCategories.contains(logMessage.traceCategory))
                    {
                        if(remoteLoggers == null)
                        {
                            remoteLoggers = new java.util.ArrayList<>();
                        }
                        remoteLoggers.add(p.remoteLogger);
                    }
                }
            }
        }

        return remoteLoggers;
    }

    void deadRemoteLogger(RemoteLoggerPrx remoteLogger, com.zeroc.Ice.Logger logger,
                          com.zeroc.Ice.LocalException ex, String operation)
    {
        //
        // No need to convert remoteLogger as we only use its identity
        //
        if(removeRemoteLogger(remoteLogger))
        {
            if(_traceLevel > 0)
            {
                logger.trace(_traceCategory,  "detached `" + remoteLogger.toString() + "' because "
                             + operation + " raised:\n" + ex.toString());
            }
        }
    }

    int getTraceLevel()
    {
        return _traceLevel;
    }

    private synchronized boolean removeRemoteLogger(RemoteLoggerPrx remoteLogger)
    {
        return _remoteLoggerMap.remove(remoteLogger.ice_getIdentity()) != null;
    }

    private static void filterLogMessages(java.util.List<LogMessage> logMessages,
                                          java.util.Set<LogMessageType> messageTypes,
                                          java.util.Set<String> traceCategories, int messageMax)
    {
        assert(!logMessages.isEmpty() && messageMax != 0);

        //
        // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
        // that match the other filters, if any.
        //
        if(!messageTypes.isEmpty() || !traceCategories.isEmpty() || messageMax > 0)
        {
            int count = 0;
            java.util.ListIterator<LogMessage> p = logMessages.listIterator(logMessages.size());
            while(p.hasPrevious())
            {
                boolean keepIt = false;
                LogMessage msg = p.previous();
                if(messageTypes.isEmpty() || messageTypes.contains(msg.type))
                {
                    if(msg.type != LogMessageType.TraceMessage || traceCategories.isEmpty() ||
                       traceCategories.contains(msg.traceCategory))
                    {
                        keepIt = true;
                    }
                }

                if(keepIt)
                {
                    ++count;
                    if(messageMax > 0 && count >= messageMax)
                    {
                        if(p.hasPrevious())
                        {
                            int removeCount = p.previousIndex() + 1;
                            for(int i = 0; i < removeCount; ++i)
                            {
                                logMessages.remove(0);
                            }
                        }
                        break; // while
                    }
                }
                else
                {
                    p.remove();
                }
            }
        }
        // else, don't need any filtering
    }

    //
    // Change this proxy's communicator, while keeping its invocation timeout
    //
    private static RemoteLoggerPrx changeCommunicator(RemoteLoggerPrx prx, com.zeroc.Ice.Communicator communicator)
    {
        if(prx == null)
        {
            return null;
        }

        com.zeroc.Ice.ObjectPrx result = communicator.stringToProxy(prx.toString());
        return RemoteLoggerPrx.uncheckedCast(result.ice_invocationTimeout(prx.ice_getInvocationTimeout()));
    }

    private static void copyProperties(String prefix, com.zeroc.Ice.Properties from, com.zeroc.Ice.Properties to)
    {
        for(java.util.Map.Entry<String, String> p : from.getPropertiesForPrefix(prefix).entrySet())
        {
            to.setProperty(p.getKey(), p.getValue());
        }
    }

    private static com.zeroc.Ice.Communicator createSendLogCommunicator(com.zeroc.Ice.Communicator communicator,
                                                                        com.zeroc.Ice.Logger logger)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.logger = logger;
        initData.properties = com.zeroc.Ice.Util.createProperties();

        com.zeroc.Ice.Properties mainProps = communicator.getProperties();

        copyProperties("Ice.Default.Locator", mainProps, initData.properties);
        copyProperties("Ice.Plugin.IceSSL", mainProps, initData.properties);
        copyProperties("IceSSL.", mainProps, initData.properties);

        String[] extraProps = mainProps.getPropertyAsList("Ice.Admin.Logger.Properties");

        if(extraProps.length > 0)
        {
            for(int i = 0; i < extraProps.length; ++i)
            {
                String p = extraProps[i];
                if(!p.startsWith("--"))
                {
                    extraProps[i] = "--" + p;
                }
            }
            initData.properties.parseCommandLineOptions("", extraProps);
        }
        return com.zeroc.Ice.Util.initialize(initData);
    }

    private final java.util.List<LogMessage> _queue = new java.util.LinkedList<>();
    private int _logCount = 0; // non-trace messages
    private final int _maxLogCount;
    private int _traceCount = 0;
    private final int _maxTraceCount;
    private final int _traceLevel;

    private int _oldestTrace = -1;
    private int _oldestLog = -1;

    private static class Filters
    {
        Filters(LogMessageType[] m, String[] c)
        {
            messageTypes = new java.util.HashSet<>(java.util.Arrays.asList(m));
            traceCategories = new java.util.HashSet<>(java.util.Arrays.asList(c));
        }

        final java.util.Set<LogMessageType> messageTypes;
        final java.util.Set<String> traceCategories;
    }

    private static class RemoteLoggerData
    {
        RemoteLoggerData(RemoteLoggerPrx prx, Filters f)
        {
            remoteLogger = prx;
            filters = f;
        }

        final RemoteLoggerPrx remoteLogger;
        final Filters filters;
    }

    private final java.util.Map<com.zeroc.Ice.Identity, RemoteLoggerData> _remoteLoggerMap = new java.util.HashMap<>();

    private final LoggerAdminLoggerI _logger;
    private com.zeroc.Ice.Communicator _sendLogCommunicator = null;
    private boolean _destroyed = false;
    static private final String _traceCategory = "Admin.Logger";
}
