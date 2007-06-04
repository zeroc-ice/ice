// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System.Globalization;

    public sealed class LoggerI :  Logger
    {
        public LoggerI(string prefix)
        {
            if(prefix.Length > 0)
            {
                _prefix = prefix + ": ";
            }
            
            _date = "G";
        }
        
        public void print(string message)
        {
            lock(_globalMutex)
            {
                System.Console.Error.WriteLine(message);
            }
        }

        public void trace(string category, string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder("[ ");
            s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
            s.Append(' ');
            s.Append(_prefix);
            s.Append(category);
            s.Append(": ");
            s.Append(message);
            s.Append(" ]");
            s.Replace("\n", "\n  ");

            lock(_globalMutex)
            {
                System.Console.Error.WriteLine(s.ToString());
            }
        }
        
        public void warning(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("warning: ");
            s.Append(message);

            lock(_globalMutex)
            {
                System.Console.Error.WriteLine(s.ToString());
            }
        }
        
        public void error(string message)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(System.DateTime.Now.ToString(_date, DateTimeFormatInfo.InvariantInfo));
            s.Append(' ');
            s.Append(_prefix);
            s.Append("error: ");
            s.Append(message);

            lock(_globalMutex)
            {
                System.Console.Error.WriteLine(s.ToString());
            }
        }
        
        internal string _prefix = "";
        internal static object _globalMutex;
        internal string _date = null;
        static LoggerI()
        {
            _globalMutex = new object();
        }
    }
}
