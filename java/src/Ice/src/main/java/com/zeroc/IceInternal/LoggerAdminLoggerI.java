// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.LogMessage;
import com.zeroc.Ice.LogMessageType;

final class LoggerAdminLoggerI implements LoggerAdminLogger, Runnable
{
    @Override
    public void print(String message)
    {
        LogMessage logMessage = new LogMessage(LogMessageType.PrintMessage, now(), "", message);
        _localLogger.print(message);
        log(logMessage);
    }

    @Override
    public void trace(String category, String message)
    {
        LogMessage logMessage = new LogMessage(LogMessageType.TraceMessage, now(), category, message);
        _localLogger.trace(category, message);
        log(logMessage);
    }

    @Override
    public void warning(String message)
    {
        LogMessage logMessage = new LogMessage(LogMessageType.WarningMessage, now(), "", message);
        _localLogger.warning(message);
        log(logMessage);
    }

    @Override
    public void error(String message)
    {
        LogMessage logMessage = new LogMessage(LogMessageType.ErrorMessage, now(), "", message);
        _localLogger.error(message);
        log(logMessage);
    }

    @Override
    public String getPrefix()
    {
        return _localLogger.getPrefix();
    }

    @Override
    public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
    {
        return _localLogger.cloneWithPrefix(prefix);
    }

    @Override
    public com.zeroc.Ice.Object getFacet()
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
                throw new com.zeroc.Ice.OperationInterruptedException();
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

            for(com.zeroc.Ice.RemoteLoggerPrx p : job.remoteLoggers)
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
                    p.logAsync(job.logMessage).whenComplete(
                        (Void v, Throwable ex) ->
                        {
                            if(ex != null)
                            {
                                if(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException)
                                {
                                    // Expected if there are outstanding calls during communicator destruction.
                                }
                                else if(ex instanceof com.zeroc.Ice.LocalException)
                                {
                                    _loggerAdmin.deadRemoteLogger(p, _localLogger, (com.zeroc.Ice.LocalException)ex,
                                                                  "log");
                                }
                                else
                                {
                                    _loggerAdmin.deadRemoteLogger(p, _localLogger,
                                                                  new com.zeroc.Ice.UnknownException(ex), "log");
                                }
                            }
                            else
                            {
                                if(_loggerAdmin.getTraceLevel() > 1)
                                {
                                    _localLogger.trace(_traceCategory, "log on `" + p.toString() +
                                                       "' completed successfully");
                                }
                            }
                        });
                }
                catch(com.zeroc.Ice.LocalException ex)
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

    LoggerAdminLoggerI(com.zeroc.Ice.Properties props, com.zeroc.Ice.Logger localLogger)
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

    com.zeroc.Ice.Logger getLocalLogger()
    {
        return _localLogger;
    }

    void log(LogMessage logMessage)
    {
        java.util.List<com.zeroc.Ice.RemoteLoggerPrx> remoteLoggers = _loggerAdmin.log(logMessage);

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
        Job(java.util.List<com.zeroc.Ice.RemoteLoggerPrx> r, LogMessage l)
        {
            remoteLoggers = r;
            logMessage = l;
        }

        final java.util.List<com.zeroc.Ice.RemoteLoggerPrx> remoteLoggers;
        final LogMessage logMessage;
    }

    private final com.zeroc.Ice.Logger _localLogger;
    private final LoggerAdminI _loggerAdmin;
    private boolean _destroyed = false;
    private Thread _sendLogThread;
    private final java.util.Deque<Job> _jobQueue = new java.util.ArrayDeque<>();

    static private final String _traceCategory = "Admin.Logger";
}
