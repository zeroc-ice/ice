// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
	
    using System.Collections;
    using System.Diagnostics;

    public class ObjectImpl : Object
    {
	public ObjectImpl()
	{
	}

	public virtual int ice_hash()
	{
	    return GetHashCode();
	}

	public object Clone()
	{
	    return MemberwiseClone();
	}

	public static readonly string[] __ids = new string[] { "::Ice::Object" };
	
        public virtual bool ice_isA(string s)
        {
            return s.Equals(__ids[0]);
        }

	public virtual bool ice_isA(string s, Current current)
	{
	    return s.Equals(__ids[0]);
	}
	
	public static IceInternal.DispatchStatus ___ice_isA(Ice.Object __obj, IceInternal.Incoming __in,
	                                                    Current __current)
	{
	    IceInternal.BasicStream __is = __in.istr();
	    IceInternal.BasicStream __os = __in.ostr();
	    string __id = __is.readString();
	    bool __ret = __obj.ice_isA(__id, __current);
	    __os.writeBool(__ret);
	    return IceInternal.DispatchStatus.DispatchOK;
	}
	
        public virtual void ice_ping()
        {
            // Nothing to do.
        }

	public virtual void ice_ping(Current current)
	{
	    // Nothing to do.
	}
	
	public static IceInternal.DispatchStatus ___ice_ping(Ice.Object __obj, IceInternal.Incoming __in,
	                                                     Current __current)
	{
	    __obj.ice_ping(__current);
	    return IceInternal.DispatchStatus.DispatchOK;
	}
	
        public virtual string[] ice_ids()
        {
            return __ids;
        }

	public virtual string[] ice_ids(Current current)
	{
	    return __ids;
	}
	
	public static IceInternal.DispatchStatus ___ice_ids(Ice.Object __obj, IceInternal.Incoming __in,
	                                                    Current __current)
	{
	    IceInternal.BasicStream __os = __in.ostr();
	    __os.writeStringSeq(__obj.ice_ids(__current));
	    return IceInternal.DispatchStatus.DispatchOK;
	}
	
        public virtual string ice_id()
        {
            return __ids[0];
        }

	public virtual string ice_id(Current current)
	{
	    return __ids[0];
	}
	
	public static IceInternal.DispatchStatus ___ice_id(Ice.Object __obj, IceInternal.Incoming __in,
	                                                   Current __current)
	{
	    IceInternal.BasicStream __os = __in.ostr();
	    string __ret = __obj.ice_id(__current);
	    __os.writeString(__ret);
	    return IceInternal.DispatchStatus.DispatchOK;
	}
	
	public static string ice_staticId()
	{
	    return __ids[0];
	}
	
	public virtual void ice_preMarshal()
	{
	}

	public virtual void ice_postUnmarshal()
	{
	}

	private static readonly string[] __all = new string[]
	{
	    "ice_id", "ice_ids", "ice_isA", "ice_ping"
	};
	
	public virtual IceInternal.DispatchStatus __dispatch(IceInternal.Incoming inc, Current current)
	{
	    int pos = System.Array.BinarySearch(__all, current.operation);
	    if(pos < 0)
	    {
		return IceInternal.DispatchStatus.DispatchOperationNotExist;
	    }
	    
	    switch(pos)
	    {
		case 0: 
		{
			return ___ice_id(this, inc, current);
		}
		case 1: 
		{
			return ___ice_ids(this, inc, current);
		}
		case 2: 
		{
			return ___ice_isA(this, inc, current);
		}
		case 3: 
		{
			return ___ice_ping(this, inc, current);
		}
	    }
	    
	    Debug.Assert(false);
	    return IceInternal.DispatchStatus.DispatchOperationNotExist;
	}
	
	public virtual void __write(IceInternal.BasicStream __os)
	{
	    __os.writeTypeId(ice_staticId());
	    __os.startWriteSlice();
            __os.writeSize(0); // For compatibility with the old AFM.  
	    __os.endWriteSlice();
	}
	
	public virtual void __read(IceInternal.BasicStream __is, bool __rid)
	{

	    if(__rid)
	    {
		string myId = __is.readTypeId();
	    }
	    
	    __is.startReadSlice();
	    
            // For compatibility with the old AFM.
	    int sz = __is.readSize();
            if(sz != 0)
            {
                throw new MarshalException();
            }
	    
	    __is.endReadSlice();
	}
    }

}
