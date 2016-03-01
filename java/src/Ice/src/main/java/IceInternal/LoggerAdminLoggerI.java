// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class LoggerAdminLoggerI implements LoggerAdminLogger, Runnable
{
    @Override
    public void print(String message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.PrintMessage, now(), "", message);
        _localLogger.print(message);
        log(logMessage);
    }

    @Override
    public void trace(String category, String message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.TraceMessage, now(), category, message);
        _localLogger.trace(category, message);
        log(logMessage);
    }

    @Override
    public void warning(String message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.WarningMessage, now(), "", message);
        _localLogger.warning(message);
        log(logMessage);
    }

    @Override
    public void error(String message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.ErrorMessage, now(), "", message);
        _localLogger.error(message);
        log(logMessage);
    }

    @Override
    public String getPrefix()
    {
        return _localLogger.getPrefix();
    }

    @Override
    public Ice.Logger cloneWithPrefix(String prefix)
    {
        return _localLogger.cloneWithPrefix(prefix);
    }
 
    @Override
    public Ice.Object getFacet()
    {
        return _loggerAdmin;
    }

    @Override
    public void destroy()
    {
        Thread thread = null;
        synchronized(this)
        {
            if(_sendLogThread != null)
            {
                thread = _sendLogThread;
                _sendLogThread = null;
                _destroyed = true;
                notifyAll();
            }
        }
        
        if(thread != null)
        {
            try
            {
                thread.join();
            }
            catch(InterruptedException e)
            {
                synchronized(this)
                {
                    _sendLogThread = thread;
                }
                throw new Ice.OperationInterruptedException();
            }
        }

        _loggerAdmin.destroy();
    }

    @Override
    public void run()
    {
        if(_loggerAdmin.getTraceLevel() > 1)
        {
            _localLogger.trace(_traceCategory, "send log thread started");
        }

        final Ice.Callback logCompletedCb = new Ice.Callback() 
            {

                @Override
                public void completed(Ice.AsyncResult r)
                {
                    Ice.RemoteLoggerPrx remoteLogger = Ice.RemoteLoggerPrxHelper.uncheckedCast(r.getProxy());
                    
                    try
                    {
                        remoteLogger.end_log(r);
                        
                        if(_loggerAdmin.getTraceLevel() > 1)
                        {
                            _localLogger.trace(_traceCategory, r.getOperation() + " on `" + remoteLogger.toString() +
                                               "' completed successfully");
                        }
                    }
                    catch(Ice.CommunicatorDestroyedException ex)
                    {
                        // expected if there are outstanding calls during
                        // communicator destruction
                    }
                    catch(Ice.LocalException ex)
                    {
                        _loggerAdmin.deadRemoteLogger(remoteLogger, _localLogger, ex, r.getOperation());
                    }
                }
            };
        
        for(;;)
        {
            Job job = null;
            synchronized(this)
            {
                while(!_destroyed && _jobQueue.isEmpty())
                {
                    try
                    {
                        wait();
                    }
                    catch(InterruptedException e)
                    {
                        // Ignored, this should never occur
                    }
                }
                
                if(_destroyed)
                {
                    break; // for(;;)
                }

                assert(!_jobQueue.isEmpty());
                job = _jobQueue.removeFirst();
            }
            
            for(Ice.RemoteLoggerPrx p : job.remoteLoggers)
            {
                if(_loggerAdmin.getTraceLevel() > 1)
                {
                    _localLogger.trace(_traceCategory, "sending log message to `" + p.toString() + "'");
                }
            
                try
                {
                    //
                    // p is a proxy associated with the _sendLogCommunicator
                    //
                    p.begin_log(job.logMessage, logCompletedCb);
                }
                catch(Ice.LocalException ex)
                {
                    _loggerAdmin.deadRemoteLogger(p, _localLogger, ex, "log");
                }
            }  
        }

        if(_loggerAdmin.getTraceLevel() > 1)
        {
            _localLogger.trace(_traceCategory, "send log thread completed");
        }
    }

    LoggerAdminLoggerI(Ice.Properties props, Ice.Logger localLogger)
    {
        if(localLogger instanceof LoggerAdminLoggerI)
        {
            _localLogger = ((LoggerAdminLoggerI)localLogger).getLocalLogger();
        }
        else
        {
            _localLogger = localLogger;
        }

        _loggerAdmin = new LoggerAdminI(props, this);
    }

    Ice.Logger getLocalLogger()
    {
        return _localLogger;
    }

    void log(Ice.LogMessage logMessage)
    {
        java.util.List<Ice.RemoteLoggerPrx> remoteLoggers = _loggerAdmin.log(logMessage);
        
        if(remoteLoggers != null)
        {
            assert(!remoteLoggers.isEmpty());
            
            synchronized(this)
            {
                if(_sendLogThread == null)
                {
                    _sendLogThread = new Thread(this, "Ice.SendLogThread");
                    _sendLogThread.start();
                }
                
                _jobQueue.addLast(new Job(remoteLoggers, logMessage));
                notifyAll();
            }            
        }
    }

    static private long now()
    {
        return java.util.Calendar.getInstance().getTimeInMillis() * 1000;
    }
    
    private static class Job
    {
        Job(java.util.List<Ice.RemoteLoggerPrx> r, Ice.LogMessage l)
        {
            remoteLoggers = r;
            logMessage = l;
        }
        
        final java.util.List<Ice.RemoteLoggerPrx> remoteLoggers;
        final Ice.LogMessage logMessage;
    } 
   
    private final Ice.Logger _localLogger;
    private final LoggerAdminI _loggerAdmin;
    private boolean _destroyed = false;
    private Thread _sendLogThread;
    private final java.util.Deque<Job> _jobQueue = new java.util.ArrayDeque<Job>();
    
    static private final String _traceCategory = "Admin.Logger";
}
