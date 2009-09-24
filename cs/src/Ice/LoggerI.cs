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
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
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
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
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
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
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

        protected abstract void write(string message);

        internal string _prefix = "";
        internal string _date = null;
        internal string _time = null;

        internal static object _globalMutex;

        static LoggerI()
        {
            _globalMutex = new object();
        }
    }

    public sealed class ConsoleLoggerI : LoggerI
    {
        public ConsoleLoggerI(string prefix)
            : base(prefix)
        {
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
                Trace.Listeners.Add(new TextWriterTraceListener(file));
            }

            if(console && !Trace.Listeners.Contains(_consoleListener))
            {
                Trace.Listeners.Add(_consoleListener);
            }
        }

        protected override void write(string message)
        {
            Trace.WriteLine(message);
            Trace.Flush();
        }

        internal static ConsoleTraceListener _consoleListener = new ConsoleTraceListener(true);
    }
}
