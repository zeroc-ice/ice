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
        internal ILogger LocalLogger { get; }
        private const string TraceCategory = "Admin.Logger";
        private static long Now() => DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() * 1000;
        private readonly LoggerAdmin _loggerAdmin;

        public ILogger CloneWithPrefix(string prefix) => LocalLogger.CloneWithPrefix(prefix);

        public void Destroy() => _loggerAdmin.Destroy();

        public void Error(string message)
        {
            var logMessage = new LogMessage(LogMessageType.ErrorMessage, Now(), "", message);
            LocalLogger.Error(message);
            Log(logMessage);
        }

        public string GetPrefix() => LocalLogger.GetPrefix();

        public ILoggerAdmin GetFacet() => _loggerAdmin;

        public void Print(string message)
        {
            var logMessage = new LogMessage(LogMessageType.PrintMessage, Now(), "", message);
            LocalLogger.Print(message);
            Log(logMessage);
        }

        public void Trace(string category, string message)
        {
            var logMessage = new LogMessage(LogMessageType.TraceMessage, Now(), category, message);
            LocalLogger.Trace(category, message);
            Log(logMessage);
        }

        public void Warning(string message)
        {
            var logMessage = new LogMessage(LogMessageType.WarningMessage, Now(), "", message);
            LocalLogger.Warning(message);
            Log(logMessage);
        }

        internal LoggerAdminLogger(Communicator communicator, ILogger localLogger)
        {
            LocalLogger = (localLogger as LoggerAdminLogger)?.LocalLogger ?? localLogger;
            _loggerAdmin = new LoggerAdmin(communicator, this);
        }

        internal void Log(LogMessage logMessage)
        {
            List<LogForwarder>? remoteLoggers = _loggerAdmin.Log(logMessage);
            if (remoteLoggers != null)
            {
                foreach (LogForwarder p in remoteLoggers)
                {
                    p.Queue("log", LocalLogger, async prx => await prx.LogAsync(logMessage));
                }
            }
        }
    }
}
