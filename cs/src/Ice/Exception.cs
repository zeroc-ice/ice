// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{

    public abstract class Exception : System.ApplicationException, System.ICloneable
    {
        public object Clone()
        {
            return MemberwiseClone();
        }

        public Exception() {}
        public Exception(System.Exception ex) : base("", ex) {}
        public override string ToString()
        {
            //
            // This prints the exception Java style. That is, the outermost
            // exception, "Caused by:" to the innermost exception. The
            // stack trace is not nicely indented as with Java, but
            // without string parsing (perhaps tokenize on "\n"), it
            // doesn't appear to be possible to reformat it.
            //
            System.IO.StringWriter sw = new System.IO.StringWriter();
            IceUtil.OutputBase op = new IceUtil.OutputBase(sw);
            op.setUseTab(false);
            op.print(GetType().FullName);
            op.inc();
            IceInternal.ValueWriter.write(this, op);
            sw.Write("\n");
            sw.Write(StackTrace);

            System.Exception curr = InnerException;
            while(curr != null)
            {
                sw.Write("\nCaused by: ");
                sw.Write(curr.GetType().FullName);
                if(!(curr is Ice.Exception))
                {
                    sw.Write(": ");
                    sw.Write(curr.Message);
                }
                sw.Write("\n");
                sw.Write(curr.StackTrace);
                curr = curr.InnerException;
            }

            return sw.ToString();
        }
    }

    public abstract class LocalException : Exception
    {
        public LocalException() {}
        public LocalException(System.Exception ex) : base(ex) {}
    }

    public abstract class UserException : Exception
    {
        public UserException() {}
        public UserException(System.Exception ex) : base(ex) {}
        public abstract void write__(IceInternal.BasicStream os__);
        public abstract void read__(IceInternal.BasicStream is__, bool rid__);

        public virtual void write__(Ice.OutputStream outS__)
        {
            Debug.Assert(false);
        }

        public virtual void read__(Ice.InputStream inS__, bool rid__)
        {
            Debug.Assert(false);
        }

        public virtual bool usesClasses__()
        {
            return false;
        }
    }

}

namespace IceInternal
{

    public class LocalExceptionWrapper : Ice.Exception
    {
        public LocalExceptionWrapper(Ice.LocalException ex, bool retry)
        {
            _ex = ex;
            _retry = retry;
        }

        public LocalExceptionWrapper(LocalExceptionWrapper ex)
        {
            _ex = ex.get();
            _retry = ex._retry;
        }

        public Ice.LocalException get()
        {
            return _ex;
        }

        //
        // If true, always repeat the request. Don't take retry settings
        // or "at-most-once" guarantees into account.
        //
        // If false, only repeat the request if the retry settings allow
        // to do so, and if "at-most-once" does not need to be guaranteed.
        //
        public bool
        retry()
        {
            return _retry;
        }
        
        private Ice.LocalException _ex;
        private bool _retry;
    }

}
