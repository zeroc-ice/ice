//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Ice
{
    /// <summary>
    /// Interface for incoming requests.
    /// </summary>
    public interface IRequest
    {
        /// <summary>
        /// Returns the {@link Current} object for this the request.
        /// </summary>
        /// <returns>The Current object for this request.</returns>
        Current? GetCurrent();
    }

    ///<summary>The base interface for all servants.</summary>
    public interface IObject
    {
        // See Dispatcher.
        public Task<OutputStream>? Dispatch(Incoming inS, Current current)
        {
            // TODO: switch to abstract method
            Debug.Assert(false);
            return null;
        }

        // The following are helper methods for generated servants.

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        /// <param name="current">The Current object for the dispatch.</param>
        public void IcePing(Current current)
        {
            // Does nothing
        }

        /// <summary>Tests whether this object supports the specified Slice interface.</summary>
        /// <param name="typeId">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>True if this object implements the interface specified by typeId.</returns>
        public bool IceIsA(string typeId, Current current)
            => Array.BinarySearch(IceIds(current), typeId, StringComparer.Ordinal) >= 0;

        /// <summary>Returns the Slice type IDs of the interfaces supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in alphabetical order.</returns>
        public string[] IceIds(Current current) => new string[]{ "::Ice::Object" };

        /// <summary>Returns the Slice type ID of the most-derived interface supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Current current) => "::Ice::Object";

        protected static void IceCheckMode(OperationMode expected, OperationMode received)
        {
            if (expected != received)
            {
                if (expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
                {
                    // Fine: typically an old client still using the deprecated nonmutating keyword or metadata.
                }
                else
                {
                    throw new MarshalException(
                        $"unexpected operation mode. expected = {expected} received = {received}");
                }
            }
        }

        protected Task<OutputStream>? IceD_ice_ping(IceInternal.Incoming inS, Current current)
        {
            inS.ReadEmptyParams();
            IcePing(current);
            inS.SetResult(inS.WriteEmptyParams());
            return null;
        }

        protected Task<OutputStream>? IceD_ice_isA(IceInternal.Incoming inS, Current current)
        {
            InputStream istr = inS.StartReadParams();
            string id = istr.ReadString();
            inS.EndReadParams();
            bool ret = IceIsA(id, current);
            OutputStream ostr = inS.StartWriteParams();
            ostr.WriteBool(ret);
            inS.EndWriteParams(ostr);
            return inS.SetResult(ostr)!;
        }

        protected Task<OutputStream>? IceD_ice_id(IceInternal.Incoming inS, Current current)
        {
            inS.ReadEmptyParams();
            string ret = IceId(current);
            OutputStream ostr = inS.StartWriteParams();
            ostr.WriteString(ret);
            inS.EndWriteParams(ostr);
            inS.SetResult(ostr);
            return null;
        }

        protected Task<OutputStream>? IceD_ice_ids(IceInternal.Incoming inS, Current current)
        {
            inS.ReadEmptyParams();
            string[] ret = IceIds(current);
            OutputStream ostr = inS.StartWriteParams();
            ostr.WriteStringSeq(ret);
            inS.EndWriteParams(ostr);
            inS.SetResult(ostr);
            return null;
        }
    }

    /// <summary>
    /// Base class for dynamic dispatch servants. A server application
    /// derives a concrete servant class from Blobject that
    /// implements the Blobject.ice_invoke method.
    /// </summary>
    public abstract class Blobject : IObject
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
        public abstract bool IceInvoke(byte[] inParams, out byte[] outParams, Current current);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Task<OutputStream>? Dispatch(Incoming incoming, Current current)
        {
            incoming.StartOver();
            byte[] inEncaps = incoming.ReadParamEncaps();
            bool ok = IceInvoke(inEncaps, out byte[] outEncaps, current);
            incoming.SetResult(incoming.WriteParamEncaps(outEncaps, ok));
            return null;
        }
    }

    public abstract class BlobjectAsync : IObject
    {
        public abstract Task<Ice.Object_Ice_invokeResult> IceInvokeAsync(byte[] inEncaps, Current current);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Task<OutputStream>? Dispatch(Incoming incoming, Current current)
        {
            incoming.StartOver();
            byte[] inEncaps = incoming.ReadParamEncaps();
            Task<Object_Ice_invokeResult> task = IceInvokeAsync(inEncaps, current);
            return task.ContinueWith((Task<Object_Ice_invokeResult> t) =>
                {
                    Object_Ice_invokeResult ret = t.GetAwaiter().GetResult();
                    return Task.FromResult(incoming.WriteParamEncaps(ret.OutEncaps!, ret.ReturnValue)!);
                },
                TaskScheduler.Current).Unwrap();
        }
    }
}
