//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace ZeroC.Ice
{
    internal sealed class LoggerAdminLogger : ILogger
    {
        private const string TraceCategory = "Admin.Logger";
        private static long Now() => DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() * 1000;
        private readonly ILogger _localLogger;
        private readonly LoggerAdmin _loggerAdmin;

        public ILogger CloneWithPrefix(string prefix) => _localLogger.CloneWithPrefix(prefix);

        public void Destroy() => _loggerAdmin.Destroy();

        public void Error(string message)
        {
            var logMessage = new LogMessage(LogMessageType.ErrorMessage, Now(), "", message);
            _localLogger.Error(message);
            Log(logMessage);
        }

        public string GetPrefix() => _localLogger.GetPrefix();

        public ILoggerAdmin GetFacet() => _loggerAdmin;

        public void Print(string message)
        {
            var logMessage = new LogMessage(LogMessageType.PrintMessage, Now(), "", message);
            _localLogger.Print(message);
            Log(logMessage);
        }

        public void Trace(string category, string message)
        {
            var logMessage = new LogMessage(LogMessageType.TraceMessage, Now(), category, message);
            _localLogger.Trace(category, message);
            Log(logMessage);
        }

        public void Warning(string message)
        {
            var logMessage = new LogMessage(LogMessageType.WarningMessage, Now(), "", message);
            _localLogger.Warning(message);
            Log(logMessage);
        }

        internal LoggerAdminLogger(Communicator communicator, ILogger localLogger)
        {
            _localLogger = (localLogger as LoggerAdminLogger)?.GetLocalLogger() ?? localLogger;
            _loggerAdmin = new LoggerAdmin(communicator, this);
        }

        internal ILogger GetLocalLogger() => _localLogger;

        internal void Log(LogMessage logMessage)
        {
            List<RemoteLoggerQueue>? remoteLoggers = _loggerAdmin.Log(logMessage);
            if (remoteLoggers != null)
            {
                foreach (RemoteLoggerQueue p in remoteLoggers)
                {
                    p.Queue(async prx =>
                    {
                        if (_loggerAdmin.GetTraceLevel() > 1)
                        {
                            _localLogger.Trace(TraceCategory, $"sending log message to `{prx}'");
                        }
                        await prx.LogAsync(logMessage);
                        if (_loggerAdmin.GetTraceLevel() > 1)
                        {
                            _localLogger.Trace(TraceCategory, $"log on `{prx}' completed successfully");
                        }
                    }, _localLogger, "log");
                }
            }
        }
    }
}
