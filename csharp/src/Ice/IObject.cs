//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The base interface for all servants.</summary>
    public interface IObject
    {
        public static class Request
        {
            public static readonly InputStreamReader<string> IceIsA = InputStream.IceReaderIntoString;
        }

        public static class Response
        {
            public static OutgoingResponseFrame IceId(Current current, string returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    OutputStream.IceWriterFromString);

            public static OutgoingResponseFrame IceIds(Current current, IEnumerable<string> returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    (ostr, returnValue) => ostr.WriteSequence(returnValue, OutputStream.IceWriterFromString));

            public static OutgoingResponseFrame IceIsA(Current current, bool returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    OutputStream.IceWriterFromBool);
        }

        /// <summary>Dispatches a request on this servant.</summary>
        /// <param name="request">The <see cref="IncomingRequestFrame"/> The request being dispatch.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// current request.</param>
        /// <returns>A value task that provides the response frame for the request.
        /// See <see cref="OutgoingResponseFrame"/>.</returns>
        /// <exception cref="Exception">Any exception thrown by Dispatch will be marshaled into the response
        /// frame.</exception>
        public ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            // TODO: switch to abstract method
            Debug.Assert(false);
            return new ValueTask<OutgoingResponseFrame>(OutgoingResponseFrame.WithVoidReturnValue(current));
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
        public bool IceIsA(string typeId, Current current) =>
            Array.BinarySearch((string[])IceIds(current), typeId, StringComparer.Ordinal) >= 0;

        /// <summary>Returns the Slice type IDs of the interfaces supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in alphabetical order.</returns>
        public IEnumerable<string> IceIds(Current current) => new string[] { "::Ice::Object" };

        /// <summary>Returns the Slice type ID of the most-derived interface supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Current current) => "::Ice::Object";

        // The following protected static methods with Ice-prefixes are Ice-internal helper methods used by
        // generated servants.

        /// <summary>The generated code calls this method to ensure that when an operation is _not_ declared
        /// idempotent, the request is not marked idempotent (which would mean the caller incorrectly believes this
        /// operation is idempotent).</summary>
        /// <param name="current">The current object for the dispatch.</param>
        protected static void IceCheckNonIdempotent(Current current)
        {
            if (current.IsIdempotent)
            {
                throw new InvalidDataException(
                        $@"idempotent mismatch for operation `{current.Operation
                        }': received request marked idempotent for a non-idempotent operation");
            }
        }

        /// <summary>This method is called to dispatch ice_ping operation, the method delegates to the servant's IcePing
        /// implementation.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <returns>The response frame</returns>
        protected ValueTask<OutgoingResponseFrame> IceD_ice_pingAsync(IncomingRequestFrame request, Current current)
        {
            request.ReadEmptyParamList();
            IcePing(current);
            return new ValueTask<OutgoingResponseFrame>(OutgoingResponseFrame.WithVoidReturnValue(current));
        }

        /// <summary>This method is called to dispatch ice_isA operation, the method delegates to the servant's IceIsA
        /// implementation.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <returns>The response frame</returns>
        protected ValueTask<OutgoingResponseFrame> IceD_ice_isAAsync(IncomingRequestFrame request, Current current)
        {
            string id = request.ReadParamList(current.Communicator, Request.IceIsA);
            bool returnValue = IceIsA(id, current);
            return new ValueTask<OutgoingResponseFrame>(Response.IceIsA(current, returnValue));
        }

        /// <summary>This method is called to dispatch ice_id operation, the method delegates to the servant's IceId
        /// implementation.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <returns>The response frame</returns>
        protected ValueTask<OutgoingResponseFrame> IceD_ice_idAsync(IncomingRequestFrame request, Current current)
        {
            request.ReadEmptyParamList();
            string returnValue = IceId(current);
            return new ValueTask<OutgoingResponseFrame>(Response.IceId(current, returnValue));
        }

        /// <summary>This method is called to dispatch ice_ids operation, the method delegates to the servant's IceIds
        /// implementation.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <returns>The response frame</returns>
        protected ValueTask<OutgoingResponseFrame> IceD_ice_idsAsync(IncomingRequestFrame request, Current current)
        {
            request.ReadEmptyParamList();
            IEnumerable<string> returnValue = IceIds(current);
            return new ValueTask<OutgoingResponseFrame>(Response.IceIds(current, returnValue));
        }
    }
}
