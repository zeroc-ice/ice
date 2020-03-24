//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
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
        /// See <see cref="OutgoingResponseFrame"/>.</returns>
        /// <exception cref="System.Exception">Any exception thrown by Dispatch will be marshaled into the response
        /// frame.</exception>
        public ValueTask<OutgoingResponseFrame> DispatchAsync(InputStream istr, Current current)
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

        // The generated code calls this method to ensure that when an operation is _not_ declared idempotent, the
        // request is not marked idempotent (which would mean the caller incorrectly believes this operation is
        // idempotent).
        protected static void IceCheckNonIdempotent(Current current)
        {
            if (current.IsIdempotent)
            {
                throw new MarshalException(
                        $@"idempotent mismatch for operation `{current.Operation
                        }': received request marked idempotent for a non-idempotent operation");
            }
        }

        protected static ValueTask<OutgoingResponseFrame> IceFromResult(OutgoingResponseFrame responseFrame)
            => new ValueTask<OutgoingResponseFrame>(responseFrame);

        protected static ValueTask<OutgoingResponseFrame> IceFromVoidResult(Current current)
        {
            // TODO: for oneway requests, we should reuse the same fake response frame, not
            // create a new one each time.
            return IceFromResult(OutgoingResponseFrame.WithVoidReturnValue(current.Encoding));
        }

        protected ValueTask<OutgoingResponseFrame> IceD_ice_pingAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            IcePing(current);
            return IceFromVoidResult(current);
        }

        protected ValueTask<OutgoingResponseFrame> IceD_ice_isAAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            string id = istr.ReadString();
            istr.EndEncapsulation();
            bool ret = IceIsA(id, current);
            var response = OutgoingResponseFrame.WithReturnValue(current.Encoding, null, ret,
                OutputStream.IceWriterFromBool);
            return IceFromResult(response);
        }

        protected ValueTask<OutgoingResponseFrame> IceD_ice_idAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string ret = IceId(current);
            var response = OutgoingResponseFrame.WithReturnValue(current.Encoding, null, ret,
                OutputStream.IceWriterFromString);
            return IceFromResult(response);
        }

        protected ValueTask<OutgoingResponseFrame> IceD_ice_idsAsync(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string[] ret = IceIds(current);
            var response = OutgoingResponseFrame.WithReturnValue(current.Encoding, null, ret,
                (OutputStream ostr, string[] ret) => ostr.WriteStringSeq(ret));
            return IceFromResult(response);
        }
    }
}
