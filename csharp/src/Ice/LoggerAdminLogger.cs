// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class LoggerAdminLogger : ILogger, IAsyncDisposable
    {
        public string Prefix => LocalLogger.Prefix;

        internal ILogger LocalLogger { get; }
        private static long Now() => DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() * 1000;
        private readonly Channel<LogMessage> _channel;
        private readonly LoggerAdmin _loggerAdmin;

        public ILogger CloneWithPrefix(string prefix) => LocalLogger.CloneWithPrefix(prefix);

        public async ValueTask DisposeAsync()
        {
            if (LocalLogger is FileLogger fileLogger)
            {
                fileLogger.Dispose();
            }
            _channel.Writer.Complete();
            await _loggerAdmin.DisposeAsync().ConfigureAwait(false);
        }

        public void Error(string message)
        {
            var logMessage = new LogMessage(LogMessageType.ErrorMessage, Now(), "", message);
            LocalLogger.Error(message);
            Log(logMessage);
        }

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

            // Create an unbounded channel to ensure the messages are sent from a separate thread. We don't allow
            // synchronous continuations to ensure that writes on the channel are never processed by the writer
            // thread.
            _channel = Channel.CreateUnbounded<LogMessage>(new UnboundedChannelOptions
            {
                AllowSynchronousContinuations = false,
                SingleReader = true,
                SingleWriter = false
            });

            Task.Run(async () =>
            {
                // The enumeration completes when the channel writer Complete method is called.
                await foreach (LogMessage logMessage in _channel.Reader.ReadAllAsync())
                {
                    List<LogForwarder>? logForwarderList = _loggerAdmin.Log(logMessage);
                    if (logForwarderList != null)
                    {
                        foreach (LogForwarder p in logForwarderList)
                        {
                            p.Queue("log", LocalLogger, prx => prx.LogAsync(logMessage));
                        }
                    }
                }
            });
        }

        internal void Log(LogMessage logMessage) => _channel.Writer.TryWrite(logMessage);
    }
}
