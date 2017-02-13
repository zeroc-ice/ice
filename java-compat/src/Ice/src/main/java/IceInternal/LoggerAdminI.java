// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class LoggerAdminI extends Ice._LoggerAdminDisp
{
    @Override
    public void attachRemoteLogger(Ice.RemoteLoggerPrx prx, Ice.LogMessageType[] messageTypes, String[] categories,
            int messageMax, Ice.Current current)
        throws Ice.RemoteLoggerAlreadyAttachedException
    {
        if(prx == null)
        {
            return; // can't send this null RemoteLogger anything!
        }
        
        Ice.RemoteLoggerPrx remoteLogger = Ice.RemoteLoggerPrxHelper.uncheckedCast(prx.ice_twoway());
       
        Filters filters = new Filters(messageTypes, categories);
        java.util.List<Ice.LogMessage> initLogMessages = null;
       
        synchronized(this)
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
            
            if(_remoteLoggerMap.containsKey(remoteLoggerId))
            {
                if(_traceLevel > 0)
                {
                    _logger.trace(_traceCategory, "rejecting `" + remoteLogger.toString() +
                                                  "' with RemoteLoggerAlreadyAttachedException");
                }
                
                throw new Ice.RemoteLoggerAlreadyAttachedException();
            }

            _remoteLoggerMap.put(remoteLoggerId, 
                                 new RemoteLoggerData(changeCommunicator(remoteLogger, _sendLogCommunicator), filters));
            
            if(messageMax != 0)
            {
                initLogMessages = new java.util.LinkedList<Ice.LogMessage>(_queue); // copy
            }
            else
            {
                initLogMessages = new java.util.LinkedList<Ice.LogMessage>();
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
        
        final Ice.Callback initCompletedCb = new Ice.Callback()
            {
                @Override
                public void completed(Ice.AsyncResult r)
                {
                    Ice.RemoteLoggerPrx remoteLogger = Ice.RemoteLoggerPrxHelper.uncheckedCast(r.getProxy());
                    
                    try
                    {
                        remoteLogger.end_init(r);
                        
                        if(_traceLevel > 1)
                        {
                            _logger.trace(_traceCategory, r.getOperation() + " on `" + remoteLogger.toString() +
                                          "' completed successfully");
                        }
                    }
                    catch(Ice.LocalException ex)
                    {
                        deadRemoteLogger(remoteLogger, _logger, ex, r.getOperation());
                    }
                }
            };
   
        try
        {
            remoteLogger.begin_init(_logger.getPrefix(), initLogMessages.toArray(new Ice.LogMessage[0]), 
                                    initCompletedCb);
        }
        catch(Ice.LocalException ex)
        {
            deadRemoteLogger(remoteLogger, _logger, ex, "init");
            throw ex;
        }    
    }
    
    @Override
    public boolean detachRemoteLogger(Ice.RemoteLoggerPrx remoteLogger, Ice.Current current)
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
    public Ice.LogMessage[] getLog(Ice.LogMessageType[] messageTypes, String[] categories, int messageMax,
            Ice.StringHolder prefix, Ice.Current current)
    {
        java.util.List<Ice.LogMessage> logMessages = null;
        synchronized(this)
        {
            if(messageMax != 0)
            {
                logMessages = new java.util.LinkedList<Ice.LogMessage>(_queue);
            }
            else
            {
                logMessages = new java.util.LinkedList<Ice.LogMessage>();
            }
        }
        
        prefix.value = _logger.getPrefix();
    
        if(!logMessages.isEmpty())
        {
            Filters filters = new Filters(messageTypes, categories);
            filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }
        return logMessages.toArray(new Ice.LogMessage[0]);
    }


    LoggerAdminI(Ice.Properties props, LoggerAdminLoggerI logger)
    {
        _maxLogCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepLogs", 100);
        _maxTraceCount = props.getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepTraces", 100);
        _traceLevel = props.getPropertyAsInt("Ice.Trace.Admin.Logger");
        _logger = logger;
    }
    
    void destroy()
    {
        Ice.Communicator sendLogCommunicator = null;

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

    synchronized java.util.List<Ice.RemoteLoggerPrx> log(Ice.LogMessage logMessage)
    {
        java.util.List<Ice.RemoteLoggerPrx> remoteLoggers = null;

        //
        // Put message in _queue
        //
        if((logMessage.type != Ice.LogMessageType.TraceMessage && _maxLogCount > 0) || 
           (logMessage.type == Ice.LogMessageType.TraceMessage && _maxTraceCount > 0)) 
        {
            _queue.add(logMessage); // add at the end 
            
            if(logMessage.type != Ice.LogMessageType.TraceMessage)
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
                    
                    while(_oldestLog < qs && _queue.get(_oldestLog).type == Ice.LogMessageType.TraceMessage)
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
                    while(_oldestTrace < qs && _queue.get(_oldestTrace).type != Ice.LogMessageType.TraceMessage)
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
                    if(logMessage.type != Ice.LogMessageType.TraceMessage || filters.traceCategories.isEmpty() ||
                       filters.traceCategories.contains(logMessage.traceCategory)) 
                    {
                        if(remoteLoggers == null)
                        {
                            remoteLoggers = new java.util.ArrayList<Ice.RemoteLoggerPrx>();
                        }
                        remoteLoggers.add(p.remoteLogger);
                    }
                }
            }
        }
   
        return remoteLoggers;
    }
   
    void deadRemoteLogger(Ice.RemoteLoggerPrx remoteLogger, Ice.Logger logger, Ice.LocalException ex, String operation)
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
   
    private synchronized boolean removeRemoteLogger(Ice.RemoteLoggerPrx remoteLogger)
    {
        return _remoteLoggerMap.remove(remoteLogger.ice_getIdentity()) != null; 
    }
    
    private static void filterLogMessages(java.util.List<Ice.LogMessage> logMessages,
                                          java.util.Set<Ice.LogMessageType> messageTypes,
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
            java.util.ListIterator<Ice.LogMessage> p = logMessages.listIterator(logMessages.size());
            while(p.hasPrevious())
            {
                boolean keepIt = false;
                Ice.LogMessage msg = p.previous();
                if(messageTypes.isEmpty() || messageTypes.contains(msg.type))
                {
                    if(msg.type != Ice.LogMessageType.TraceMessage || traceCategories.isEmpty() ||
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
    private static Ice.RemoteLoggerPrx changeCommunicator(Ice.RemoteLoggerPrx prx, Ice.Communicator communicator)
    {
        if(prx == null)
        {
            return null;
        }

        Ice.ObjectPrx result = communicator.stringToProxy(prx.toString());
        return Ice.RemoteLoggerPrxHelper.uncheckedCast(result.ice_invocationTimeout(prx.ice_getInvocationTimeout()));
    }

    private static void copyProperties(String prefix, Ice.Properties from, Ice.Properties to)
    {
        for(java.util.Map.Entry<String, String> p : from.getPropertiesForPrefix(prefix).entrySet())
        {
            to.setProperty(p.getKey(), p.getValue());
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
        return Ice.Util.initialize(initData);
    }

     
    private final java.util.List<Ice.LogMessage> _queue = new java.util.LinkedList<Ice.LogMessage>();
    private int _logCount = 0; // non-trace messages
    private final int _maxLogCount;
    private int _traceCount = 0;
    private final int _maxTraceCount;
    private final int _traceLevel;

    private int _oldestTrace = -1;
    private int _oldestLog = -1;

    private static class Filters
    {
        Filters(Ice.LogMessageType[] m, String[] c)
        {
            messageTypes = new java.util.HashSet<Ice.LogMessageType>(java.util.Arrays.asList(m));
            traceCategories = new java.util.HashSet<String>(java.util.Arrays.asList(c));
        }

        final java.util.Set<Ice.LogMessageType> messageTypes;
        final java.util.Set<String> traceCategories;
    }

    private static class RemoteLoggerData
    {
        RemoteLoggerData(Ice.RemoteLoggerPrx prx, Filters f)
        {
            remoteLogger = prx;
            filters = f;
        }

        final Ice.RemoteLoggerPrx remoteLogger;
        final Filters filters;
    }

    private final java.util.Map<Ice.Identity, RemoteLoggerData> _remoteLoggerMap 
        = new java.util.HashMap<Ice.Identity, RemoteLoggerData>(); 
    
    private final LoggerAdminLoggerI _logger;
    private Ice.Communicator _sendLogCommunicator = null;
    private boolean _destroyed = false;
    static private final String _traceCategory = "Admin.Logger";
}
