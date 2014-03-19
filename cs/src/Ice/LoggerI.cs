// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            if(prefix.Length > 0)
            {
                _prefix = prefix + ": ";
            }
            
            _date = "d";
            _time = "HH:mm:ss:fff";
        }

        public void print( string message)
        {
            lock(_globalMutex)
            {
                write(message);
            }
        }

        public void trace(string category, string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder("-- ");
            s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(_prefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Replace("\n", "\n   ");

            lock(_globalMutex)
            {
                write(s.ToString());
            }
        }
        
        public void warning(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder("-! ");
            s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("warning: ");
            s.Append(message);
            s.Replace("\n", "\n   ");

            lock(_globalMutex)
            {
                write(s.ToString());
            }
        }
        
        public void error(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder("!! ");
            s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("error: ");
            s.Append(message);
            s.Replace("\n", "\n   ");

            lock(_globalMutex)
            {
                write(s.ToString());
            }
        }

        public abstract Logger cloneWithPrefix(string prefix);

        protected abstract void write(string message);

        internal string _prefix = "";
        internal string _date = null;
        internal string _time = null;

        internal static object _globalMutex = new object();
    }

    public sealed class ConsoleLoggerI : LoggerI
    {
        public ConsoleLoggerI(string prefix)
            : base(prefix)
        {
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

        public override Logger cloneWithPrefix(string prefix)
        {
            return new TraceLoggerI(prefix, _console);
        }

        protected override void write(string message)
        {
            Trace.WriteLine(message);
            Trace.Flush();
        }

        private bool _console;
        internal static ConsoleTraceListener _consoleListener = new ConsoleTraceListener(true);
    }
#  endif
#endif
}
