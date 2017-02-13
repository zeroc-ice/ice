// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define TRACE

namespace Ice
{
    using System.Diagnostics;
    using System.Globalization;
#if !SILVERLIGHT && !UNITY
    using System.IO;
#endif

    public abstract class LoggerI : Logger
    {
        public LoggerI(string prefix)
        {
            _prefix = prefix;

            if(prefix.Length > 0)
            {
                _formattedPrefix = prefix + ": ";
            }

            _date = "d";
            _time = "HH:mm:ss:fff";
        }

        public void print(string message)
        {
            lock(_globalMutex)
            {
                write(message);
            }
        }

        public virtual void trace(string category, string message)
        {
            string s = format("--", category, message);
            lock(_globalMutex)
            {
                write(s);
            }
        }

        public virtual void warning(string message)
        {
            string s = format("-!", "warning", message);
            lock(_globalMutex)
            {
                write(s);
            }
        }

        public virtual void error(string message)
        {
            string s = format("!!", "error", message);
            lock(_globalMutex)
            {
                write(s);
            }
        }

        public string getPrefix()
        {
            return _prefix;
        }

        private string format(string prefix, string category, string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder(prefix);
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(_formattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }

        public abstract Logger cloneWithPrefix(string prefix);

        protected abstract void write(string message);

        internal string _prefix = "";
        internal string _formattedPrefix = "";
        internal string _date = null;
        internal string _time = null;

        internal static object _globalMutex = new object();
    }

    public sealed class ConsoleLoggerI : LoggerI
    {
        public ConsoleLoggerI(string prefix)
            : base(prefix)
        {
            _date = "d";
            _time = "HH:mm:ss:fff";
        }

        public override Logger cloneWithPrefix(string prefix)
        {
            return new ConsoleLoggerI(prefix);
        }

        protected override void write(string message)
        {
            System.Console.Error.WriteLine(message);
        }
    }

#if !SILVERLIGHT && !UNITY
    public sealed class FileLoggerI : LoggerI
    {
        public FileLoggerI(string prefix, string file) :
            base(prefix)
        {
            _file = file;
            _writer = new StreamWriter(new FileStream(file, FileMode.Append, FileAccess.Write, FileShare.None));
        }

        public override Logger cloneWithPrefix(string prefix)
        {
            return new FileLoggerI(prefix, _file);
        }

        protected override void write(string message)
        {
            _writer.WriteLine(message);
            _writer.Flush();
        }

        private string _file;
        private TextWriter _writer;
    }


#  if !COMPACT
    public class ConsoleListener : TraceListener
    {
        public ConsoleListener()
        {
            _date = "d";
            _time = "HH:mm:ss:fff";
        }

        public override void TraceEvent(TraceEventCache cache, string source, TraceEventType type,
                                        int id, string message)
        {
            System.Text.StringBuilder s;
            if(type == TraceEventType.Error)
            {
                s = new System.Text.StringBuilder("!!");
            }
            else if(type == TraceEventType.Warning)
            {
                s = new System.Text.StringBuilder("-!");
            }
            else
            {
                s = new System.Text.StringBuilder("--");
            }
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(message);
            this.WriteLine(s.ToString());
        }

        public override void Write(string message)
        {
            System.Console.Error.Write(message);
        }

        public override void WriteLine(string message)
        {
            System.Console.Error.WriteLine(message);
        }

        internal string _date = null;
        internal string _time = null;
    }

    public sealed class TraceLoggerI : LoggerI
    {
        public TraceLoggerI(string prefix, bool console)
            : base(prefix)
        {
            _console = console;
            if(console && !Trace.Listeners.Contains(_consoleListener))
            {
                Trace.Listeners.Add(_consoleListener);
            }
        }

        public override void trace(string category, string message)
        {
            string s = format(category, message);
            lock(_globalMutex)
            {
                Trace.TraceInformation(s);
                Trace.Flush();
            }
        }

        public override void warning(string message)
        {
            string s = format("warning", message);
            lock(_globalMutex)
            {
                Trace.TraceWarning(s);
                Trace.Flush();
            }
        }

        public override void error(string message)
        {
            string s = format("error", message);
            {
                Trace.TraceError(s);
                Trace.Flush();
            }
        }

        public override Logger cloneWithPrefix(string prefix)
        {
            return new TraceLoggerI(prefix, _console);
        }

        protected override void write(string message)
        {
            Trace.WriteLine(message);
            Trace.Flush();
        }

        private string format(string category, string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder(_formattedPrefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");
            return s.ToString();
        }

        private bool _console;
        internal static ConsoleListener _consoleListener = new ConsoleListener();
    }
#  endif
#endif
}
