// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System.Globalization;

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
                try
                {
                    _out = System.IO.File.AppendText(file);
                }
                catch(System.IO.IOException)
                {
                    throw new Ice.InitializationException("FileLogger: cannot open " + file);
                }
                _out.AutoFlush = true;
            }
        }

        ~LoggerI()
        {
            if(_out != null)
            {
                try
                {
                    _out.Close();
                }
                catch(System.Exception)
                {
                }
            }
        }
        
        public void print(string message)
        {
            write(message);
        }
        
        public void trace(string category, string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder("[ ");
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
            s.Append(' ');
            s.Append(_prefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Append(" ]");
            s.Replace("\n", "\n  ");
            write(s.ToString());
        }
        
        public void warning(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("warning: ");
            s.Append(message);
            write(s.ToString());
        }
        
        public void error(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(System.DateTime.Now.ToString(_date));
            s.Append(' ');
            s.Append(System.DateTime.Now.ToString(_time));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("error: ");
            s.Append(message);
            write(s.ToString());
        }

        private void write(string message)
        {
            lock(_globalMutex)
            {
                if(_out == null)
                {
                    System.Console.Error.WriteLine(message);
                }
                else
                {
                    _out.WriteLine(message);
                }
            }
        }
        
        internal string _prefix = "";
        internal string _date = null;
        internal string _time = null;
        internal System.IO.StreamWriter _out = null;

        internal static object _globalMutex;
        static LoggerI()
        {
            _globalMutex = new object();
        }
    }
}
