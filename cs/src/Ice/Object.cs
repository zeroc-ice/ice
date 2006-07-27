// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Diagnostics;

namespace IceInternal
{

    public enum DispatchStatus 
    {
        DispatchOK,
        DispatchUserException,
        DispatchObjectNotExist,
        DispatchFacetNotExist,
        DispatchOperationNotExist,
        DispatchUnknownLocalException,
        DispatchUnknownUserException,
        DispatchUnknownException,
        DispatchAsync
    };

}

namespace Ice
{

    public interface Object : System.ICloneable
    {
	int ice_hash();

        bool ice_isA(string s);
	bool ice_isA(string s, Current current);

        void ice_ping();
	void ice_ping(Current current);

        string[] ice_ids();
	string[] ice_ids(Current current);

        string ice_id();
	string ice_id(Current current);

	void ice_preMarshal();
	void ice_postUnmarshal();

	IceInternal.DispatchStatus dispatch__(IceInternal.Incoming inc, Current current);

	void write__(IceInternal.BasicStream os__);
	void read__(IceInternal.BasicStream is__, bool rid__);

	void write__(OutputStream outS__);
	void read__(InputStream inS__, bool rid__);
    }

    public abstract class ObjectImpl : Object
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

        public static readonly string[] ids__ = new string[] { "::Ice::Object" };
	
        public virtual bool ice_isA(string s)
        {
            return s.Equals(ids__[0]);
        }

        public virtual bool ice_isA(string s, Current current)
        {
            return s.Equals(ids__[0]);
        }
	
