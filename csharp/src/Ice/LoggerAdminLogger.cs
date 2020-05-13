//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace IceInternal
{
    internal interface ILoggerAdminLogger : Ice.ILogger
    {
        Ice.ILoggerAdmin GetFacet();
        void Destroy();
    }

    internal sealed class LoggerAdminLogger : ILoggerAdminLogger
    {
        public void Print(string message)
        {
            var logMessage = new Ice.LogMessage(Ice.LogMessageType.PrintMessage, Now(), "", message);
            _localLogger.Print(message);
            Log(logMessage);
        }

        public void Trace(string category, string message)
        {
            var logMessage = new Ice.LogMessage(Ice.LogMessageType.TraceMessage, Now(), category, message);
            _localLogger.Trace(category, message);
            Log(logMessage);
        }

        public void Warning(string message)
        {
            var logMessage = new Ice.LogMessage(Ice.LogMessageType.WarningMessage, Now(), "", message);
            _localLogger.Warning(message);
            Log(logMessage);
        }

        public void Error(string message)
        {
            var logMessage = new Ice.LogMessage(Ice.LogMessageType.ErrorMessage, Now(), "", message);
            _localLogger.Error(message);
            Log(logMessage);
        }

        public string GetPrefix() => _localLogger.GetPrefix();

        public Ice.ILogger CloneWithPrefix(string prefix) => _localLogger.CloneWithPrefix(prefix);

        public Ice.ILoggerAdmin GetFacet() => _loggerAdmin;

        public void Destroy()
        {
            Thread? thread = null;
            lock (this)
            {
                if (_sendLogThread != null)
                {
                    thread = _sendLogThread;
                    _sendLogThread = null;
                    _destroyed = true;
                    Monitor.PulseAll(this);
                }
            }

            if (thread != null)
            {
                thread.Join();
            }

            _loggerAdmin.Destroy();
        }

        internal LoggerAdminLogger(Ice.Communicator communicator, Ice.ILogger localLogger)
        {
            if (localLogger is LoggerAdminLogger)
            {
                _localLogger = ((LoggerAdminLogger)localLogger).GetLocalLogger();
            }
            else
            {
                _localLogger = localLogger;
            }
            _loggerAdmin = new LoggerAdmin(communicator, this);
        }

        internal Ice.ILogger GetLocalLogger() => _localLogger;

        internal void Log(Ice.LogMessage logMessage)
        {
            List<Ice.IRemoteLoggerPrx>? remoteLoggers = _loggerAdmin.Log(logMessage);

            if (remoteLoggers != null)
            {
                Debug.Assert(remoteLoggers.Count > 0);

                lock (this)
                {
                    if (_sendLogThread == null)
                    {
                        _sendLogThread = new Thread(new ThreadStart(Run));
                        _sendLogThread.Name = "Ice.SendLogThread";
                        _sendLogThread.IsBackground = true;
                        _sendLogThread.Start();
                    }

                    _jobQueue.Enqueue(new Job(remoteLoggers, logMessage));
                    Monitor.PulseAll(this);
                }
            }
        }

        private void Run()
        {
            if (_loggerAdmin.GetTraceLevel() > 1)
            {
                _localLogger.Trace(TraceCategory, "send log thread started");
            }

            for (; ; )
            {
                Job job;
                lock (this)
                {
                    while (!_destroyed && _jobQueue.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    if (_destroyed)
                    {
                        break; // for(;;)
                    }

                    Debug.Assert(_jobQueue.Count > 0);
                    job = _jobQueue.Dequeue();
                }

                foreach (Ice.IRemoteLoggerPrx p in job.RemoteLoggers)
                {
                    if (_loggerAdmin.GetTraceLevel() > 1)
                    {
                        _localLogger.Trace(TraceCategory, "sending log message to `" + p.ToString() + "'");
                    }

                    try
                    {
                        //
                        // p is a proxy associated with the _sendLogCommunicator
                        //
                        p.LogAsync(job.LogMessage).ContinueWith(
                            (t) =>
                            {
                                try
                                {
                                    t.Wait();
                                    if (_loggerAdmin.GetTraceLevel() > 1)
                                    {
                                        _localLogger.Trace(TraceCategory, "log on `" + p.ToString()
                                                           + "' completed successfully");
                                    }
                                }
                                catch (AggregateException ae)
                                {
                                    if (ae.InnerException is Ice.CommunicatorDestroyedException)
                                    {
                                        // expected if there are outstanding calls during communicator destruction
                                    }

                                    Debug.Assert(ae.InnerException != null);
                                    _loggerAdmin.DeadRemoteLogger(p, _localLogger, ae.InnerException, "log");
                                }
                            },
                            System.Threading.Tasks.TaskScheduler.Current);
                    }
                    catch (System.Exception ex)
                    {
                        _loggerAdmin.DeadRemoteLogger(p, _localLogger, ex, "log");
                    }
                }
            }

            if (_loggerAdmin.GetTraceLevel() > 1)
            {
                _localLogger.Trace(TraceCategory, "send log thread completed");
            }
        }

        private static long Now() => DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() * 1000;

        private class Job
        {
            internal Job(List<Ice.IRemoteLoggerPrx> r, Ice.LogMessage l)
            {
                RemoteLoggers = r;
                LogMessage = l;
            }

            internal readonly List<Ice.IRemoteLoggerPrx> RemoteLoggers;
            internal readonly Ice.LogMessage LogMessage;
        }

        private readonly Ice.ILogger _localLogger;
        private readonly LoggerAdmin _loggerAdmin;
        private bool _destroyed = false;
        private Thread? _sendLogThread;
        private readonly Queue<Job> _jobQueue = new Queue<Job>();

        private const string TraceCategory = "Admin.Logger";
    }
}
