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
        public abstract void __write(IceInternal.BasicStream __os);
        public abstract void __read(IceInternal.BasicStream __is, bool __rid);

	public virtual void __write(Ice.OutputStream __outS)
	{
	    Debug.Assert(false);
	}

	public virtual void __read(Ice.InputStream __inS, bool __rid)
	{
	    Debug.Assert(false);
	}

	public virtual bool __usesClasses()
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
