//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Globalization;
using System.IO;

namespace Ice
{
    public abstract class Logger : ILogger
    {
        public Logger(string prefix)
        {
            Prefix = prefix;

            if (prefix.Length > 0)
            {
                FormattedPrefix = prefix + ": ";
            }
            else
            {
                FormattedPrefix = "";
            }
        }

        public void Print(string message)
        {
            lock (GlobalMutex)
            {
                Write(message);
            }
        }

        public virtual void Trace(string category, string message)
        {
            string s = Format("--", category, message);
            lock (GlobalMutex)
            {
                Write(s);
            }
        }

        public virtual void Warning(string message)
        {
            string s = Format("-!", "warning", message);
            lock (GlobalMutex)
            {
                Write(s);
            }
        }

        public virtual void Error(string message)
        {
            string s = Format("!!", "error", message);
            lock (GlobalMutex)
            {
                Write(s);
            }
        }

        public string GetPrefix() => Prefix;

        private string Format(string prefix, string category, string message)
        {
            var s = new System.Text.StringBuilder(prefix);
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(Date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(Time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(FormattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }

        public abstract ILogger CloneWithPrefix(string prefix);

        protected abstract void Write(string message);

        protected readonly string Prefix;
        protected readonly string FormattedPrefix;
        protected const string Date = "d";
        protected const string Time = "HH:mm:ss:fff";

        internal static object GlobalMutex = new object();
    }

    public sealed class ConsoleLogger : Logger
    {
        public ConsoleLogger(string prefix)
            : base(prefix)
        {
        }

        public override ILogger CloneWithPrefix(string prefix) => new ConsoleLogger(prefix);

        protected override void Write(string message) => System.Console.Error.WriteLine(message);
    }

    public sealed class FileLogger : Logger
    {
        public FileLogger(string prefix, string file)
            : base(prefix)
        {
            _file = file;
            _writer = new StreamWriter(new FileStream(file, FileMode.Append, FileAccess.Write, FileShare.ReadWrite));
        }

        public override ILogger CloneWithPrefix(string prefix) => new FileLogger(prefix, _file);

        protected override void Write(string message)
        {
            _writer.WriteLine(message);
            _writer.Flush();
        }

        public void Destroy() => _writer.Close();

        private readonly string _file;
        private readonly TextWriter _writer;
    }

    public class ConsoleListener : TraceListener
    {
        public override bool IsThreadSafe => true;

        public override void TraceEvent(TraceEventCache cache, string source, TraceEventType type,
                                        int id, string message)
        {
            System.Text.StringBuilder s;
            if (type == TraceEventType.Error)
            {
                s = new System.Text.StringBuilder("!!");
            }
            else if (type == TraceEventType.Warning)
            {
                s = new System.Text.StringBuilder("-!");
            }
            else
            {
                s = new System.Text.StringBuilder("--");
            }
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(Date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(Time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(message);
            WriteLine(s.ToString());
        }

        public override void Write(string message) => System.Console.Error.Write(message);

        public override void WriteLine(string message) => System.Console.Error.WriteLine(message);

        private const string Date = "d";
        private const string Time = "HH:mm:ss:fff";
    }

    public sealed class TraceLogger : Logger
    {
        public TraceLogger(string prefix, bool console)
            : base(prefix)
        {
            _console = console;
            if (console && !System.Diagnostics.Trace.Listeners.Contains(_consoleListener))
            {
                System.Diagnostics.Trace.Listeners.Add(_consoleListener);
            }
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

        public override void Error(string message)
        {
            string s = Format("error", message);
            {
                System.Diagnostics.Trace.TraceError(s);
                System.Diagnostics.Trace.Flush();
            }
        }

        public override ILogger CloneWithPrefix(string prefix) => new TraceLogger(prefix, _console);

        protected override void Write(string message)
        {
            System.Diagnostics.Trace.WriteLine(message);
            System.Diagnostics.Trace.Flush();
        }

        private string Format(string category, string message)
        {
            var s = new System.Text.StringBuilder(FormattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }

        private readonly bool _console;
        private static readonly ConsoleListener _consoleListener = new ConsoleListener();
    }
}
