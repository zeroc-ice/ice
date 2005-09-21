// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{

    public abstract class Exception : System.ApplicationException
    {
        public Exception() {}
        public Exception(string msg) : base(msg) {}
        public Exception(System.Exception ex) : base(_dflt, ex) {}
        public Exception(string msg, System.Exception ex) : base(msg, ex) {}
        private static readonly string _dflt = "Ice.Exception";
    }

    public abstract class LocalException : Exception
    {
        public LocalException() {}
        public LocalException(string msg) : base(msg) {}
        public LocalException(System.Exception ex) : base(_dflt, ex) {}
        public LocalException(string msg, System.Exception ex) : base(msg, ex) {}
        private static readonly string _dflt = "Ice.LocalException";
    }

    public abstract class UserException : Exception
    {
        public UserException() {}
        public UserException(string msg) : base(msg) {}
        public UserException(System.Exception ex) : base(_dflt, ex) {}
        public UserException(string msg, System.Exception ex) : base(msg, ex) {}
        private static readonly string _dflt = "Ice.UserException";
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

    public class NonRepeatable : Ice.Exception
    {
        public NonRepeatable(Ice.LocalException ex)
        {
            _ex = ex;
        }
	
        public virtual Ice.LocalException get()
        {
            return _ex;
        }
	
        private Ice.LocalException _ex;
    }

}
