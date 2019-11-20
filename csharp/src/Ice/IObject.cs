//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.ComponentModel;
using System.Threading.Tasks;
using IceInternal;

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
    public interface IObject
    {

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        ///
        /// <param name="s">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>True if this object has the interface
        /// specified by s or derives from the interface specified by s.</returns>
        public bool IceIsA(string s, Current current);

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        /// <param name="current">The Current object for the dispatch.</param>
        public void IcePing(Current current);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(Current current);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Current current);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static Task<OutputStream> iceD_ice_isA(IObject obj, IceInternal.Incoming inS, Current current)
        {
            InputStream istr = inS.startReadParams();
            var id = istr.readString();
            inS.endReadParams();
            var ret = obj.IceIsA(id, current);
            var ostr = inS.startWriteParams();
            ostr.writeBool(ret);
            inS.endWriteParams(ostr);
            inS.setResult(ostr);
            return null;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static Task<OutputStream> iceD_ice_ping(IObject obj, IceInternal.Incoming inS, Current current)
        {
            inS.readEmptyParams();
            obj.IcePing(current);
            inS.setResult(inS.writeEmptyParams());
            return null;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static Task<OutputStream> iceD_ice_ids(IObject obj, IceInternal.Incoming inS, Current current)
        {
            inS.readEmptyParams();
            var ret = obj.IceIds(current);
            var ostr = inS.startWriteParams();
            ostr.writeStringSeq(ret);
            inS.endWriteParams(ostr);
            inS.setResult(ostr);
            return null;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static Task<OutputStream> iceD_ice_id(IObject obj, IceInternal.Incoming inS, Current current)
        {
            inS.readEmptyParams();
            var ret = obj.IceId(current);
            var ostr = inS.startWriteParams();
            ostr.writeString(ret);
            inS.endWriteParams(ostr);
            inS.setResult(ostr);
            return null;
        }

        private static string operationModeToString(OperationMode mode)
        {
            if (mode == OperationMode.Normal)
            {
                return "::Ice::Normal";
            }
            if (mode == OperationMode.Nonmutating)
            {
                return "::Ice::Nonmutating";
            }

            if (mode == OperationMode.Idempotent)
            {
                return "::Ice::Idempotent";
            }

            return "???";
        }

        public static void iceCheckMode(OperationMode expected, OperationMode received)
        {
            if (expected != received)
            {
                if (expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
                {
                    //
                    // Fine: typically an old client still using the
                    // deprecated nonmutating keyword
                    //
                }
                else
                {
                    MarshalException ex = new MarshalException();
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
    public abstract class Blobject
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public new Task<OutputStream> Dispatch(Incoming incoming, Current current)
        {
            incoming.startOver();
            byte[] inEncaps = incoming.readParamEncaps();
            byte[] outEncaps;
            bool ok = ice_invoke(inEncaps, out outEncaps, current);
            incoming.setResult(incoming.writeParamEncaps(incoming.getAndClearCachedOutputStream(), outEncaps, ok));
            return null;
        }
    }

    public abstract class BlobjectAsync
    {
        public abstract Task<Ice.Object_Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Task<OutputStream> Dispatch(Incoming incoming, Current current)
        {
            incoming.startOver();
            byte[] inEncaps = incoming.readParamEncaps();
            var task = ice_invokeAsync(inEncaps, current);
            var cached = incoming.getAndClearCachedOutputStream();
            return task.ContinueWith((Task<Object_Ice_invokeResult> t) =>
                {
                    var ret = t.GetAwaiter().GetResult();
                    return Task.FromResult(incoming.writeParamEncaps(cached, ret.outEncaps, ret.returnValue));
                },
                TaskScheduler.Current).Unwrap();
        }
    }

    public interface IInterfaceTraits<T>
    {
        string Id { get; }
        string[] Ids { get; }
        Task<OutputStream> Dispatch(T servant, Incoming incoming, Current current);
    }

    public class Object<T, Traits> : IObject where Traits : struct, IInterfaceTraits<T>
    {
        public string IceId(Current current)
        {
            return _traits.Id;
        }

        public virtual bool IceIsA(string s, Current current)
        {
            return Array.BinarySearch(_traits.Ids, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;
        }

        public virtual void IcePing(Current current)
        {
            // Nothing to do
        }

        public virtual string[] IceIds(Current current)
        {
            return _traits.Ids;
        }

        private static Traits _traits = default;
    }
}
