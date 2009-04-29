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

    using System.Globalization;
    using System.Diagnostics;

    public sealed class LoggerI : Logger
    {
        public LoggerI(string prefix, string file)
        {
            if(prefix.Length > 0)
            {
                _prefix = prefix + ": ";
            }
            
            _date = "d";
            _time = "HH:mm:ss:fff";

            if(file.Length != 0)
            {
                Trace.Listeners.Add(new TextWriterTraceListener(file));
            }

            if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono)
            {
                //
                // COMPILERFIX: With Mono if a write is not done before the TraceSwitch
                // members are accessed then the ConsoleTraceListener double prints.
                //
                Trace.Write("");
            }
        }

        public void print(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder(message);
            write(s, false, _switch.TraceInfo);
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
            write(s, true, _switch.TraceInfo);
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
            write(s, true, _switch.TraceWarning);
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
            write(s, true, _switch.TraceError);
        }

        private void write(System.Text.StringBuilder message, bool indent, bool condition)
        {
            lock(_globalMutex)
            {
                if(indent)
                {
                    message.Replace("\n", "\n   ");
                }

                Trace.WriteLineIf(condition, message);
            }
        }
        
        internal string _prefix = "";
        internal string _date = null;
        internal string _time = null;

        internal static object _globalMutex;
        internal static TraceSwitch _switch;
        static LoggerI()
        {
            _globalMutex = new object();
            if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono)
            {
                _switch = new TraceSwitch("IceLogger", "Ice Logger Switch");
            }
            else
            {
                _switch = new TraceSwitch("IceLogger", "Ice Logger Switch", "Info");
            }
        }

    }
}
