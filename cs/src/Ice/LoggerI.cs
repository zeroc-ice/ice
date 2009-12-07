// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

    public sealed class TraceLoggerI : LoggerI
    {
        public TraceLoggerI(string prefix, string file, bool console)
            : base(prefix)
        {
            if(file.Length != 0)
            {
                _file = file;
                Trace.Listeners.Add(new TextWriterTraceListener(file));
            }
            _console = console;
            if(console && !Trace.Listeners.Contains(_consoleListener))
            {
                Trace.Listeners.Add(_consoleListener);
            }
        }

        public override Logger cloneWithPrefix(string prefix)
        {
            return new TraceLoggerI(prefix, _file, _console);
        }

        protected override void write(string message)
        {
            Trace.WriteLine(message);
            Trace.Flush();
        }

        private string _file = "";
        private bool _console = false;
        internal static ConsoleTraceListener _consoleListener = new ConsoleTraceListener(true);
    }
}
