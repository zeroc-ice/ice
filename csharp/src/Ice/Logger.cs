// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Text;

namespace ZeroC.Ice
{
    internal abstract class Logger : ILogger
    {
        public string Prefix { get; }

        protected const string Date = "d";
        protected readonly string FormattedPrefix;
        protected const string Time = "HH:mm:ss:fff";

        public abstract ILogger CloneWithPrefix(string prefix);

        public virtual void Error(string message) => Write(Format("!!", "error", message));

        public virtual void Print(string message) => Write(message);

        public virtual void Trace(string category, string message) => Write(Format("--", category, message));

        public virtual void Warning(string message) => Write(Format("-!", "warning", message));

        /// <summary>Creates a new logger, used only by derived classes.</summary>
        /// <param name="prefix">The prefix to perpend to messages write by this logger.</param>
        protected Logger(string prefix)
        {
            Prefix = prefix;
            FormattedPrefix = prefix.Length > 0 ? $"{prefix}: " : "";
        }

        /// <summary>Writes a message using this logger.</summary>
        /// <param name="message">The message to write.</param>
        protected abstract void Write(string message);

        private string Format(string prefix, string category, string message)
        {
            var s = new StringBuilder(prefix);
            s.Append(' ');
            s.Append(DateTime.Now.ToString(Date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(DateTime.Now.ToString(Time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(FormattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }
    }

    /// <summary>Represents a logger that writes messages to the standard error output stream.</summary>
    internal sealed class ConsoleLogger : Logger
    {
        public override ILogger CloneWithPrefix(string prefix) => new ConsoleLogger(prefix);

        protected override void Write(string message) => Console.Error.WriteLine(message);

        /// <summary>Creates a new console logger.</summary>
        /// <param name="prefix">The prefix to perpend to messages write by this logger.</param>
        internal ConsoleLogger(string prefix)
            : base(prefix)
        {
        }
    }

    /// <summary>Represents a logger that writes messages to a file.</summary>
    internal sealed class FileLogger : Logger, IDisposable
    {
        private readonly string _file;
        private readonly StreamWriter _streamWriter;
        private readonly TextWriter _synchronizedWriter;

        public override ILogger CloneWithPrefix(string prefix) =>
            new FileLogger(prefix, _file, _streamWriter, _synchronizedWriter);

        public void Dispose()
        {
            _streamWriter.Dispose();
            _synchronizedWriter.Dispose();
        }

        /// <summary>Creates a new file logger.</summary>
        /// <param name="prefix">The prefix to perpend to messages write by this logger.</param>
        /// <param name="file">The file path to write to.</param>
        internal FileLogger(string prefix, string file)
            : base(prefix)
        {
            _file = file;
            _streamWriter = new StreamWriter(new FileStream(file,
                                                            FileMode.Append,
                                                            FileAccess.Write,
                                                            FileShare.ReadWrite));
            _synchronizedWriter = TextWriter.Synchronized(_streamWriter);
        }

        protected override void Write(string message)
        {
            try
            {
                _synchronizedWriter.WriteLine(message);
                _synchronizedWriter.Flush();
            }
            catch (ObjectDisposedException)
            {
                // Expected if logging occurs after communicator destruction.
            }
        }

        // only to use by clone with prefix and avoid reopening the file in write mode
        private FileLogger(string prefix, string file, StreamWriter streamWriter, TextWriter textWriter)
            : base(prefix)
        {
            _file = file;
            _streamWriter = streamWriter;
            _synchronizedWriter = textWriter;
        }
    }

    /// <summary>Creates a logger that trace messages using <see cref="System.Diagnostics.Trace"/> API,
    /// this is the default logger.</summary>
    internal sealed class TraceLogger : Logger
    {
        private static readonly ConsoleListener _consoleListener = new ConsoleListener();
        private readonly bool _addConsoleListener;

        public override ILogger CloneWithPrefix(string prefix) => new TraceLogger(prefix, _addConsoleListener);
        public override void Error(string message)
        {
            System.Diagnostics.Trace.TraceError(Format("error", message));
            System.Diagnostics.Trace.Flush();
        }

        public override void Trace(string category, string message)
        {
            System.Diagnostics.Trace.TraceInformation(Format(category, message));
            System.Diagnostics.Trace.Flush();
        }

        public override void Warning(string message)
        {
            System.Diagnostics.Trace.TraceWarning(Format("warning", message));
            System.Diagnostics.Trace.Flush();
        }

        /// <summary>Creates a new trace logger.</summary>
        /// <param name="prefix">The prefix to perpend to messages write by this logger.</param>
        /// <param name="addConsoleListener">If true a listener that writes messages to the standard error is added
        /// to trace listeners <see cref="Trace.Listeners"/> this is only done once per process.</param>
        internal TraceLogger(string prefix, bool addConsoleListener)
            : base(prefix)
        {
            _addConsoleListener = addConsoleListener;
            if (addConsoleListener && !System.Diagnostics.Trace.Listeners.Contains(_consoleListener))
            {
                System.Diagnostics.Trace.Listeners.Add(_consoleListener);
            }
        }

        protected override void Write(string message)
        {
            System.Diagnostics.Trace.WriteLine(message);
            System.Diagnostics.Trace.Flush();
        }

        private string Format(string category, string message)
        {
            var s = new StringBuilder(FormattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }
    }

    internal class ConsoleListener : TraceListener
    {
        public override bool IsThreadSafe => true;

        private const string Date = "d";
        private const string Time = "HH:mm:ss:fff";

        public override void TraceEvent(
            TraceEventCache? cache,
            string source,
            TraceEventType type,
            int id,
            string? message)
        {
            var s = new StringBuilder(
                type switch
                {
                    TraceEventType.Error => "!!",
                    TraceEventType.Warning => "-!",
                    _ => "--"
                });
            s.Append(' ');
            s.Append(DateTime.Now.ToString(Date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(DateTime.Now.ToString(Time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(message);
            WriteLine(s.ToString());
        }

        public override void Write(string? message) => Console.Error.Write(message);

        public override void WriteLine(string? message) => Console.Error.WriteLine(message);
    }
}
