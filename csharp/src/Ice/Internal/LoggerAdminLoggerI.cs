// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

internal interface LoggerAdminLogger : Logger
{
    Ice.Object getFacet();

    // Once detach is called, the logger only logs to the local logger.
    void detach();
}

// Decorates a logger.
internal sealed class LoggerAdminLoggerI : LoggerAdminLogger
{
    private static readonly DateTime _unixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
    private readonly Logger _localLogger;
    private readonly LoggerAdminI _loggerAdmin;
    private Thread _sendLogThread;
    private volatile bool _detached;
    private readonly Queue<Job> _jobQueue = new Queue<Job>();
    private const string _traceCategory = "Admin.Logger";
    private readonly object _mutex = new();

    public void print(string message)
    {
        _localLogger.print(message);
        if (!_detached)
        {
            var logMessage = new LogMessage(LogMessageType.PrintMessage, now(), "", message);
            log(logMessage);
        }
    }

    public void trace(string category, string message)
    {
        _localLogger.trace(category, message);
        if (!_detached)
        {
            var logMessage = new LogMessage(LogMessageType.TraceMessage, now(), category, message);
            log(logMessage);
        }
    }

    public void warning(string message)
    {
        _localLogger.warning(message);
        if (!_detached)
        {
            var logMessage = new LogMessage(LogMessageType.WarningMessage, now(), "", message);
            log(logMessage);
        }
    }

    public void error(string message)
    {
        _localLogger.error(message);
        if (!_detached)
        {
            var logMessage = new LogMessage(LogMessageType.ErrorMessage, now(), "", message);
            log(logMessage);
        }
    }

    public string getPrefix() => _localLogger.getPrefix();

    public Logger cloneWithPrefix(string prefix) => _localLogger.cloneWithPrefix(prefix);

    public Ice.Object getFacet() => _loggerAdmin;

    public void detach()
    {
        Thread thread = null;
        lock (_mutex)
        {
            if (_sendLogThread != null)
            {
                thread = _sendLogThread;
                _sendLogThread = null;
                _detached = true;
                Monitor.PulseAll(_mutex);
            }
        }

        thread?.Join();

        _loggerAdmin.destroy();
    }

    public void Dispose()
    {
        detach();
        _localLogger.Dispose();
    }

    internal LoggerAdminLoggerI(Ice.Properties props, Ice.Logger localLogger)
    {
        if (localLogger is LoggerAdminLoggerI wrapper)
        {
            _localLogger = wrapper.getLocalLogger();
        }
        else
        {
            _localLogger = localLogger;
        }

        _loggerAdmin = new LoggerAdminI(props, this);
    }

    internal Ice.Logger getLocalLogger() => _localLogger;

    internal void log(Ice.LogMessage logMessage)
    {
        List<Ice.RemoteLoggerPrx> remoteLoggers = _loggerAdmin.log(logMessage);

        if (remoteLoggers != null)
        {
            Debug.Assert(remoteLoggers.Count > 0);

            lock (_mutex)
            {
                if (_sendLogThread == null)
                {
                    _sendLogThread = new Thread(new ThreadStart(run));
                    _sendLogThread.Name = "Ice.SendLogThread";
                    _sendLogThread.IsBackground = true;
                    _sendLogThread.Start();
                }

                _jobQueue.Enqueue(new Job(remoteLoggers, logMessage));
                Monitor.PulseAll(_mutex);
            }
        }
    }

    private static long now()
    {
        TimeSpan t = DateTime.UtcNow - _unixEpoch;
        return Convert.ToInt64(t.TotalMilliseconds * 1000);
    }

    private void run()
    {
        if (_loggerAdmin.getTraceLevel() > 1)
        {
            _localLogger.trace(_traceCategory, "send log thread started");
        }

        while (true)
        {
            Job job = null;
            lock (_mutex)
            {
                while (!_detached && _jobQueue.Count == 0)
                {
                    Monitor.Wait(_mutex);
                }

                if (_detached)
                {
                    break;
                }

                Debug.Assert(_jobQueue.Count > 0);
                job = _jobQueue.Dequeue();
            }

            foreach (RemoteLoggerPrx p in job.remoteLoggers)
            {
                if (_loggerAdmin.getTraceLevel() > 1)
                {
                    _localLogger.trace(_traceCategory, $"sending log message to '{p}'");
                }

                //
                // p is a proxy associated with the _sendLogCommunicator
                //
                _ = performLogAsync(p, job.logMessage);
            }
        }

        if (_loggerAdmin.getTraceLevel() > 1)
        {
            _localLogger.trace(_traceCategory, "send log thread completed");
        }

        async Task performLogAsync(RemoteLoggerPrx logger, LogMessage logMessage)
        {
            try
            {
                await logger.logAsync(logMessage).ConfigureAwait(false);
                if (_loggerAdmin.getTraceLevel() > 1)
                {
                    _localLogger.trace(_traceCategory, $"log on '{logger}' completed successfully");
                }
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // expected if there are outstanding calls during communicator destruction
            }
            catch (Ice.LocalException ex)
            {
                _loggerAdmin.deadRemoteLogger(logger, _localLogger, ex, "log");
            }
        }
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
}
