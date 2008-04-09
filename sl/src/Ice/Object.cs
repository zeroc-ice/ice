// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Diagnostics;

namespace Ice
{
    public enum DispatchStatus 
    {
        DispatchOK,
        DispatchUserException,
        DispatchAsync
    };

    public interface Object : System.ICloneable
    {
        int ice_hash();

        bool ice_isA(string s);

        void ice_ping();

        string[] ice_ids();

        string ice_id();

        void ice_preMarshal();
        void ice_postUnmarshal();

        void write__(IceInternal.BasicStream os__);
        void read__(IceInternal.BasicStream is__, bool rid__);
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

        public virtual void ice_ping()
        {
            // Nothing to do.
        }

        public virtual string[] ice_ids()
        {
            return ids__;
        }

        public virtual string ice_id()
        {
            return ids__[0];
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
    }
}
