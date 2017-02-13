// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace IceInternal
{

interface LoggerAdminLogger : Ice.Logger
{
    Ice.Object getFacet();
    void destroy();
}

sealed class LoggerAdminLoggerI : LoggerAdminLogger
{
    public void print(string message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.PrintMessage, now(), "", message);
        _localLogger.print(message);
        log(logMessage);
    }

    public void trace(string category, string message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.TraceMessage, now(), category, message);
        _localLogger.trace(category, message);
        log(logMessage);
    }

    public void warning(string message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.WarningMessage, now(), "", message);
        _localLogger.warning(message);
        log(logMessage);
    }

    public void error(string message)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(Ice.LogMessageType.ErrorMessage, now(), "", message);
        _localLogger.error(message);
        log(logMessage);
    }

    public string getPrefix()
    {
        return _localLogger.getPrefix();
    }

    public Ice.Logger cloneWithPrefix(string prefix)
    {
        return _localLogger.cloneWithPrefix(prefix);
    }

    public Ice.Object getFacet()
    {
        return _loggerAdmin;
    }

    public void destroy()
    {
        Thread thread = null;
        lock(this)
        {
            if(_sendLogThread != null)
            {
                thread = _sendLogThread;
                _sendLogThread = null;
                _destroyed = true;
                Monitor.PulseAll(this);
            }
        }

        if(thread != null)
        {
            thread.Join();
        }

        _loggerAdmin.destroy();
    }

    internal LoggerAdminLoggerI(Ice.Properties props, Ice.Logger localLogger)
    {
        LoggerAdminLoggerI wrapper = localLogger as LoggerAdminLoggerI;

        if(wrapper != null)
        {
            _localLogger = wrapper.getLocalLogger();
        }
        else
        {
            _localLogger = localLogger;
        }

        _loggerAdmin = new LoggerAdminI(props, this);
    }

    internal Ice.Logger getLocalLogger()
    {
        return _localLogger;
    }

    internal void log(Ice.LogMessage logMessage)
    {
        List<Ice.RemoteLoggerPrx> remoteLoggers = _loggerAdmin.log(logMessage);

        if(remoteLoggers != null)
        {
            Debug.Assert(remoteLoggers.Count > 0);

            lock(this)
            {
                if(_sendLogThread == null)
                {
                    _sendLogThread = new Thread(new ThreadStart(run));
                    _sendLogThread.Name = "Ice.SendLogThread";
                    _sendLogThread.IsBackground = true;
                    _sendLogThread.Start();
                }

                _jobQueue.Enqueue(new Job(remoteLoggers, logMessage));
                Monitor.PulseAll(this);
            }
        }
    }

    private void run()
    {
        if(_loggerAdmin.getTraceLevel() > 1)
        {
            _localLogger.trace(_traceCategory, "send log thread started");
        }

        for(;;)
        {
            Job job = null;
            lock(this)
            {
                while(!_destroyed && _jobQueue.Count == 0)
                {
                    Monitor.Wait(this);
                }

                if(_destroyed)
                {
                    break; // for(;;)
                }

                Debug.Assert(_jobQueue.Count > 0);
                job = _jobQueue.Dequeue();
            }

            foreach(var p in job.remoteLoggers)
            {
                if(_loggerAdmin.getTraceLevel() > 1)
                {
                    _localLogger.trace(_traceCategory, "sending log message to `" + p.ToString() + "'");
                }

                try
                {
                    //
                    // p is a proxy associated with the _sendLogCommunicator
                    //
                    p.logAsync(job.logMessage).ContinueWith(
                        (t) =>
                        {
                            try
                            {
                                t.Wait();
                                if(_loggerAdmin.getTraceLevel() > 1)
                                {
                                    _localLogger.trace(_traceCategory, "log on `" + p.ToString()
                                                       + "' completed successfully");
                                }
                            }
                            catch(AggregateException ae)
                            {
                                if(ae.InnerException is Ice.CommunicatorDestroyedException)
                                {
                                    // expected if there are outstanding calls during communicator destruction
                                }
                                if(ae.InnerException is Ice.LocalException)
                                {
                                    _loggerAdmin.deadRemoteLogger(p, _localLogger,
                                                                  (Ice.LocalException)ae.InnerException, "log");
                                }
                            }
                        });
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

    static private long now()
    {
        TimeSpan t = DateTime.UtcNow - _unixEpoch;
        return Convert.ToInt64(t.TotalMilliseconds * 1000);
    }

    private class Job
    {
        internal Job(List<Ice.RemoteLoggerPrx> r, Ice.LogMessage l)
        {
            remoteLoggers = r;
            logMessage = l;
        }

        internal readonly List<Ice.RemoteLoggerPrx> remoteLoggers;
        internal readonly Ice.LogMessage logMessage;
    }

    private readonly Ice.Logger _localLogger;
    private readonly LoggerAdminI _loggerAdmin;
    private bool _destroyed = false;
    private Thread _sendLogThread;
    private readonly Queue<Job> _jobQueue = new Queue<Job>();

    static private readonly DateTime _unixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
    static private readonly string _traceCategory = "Admin.Logger";
}

}
