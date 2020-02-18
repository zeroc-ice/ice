//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    ///<summary>The base interface for all servants.</summary>
    public interface IObject
    {
        /// <summary>Dispatches a request on this servant.</summary>
        /// <param name="istr">The <see cref="InputStream"/> that holds the request frame. It is positioned at
        /// the start of the request's payload, just after starting to read the encapsulation.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// current request.</param>
        /// <returns>A value task that provides the response frame for the request.
        /// See <see cref="IceInternal.Protocol.StartResponseFrame"/>.</returns>
        /// <exception cref="System.Exception">Any exception thrown by Dispatch will be marshaled into the response
        /// frame.</exception>
        public ValueTask<OutputStream> DispatchAsync(InputStream istr, Current current)
        {
            // TODO: switch to abstract method
            Debug.Assert(false);
            return IceFromVoidResult(current);
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

        // The following protected static methods with Ice-prefixes are Ice-internal helper methods used by
        // generated servants.

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

        protected static ValueTask<Ice.OutputStream> IceFromResult(Ice.OutputStream ostr)
            => new ValueTask<Ice.OutputStream>(ostr);

        protected static ValueTask<Ice.OutputStream> IceFromVoidResult(Ice.Current current)
        {
            // TODO: for oneway requests, we should reuse the same fake response frame, not
            // create a new one each time. It could be OutputStream.Empty.
            return IceFromResult(Protocol.CreateEmptyResponseFrame(current));
        }

        protected ValueTask<OutputStream> IceD_ice_pingAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            IcePing(current);
            return IceFromVoidResult(current);
        }

        protected ValueTask<OutputStream> IceD_ice_isAAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            string id = istr.ReadString();
            istr.EndEncapsulation();
            bool ret = IceIsA(id, current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteBool(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }

        protected ValueTask<OutputStream> IceD_ice_idAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string ret = IceId(current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteString(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }

        protected ValueTask<OutputStream> IceD_ice_idsAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string[] ret = IceIds(current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteStringSeq(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }
    }

    /// <summary>All marshaled return value structs generated by the Slice compiler implement this
    /// IMarshaledResultValue interface.</summary>
    public interface IMarshaledReturnValue
    {
        public OutputStream OutputStream { get; }
    }

    // TODO: remove once the proxy InvokeAsync signature is updated to match the Dispatch signature.
    public abstract class BlobjectAsync : IObject
    {
        public abstract Task<Ice.Object_Ice_invokeResult> IceInvokeAsync(byte[] inEncaps, Current current);
        public async ValueTask<OutputStream> DispatchAsync(InputStream istr, Current current)
        {
            byte[] inEncaps = ReadParamEncaps(istr);
            Object_Ice_invokeResult ret = await IceInvokeAsync(inEncaps, current).ConfigureAwait(false);
            return WriteParamEncaps(ret.OutEncaps, ret.ReturnValue, current);
        }

        private static byte[] ReadParamEncaps(InputStream istr)
        {
            istr.Pos -= 6;
            byte[] result = istr.ReadEncapsulation(out Ice.EncodingVersion encoding);
            return result;
        }

        private static Ice.OutputStream WriteParamEncaps(byte[]? v, bool ok, Current current)
        {
            if (current.IsOneway)
            {
                Debug.Assert(ok);
                return Protocol.CreateEmptyResponseFrame(current);
            }

            if (!ok)
            {
                current.DispatchObserver?.UserException();
            }

            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);
            ostr.WriteByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);

            if (v == null || v.Length == 0)
            {
                ostr.WriteEmptyEncapsulation(current.Encoding);
            }
            else
            {
                ostr.WriteEncapsulation(v);
            }
            return ostr;
        }
    }
}
