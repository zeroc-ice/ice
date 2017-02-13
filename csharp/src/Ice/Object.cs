// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// Indicates the status of operation dispatch.
    /// </summary>
    public enum DispatchStatus
    {
        /// <summary>
        /// Indicates that an operation was dispatched synchronously and successfully.
        /// </summary>
        DispatchOK,

        /// <summary>
        /// Indicates that an operation was dispatched synchronously and raised a user exception.
        /// </summary>
        DispatchUserException,

        /// <summary>
        /// Indicates that an operation was dispatched asynchronously.
        /// </summary>
        DispatchAsync
    }

    public interface DispatchInterceptorAsyncCallback
    {
        bool response(bool ok);
        bool exception(System.Exception ex);
    }

    /// <summary>
    /// Interface for incoming requests.
    /// </summary>
    public interface Request
    {
        /// <summary>
        /// Returns the {@link Current} object for this the request.
        /// </summary>
        /// <returns>The Current object for this request.</returns>
        Current getCurrent();
    }

    /// <summary>
    /// the base interface for servants.
    /// </summary>
    public interface Object :    System.ICloneable
    {
        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        ///
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <returns>True if this object has the interface
        /// specified by s or derives from the interface specified by s.</returns>
        bool ice_isA(string s);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        ///
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>True if this object has the interface
        /// specified by s or derives from the interface specified by s.</returns>
        bool ice_isA(string s, Current current);

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        void ice_ping();

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        void ice_ping(Current current);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids();

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids(Current current);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id();

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id(Current current);

        /// <summary>
        /// The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
        /// to override this method in order to validate its data members.
        /// </summary>
        void ice_preMarshal();

        /// <summary>
        /// This Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
        /// subclass to override this method in order to perform additional initialization.
        /// </summary>
        void ice_postUnmarshal();

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <param name="cb">The callback object for asynchchronous dispatch. For synchronous dispatch,
        /// the callback object must be null.</param>
        /// <returns>The dispatch status for the operation.</returns>
        DispatchStatus ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb);

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <returns>The dispatch status for the operation.</returns>
        DispatchStatus ice_dispatch(Request request);

        DispatchStatus dispatch__(IceInternal.Incoming inc, Current current);

        void write__(IceInternal.BasicStream os__);
        void read__(IceInternal.BasicStream is__);
       
        void write__(OutputStream outS__);
        void read__(InputStream inS__);
    }

    /// <summary>
    /// Base class for all Slice classes.
    /// </summary>
    public abstract class ObjectImpl : Object
    {
        /// <summary>
        /// Instantiates an Ice object.
        /// </summary>
        public ObjectImpl()
        {
        }

        /// <summary>
        /// Returns a copy of the object. The cloned object contains field-for-field copies
        /// of the state.
        /// </summary>
        /// <returns>The cloned object.</returns>
        public object Clone()
        {
            return MemberwiseClone();
        }

        public static readonly string[] ids__ =
        {
            "::Ice::Object"
        };

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <returns>The return value is true if s is ::Ice::Object.</returns>
        public virtual bool ice_isA(string s)
        {
            return s.Equals(ids__[0]);
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The return value is true if s is ::Ice::Object.</returns>
        public virtual bool ice_isA(string s, Current current)
        {
            return s.Equals(ids__[0]);
        }

        public static DispatchStatus ice_isA___(Ice.Object __obj, IceInternal.Incoming inS__, Current __current)
        {
            IceInternal.BasicStream is__ = inS__.startReadParams();
            string __id = is__.readString();
            inS__.endReadParams();
            bool __ret = __obj.ice_isA(__id, __current);
            IceInternal.BasicStream os__ = inS__.startWriteParams__(FormatType.DefaultFormat);
            os__.writeBool(__ret);
            inS__.endWriteParams__(true);
            return DispatchStatus.DispatchOK;
        }

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        public virtual void ice_ping()
        {
            // Nothing to do.
        }

        /// <summary>
        /// Tests whether this object can be reached.
        /// <param name="current">The Current object for the invocation.</param>
        /// </summary>
        public virtual void ice_ping(Current current)
        {
            // Nothing to do.
        }

        public static DispatchStatus ice_ping___(Ice.Object __obj, IceInternal.Incoming inS__, Current __current)
        {
            inS__.readEmptyParams();
            __obj.ice_ping(__current);
            inS__.writeEmptyParams__();
            return DispatchStatus.DispatchOK;
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <returns>An array whose only element is ::Ice::Object.</returns>
        public virtual string[] ice_ids()
        {
            return ids__;
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>An array whose only element is ::Ice::Object.</returns>
        public virtual string[] ice_ids(Current current)
        {
            return ids__;
        }

        public static DispatchStatus ice_ids___(Ice.Object __obj, IceInternal.Incoming inS__, Current __current)
        {
            inS__.readEmptyParams();
            string[] ret__ = __obj.ice_ids(__current);
            IceInternal.BasicStream os__ = inS__.startWriteParams__(FormatType.DefaultFormat);
            os__.writeStringSeq(ret__);
            inS__.endWriteParams__(true);
            return DispatchStatus.DispatchOK;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public virtual string ice_id()
        {
            return ids__[0];
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public virtual string ice_id(Current current)
        {
            return ids__[0];
        }

        public static DispatchStatus ice_id___(Ice.Object __obj, IceInternal.Incoming inS__, Current __current)
        {
            inS__.readEmptyParams();
            string __ret = __obj.ice_id(__current);
            IceInternal.BasicStream os__ = inS__.startWriteParams__(FormatType.DefaultFormat);
            os__.writeString(__ret);
            inS__.endWriteParams__(true);
            return DispatchStatus.DispatchOK;
        }

        /// <summary>
        /// Returns the Slice type ID of the interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public static string ice_staticId()
        {
            return ids__[0];
        }

        /// <summary>
        /// The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
        /// to override this method in order to validate its data members.
        /// </summary>
        public virtual void ice_preMarshal()
        {
        }

        /// <summary>
        /// This Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
        /// subclass to override this method in order to perform additional initialization.
        /// </summary>
        public virtual void ice_postUnmarshal()
        {
        }

        private static readonly string[] all__ = new string[]
        {
            "ice_id", "ice_ids", "ice_isA", "ice_ping"
        };

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <param name="cb">The callback object for asynchchronous dispatch. For synchronous dispatch, the
        /// callback object must be null.</param>
        /// <returns>The dispatch status for the operation.</returns>
        public virtual DispatchStatus ice_dispatch(Request request, DispatchInterceptorAsyncCallback cb)
        {
            IceInternal.Incoming inc = (IceInternal.Incoming)request;
            if(cb != null)
            {
                inc.push(cb);
            }
            try
            {
                inc.startOver(); // may raise ResponseSentException
                return dispatch__(inc, inc.getCurrent());
            }
            finally
            {
                if(cb != null)
                {
                    inc.pop();
                }
            }
        }

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <returns>The dispatch status for the operation.</returns>
        public virtual DispatchStatus ice_dispatch(Request request)
        {
            return ice_dispatch(request, null);
        }

        public virtual DispatchStatus dispatch__(IceInternal.Incoming inc, Current current)
        {
            int pos = System.Array.BinarySearch(all__, current.operation);
            if(pos < 0)
            {
                throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
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
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
        }

        public virtual void write__(IceInternal.BasicStream os__)
        {
            os__.startWriteObject(null);
            writeImpl__(os__);
            os__.endWriteObject();
        }

        public virtual void read__(IceInternal.BasicStream is__)
        {
             is__.startReadObject();
             readImpl__(is__);
             is__.endReadObject(false);
        }

        public virtual void write__(OutputStream os__)
        {
            os__.startObject(null);
            writeImpl__(os__);
            os__.endObject();
        }

        public virtual void read__(InputStream is__)
        {
             is__.startObject();
             readImpl__(is__);
             is__.endObject(false);
        }

        protected virtual void writeImpl__(IceInternal.BasicStream os__)
        {
        }

        protected virtual void readImpl__(IceInternal.BasicStream is__)
        {
        }

        protected virtual void writeImpl__(OutputStream os__)
        {
            throw new MarshalException("class was not generated with stream support");
        }

        protected virtual void readImpl__(InputStream is__)
        {
            throw new MarshalException("class was not generated with stream support");
        }

        private static string operationModeToString(OperationMode mode)
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

        public static void checkMode__(OperationMode expected, OperationMode received)
        {
            if(expected != received)
            {
                if(expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
                {
                    //
                    // Fine: typically an old client still using the
                    // deprecated nonmutating keyword
                    //
                }
                else
                {
                    Ice.MarshalException ex = new Ice.MarshalException();
                    ex.reason = "unexpected operation mode. expected = " + operationModeToString(expected) +
                        " received = " + operationModeToString(received);
                    throw ex;
                }
            }
        }

        public static Ice.Current defaultCurrent = new Ice.Current();
    }

    /// <summary>
    /// Base class for dynamic dispatch servants. A server application
    /// derives a concrete servant class from Blobject that
    /// implements the Blobject.ice_invoke method.
    /// </summary>
    public abstract class Blobject : Ice.ObjectImpl
    {
        /// <summary>
        /// Dispatch an incoming request.
        /// </summary>
        /// <param name="inParams">The encoded in-parameters for the operation.</param>
        /// <param name="outParams">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <param name="current">The Current object to pass to the operation.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outParams
        /// must contain the encoded user exception. If the operation raises an
        /// Ice run-time exception, it must throw it directly.</returns>
        public abstract bool ice_invoke(byte[] inParams, out byte[] outParams, Current current);

        public override DispatchStatus dispatch__(IceInternal.Incoming inS__, Current current)
        {
            byte[] inEncaps = inS__.readParamEncaps();
            byte[] outEncaps;
            bool ok = ice_invoke(inEncaps, out outEncaps, current);
            inS__.writeParamEncaps__(outEncaps, ok);
            if(ok)
            {
                return DispatchStatus.DispatchOK;
            }
            else
            {
                return DispatchStatus.DispatchUserException;
            }
        }
    }

    public abstract class BlobjectAsync : Ice.ObjectImpl
    {
        public abstract void ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inEncaps, Current current);

        public override DispatchStatus dispatch__(IceInternal.Incoming inS__, Current current)
        {
            byte[] inEncaps = inS__.readParamEncaps();
            AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(inS__);
            try
            {
                ice_invoke_async(cb, inEncaps, current);
            }
            catch(System.Exception ex)
            {
                cb.ice_exception(ex);
            }
            return DispatchStatus.DispatchAsync;
        }
    }
}