        public static IceInternal.DispatchStatus ice_isA___(Ice.Object __obj, IceInternal.Incoming inS__,
            Current __current)
        {
            IceInternal.BasicStream is__ = inS__.istr();
            IceInternal.BasicStream os__ = inS__.ostr();
            string __id = is__.readString();
            bool __ret = __obj.ice_isA(__id, __current);
            os__.writeBool(__ret);
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
	
        public static IceInternal.DispatchStatus ice_ping___(Ice.Object __obj, IceInternal.Incoming inS__,
            Current __current)
        {
            __obj.ice_ping(__current);
            return IceInternal.DispatchStatus.DispatchOK;
        }
	
        public virtual string[] ice_ids()
        {
            return ids__;
        }

        public virtual string[] ice_ids(Current current)
        {
            return ids__;
        }
	
        public static IceInternal.DispatchStatus ice_ids___(Ice.Object __obj, IceInternal.Incoming inS__,
            Current __current)
        {
            IceInternal.BasicStream os__ = inS__.ostr();
            os__.writeStringSeq(__obj.ice_ids(__current));
            return IceInternal.DispatchStatus.DispatchOK;
        }
	
        public virtual string ice_id()
        {
            return ids__[0];
        }

        public virtual string ice_id(Current current)
        {
            return ids__[0];
        }
	
        public static IceInternal.DispatchStatus ice_id___(Ice.Object __obj, IceInternal.Incoming inS__,
            Current __current)
        {
            IceInternal.BasicStream os__ = inS__.ostr();
            string __ret = __obj.ice_id(__current);
            os__.writeString(__ret);
            return IceInternal.DispatchStatus.DispatchOK;
        }
	
        public static string ice_staticId()
        {
            return ids__[0];
        }
	
        public virtual void ice_preMarshal()
        {
        }

        public virtual void ice_postUnmarshal()
        {
        }

        private static readonly string[] all__ = new string[]
        {
            "ice_id", "ice_ids", "ice_isA", "ice_ping"
        };
	
        public virtual IceInternal.DispatchStatus dispatch__(IceInternal.Incoming inc, Current current)
        {
            int pos = System.Array.BinarySearch(all__, current.operation);
            if(pos < 0)
            {
                return IceInternal.DispatchStatus.DispatchOperationNotExist;
            }
	    
            switch(pos)
            {
                case 0: 
                {
                    return ice_id___(this, inc, current);
                }
                case 1: 
                {
                    return ice_ids___(this, inc, current);
                }
                case 2: 
                {
                    return ice_isA___(this, inc, current);
                }
                case 3: 
                {
                    return ice_ping___(this, inc, current);
                }
            }
	    
            Debug.Assert(false);
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }
	
        public virtual void write__(IceInternal.BasicStream os__)
        {
            os__.writeTypeId(ice_staticId());
            os__.startWriteSlice();
            os__.writeSize(0); // For compatibility with the old AFM.  
            os__.endWriteSlice();
        }
	
        public virtual void read__(IceInternal.BasicStream is__, bool rid__)
        {

            if(rid__)
            {
                /* string myId = */ is__.readTypeId();
            }
	    
            is__.startReadSlice();
	    
            // For compatibility with the old AFM.
            int sz = is__.readSize();
            if(sz != 0)
            {
                throw new MarshalException();
            }
	    
            is__.endReadSlice();
        }

	public virtual void write__(OutputStream outS__)
	{
	    outS__.writeTypeId(ice_staticId());
	    outS__.startSlice();
	    outS__.writeSize(0); // For compatibility with the old AFM.
	    outS__.endSlice();
	}

	public virtual void read__(InputStream inS__, bool rid__)
	{
	    if(rid__)
	    {
		/* string myId = */ inS__.readTypeId();
	    }

	    inS__.startSlice();

	    // For compatibility with the old AFM.
	    int sz = inS__.readSize();
	    if(sz != 0)
	    {
		throw new MarshalException();
	    }

	    inS__.endSlice();
	}

	private static string
	operationModeToString(OperationMode mode)
	{
	    if(mode == Ice.OperationMode.Normal)
	    {
		return "::Ice::Normal";
	    }
	    if(mode == Ice.OperationMode.Nonmutating)
	    {
		return "::Ice::Nonmutating";
	    }

	    if(mode == Ice.OperationMode.Idempotent)
	    {
		return "::Ice::Idempotent";
	    }

	    return "???";
	}

	protected static void
	checkMode__(OperationMode expected, OperationMode received)
	{
	    if(expected != received)
	    {
		if(expected == OperationMode.Idempotent 
		   && received == OperationMode.Nonmutating)
		{
		    //
		    // Fine: typically an old client still using the 
		    // deprecated nonmutating keyword
		    //
		}
		else
		{
		    Ice.MarshalException ex = new Ice.MarshalException();
		    ex.reason = "unexpected operation mode. expected = "
			+ operationModeToString(expected) + " received = "
			+ operationModeToString(received);
		    throw ex;
		}
	    }
	}
        public static Ice.Current defaultCurrent = new Ice.Current();
    }

    public abstract class Blobject : Ice.ObjectImpl
    {
        // Returns true if ok, false if user exception.
        public abstract bool ice_invoke(byte[] inParams, out byte[] outParams, Current current);
	
        public override IceInternal.DispatchStatus dispatch__(IceInternal.Incoming inc, Current current)
        {
            byte[] inParams;
            byte[] outParams;
            int sz = inc.istr().getReadEncapsSize();
            inParams = inc.istr().readBlob(sz);
            bool ok = ice_invoke(inParams, out outParams, current);
            if(outParams != null)
            {
                inc.ostr().writeBlob(outParams);
            }
            if(ok)
            {
                return IceInternal.DispatchStatus.DispatchOK;
            }
            else
            {
                return IceInternal.DispatchStatus.DispatchUserException;
            }
        }
    }

    public abstract class BlobjectAsync : Ice.ObjectImpl
    {
        public abstract void ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inParams, Current current);
	
        public override IceInternal.DispatchStatus dispatch__(IceInternal.Incoming inc, Current current)
        {
            byte[] inParams;
            int sz = inc.istr().getReadEncapsSize();
            inParams = inc.istr().readBlob(sz);
            AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(inc);
            try
            {
                ice_invoke_async(cb, inParams, current);
            }
            catch(System.Exception ex)
            {
                cb.ice_exception(ex);
            }
            return IceInternal.DispatchStatus.DispatchAsync;
        }
    }

}
