// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Ice
{
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
    public interface Object : System.ICloneable
    {
        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        ///
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>True if this object has the interface
        /// specified by s or derives from the interface specified by s.</returns>
        bool ice_isA(string s, Current current = null);

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        void ice_ping(Current current = null);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids(Current current = null);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id(Current current = null);

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <returns>The task if dispatched asynchronously, null otherwise.</returns>
        Task<Ice.OutputStream> ice_dispatch(Request request);

        Task<Ice.OutputStream> iceDispatch(IceInternal.Incoming inc, Current current);
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

        public static readonly string[] s_iceIds =
        {
            "::Ice::Object"
        };

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The return value is true if s is ::Ice::Object.</returns>
        public virtual bool ice_isA(string s, Current current = null)
        {
            return s.Equals(s_iceIds[0]);
        }

        public static Task<Ice.OutputStream> iceD_ice_isA(Ice.Object iceObj, IceInternal.Incoming iceInS, Current iceCurrent)
        {
            InputStream iceIs = iceInS.startReadParams();
            var iceId = iceIs.readString();
            iceInS.endReadParams();
            var iceRet = iceObj.ice_isA(iceId, iceCurrent);
            var iceOs = iceInS.startWriteParams();
            iceOs.writeBool(iceRet);
            iceInS.endWriteParams(iceOs);
            iceInS.setResult(iceOs);
            return null;
        }

        /// <summary>
        /// Tests whether this object can be reached.
        /// <param name="current">The Current object for the invocation.</param>
        /// </summary>
        public virtual void ice_ping(Current current = null)
        {
            // Nothing to do.
        }

        public static Task<Ice.OutputStream> iceD_ice_ping(Ice.Object iceObj, IceInternal.Incoming iceInS, Current iceCurrent)
        {
            iceInS.readEmptyParams();
            iceObj.ice_ping(iceCurrent);
            iceInS.setResult(iceInS.writeEmptyParams());
            return null;
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>An array whose only element is ::Ice::Object.</returns>
        public virtual string[] ice_ids(Current current = null)
        {
            return s_iceIds;
        }

        public static Task<Ice.OutputStream> iceD_ice_ids(Ice.Object iceObj, IceInternal.Incoming iceInS, Current iceCurrent)
        {
            iceInS.readEmptyParams();
            var iceRet = iceObj.ice_ids(iceCurrent);
            var iceOs = iceInS.startWriteParams();
            iceOs.writeStringSeq(iceRet);
            iceInS.endWriteParams(iceOs);
            iceInS.setResult(iceOs);
            return null;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the invocation.</param>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public virtual string ice_id(Current current = null)
        {
            return s_iceIds[0];
        }

        public static Task<Ice.OutputStream> iceD_ice_id(Ice.Object iceObj, IceInternal.Incoming iceInS, Current iceCurrent)
        {
            iceInS.readEmptyParams();
            var iceRet = iceObj.ice_id(iceCurrent);
            var iceOs = iceInS.startWriteParams();
            iceOs.writeString(iceRet);
            iceInS.endWriteParams(iceOs);
            iceInS.setResult(iceOs);
            return null;
        }

        /// <summary>
        /// Returns the Slice type ID of the interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public static string ice_staticId()
        {
            return s_iceIds[0];
        }

        private static readonly string[] s_iceAll = new string[]
        {
            "ice_id", "ice_ids", "ice_isA", "ice_ping"
        };

        /// <summary>
        /// Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
        /// to a servant (or to another interceptor).
        /// </summary>
        /// <param name="request">The details of the invocation.</param>
        /// <returns>The task if dispatched asynchronously, null otherwise.</returns>
        public virtual Task<Ice.OutputStream> ice_dispatch(Request request)
        {
            var inc = (IceInternal.Incoming)request;
            inc.startOver();
            return iceDispatch(inc, inc.getCurrent());
        }

        public virtual Task<Ice.OutputStream> iceDispatch(IceInternal.Incoming inc, Current current)
        {
            int pos = System.Array.BinarySearch(s_iceAll, current.operation);
            if(pos < 0)
            {
                throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
            }

            switch(pos)
            {
                case 0:
                {
                    return iceD_ice_id(this, inc, current);
                }
                case 1:
                {
                    return iceD_ice_ids(this, inc, current);
                }
                case 2:
                {
                    return iceD_ice_isA(this, inc, current);
                }
                case 3:
                {
                    return iceD_ice_ping(this, inc, current);
                }
            }

            Debug.Assert(false);
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
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

        public static void iceCheckMode(OperationMode expected, OperationMode received)
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

        public override Task<Ice.OutputStream> iceDispatch(IceInternal.Incoming inS, Current current)
        {
            byte[] inEncaps = inS.readParamEncaps();
            byte[] outEncaps;
            bool ok = ice_invoke(inEncaps, out outEncaps, current);
            inS.setResult(inS.writeParamEncaps(outEncaps, ok));
            return null;
        }
    }

    public abstract class BlobjectAsync : Ice.ObjectImpl
    {
        public abstract Task<Ice.Object_Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current);

        public override Task<Ice.OutputStream> iceDispatch(IceInternal.Incoming inS, Current current)
        {
            byte[] inEncaps = inS.readParamEncaps();
            return ice_invokeAsync(inEncaps, current).ContinueWith((Task<Ice.Object_Ice_invokeResult> t) =>
            {
                var ret = t.GetAwaiter().GetResult();
                return Task.FromResult<Ice.OutputStream>(inS.writeParamEncaps(ret.outEncaps, ret.returnValue));
            }).Unwrap();
        }
    }
}
