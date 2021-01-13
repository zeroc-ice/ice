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
        private readonly Channel<(List<LogForwarder>, LogMessage)> _channel;
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

        public IAsyncLoggerAdmin GetFacet() => _loggerAdmin;

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
            _channel = Channel.CreateUnbounded<(List<LogForwarder>, LogMessage)>(new UnboundedChannelOptions
            {
                AllowSynchronousContinuations = false,
                SingleReader = true,
                SingleWriter = false
            });

            Task.Run(async () =>
            {
                // The enumeration completes when the channel writer Complete method is called.
                await foreach ((List<LogForwarder> forwarders, LogMessage logMessage) in _channel.Reader.ReadAllAsync())
                {
                    foreach (LogForwarder forwarder in forwarders)
                    {
                        forwarder.Queue("log", LocalLogger, proxy => proxy.LogAsync(logMessage));
                    }
                }
            });
        }

        internal void Log(LogMessage logMessage)
        {
            List<LogForwarder>? logForwarderList = _loggerAdmin.Log(logMessage);
            if (logForwarderList != null)
            {
                _channel.Writer.TryWrite((logForwarderList, logMessage));
            }
        }
    }
}
