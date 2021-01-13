// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A delegate that reads the request parameters from a request frame.</summary>
    /// <typeparam name="T">The type of the request parameters to read.</typeparam>
    /// <param name="connection">The connection used to received the request frame.</param>
    /// <param name="request">The request frame to read the parameters from.</param>
    /// <returns>The request parmeters.</returns>
    public delegate T RequestReader<T>(Connection connection, IncomingRequestFrame request);

    /// <summary>The base interface for all servants.</summary>
    public interface IObject
    {
        /// <summary>Dispatches a request on this servant.</summary>
        /// <param name="request">The <see cref="IncomingRequestFrame"/> to dispatch.</param>
        /// <param name="current">Holds decoded header data and other information about the current request.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is cancelled.
        /// </param>
        /// <returns>A value task that provides the <see cref="OutgoingResponseFrame"/> for the request.</returns>
        /// <exception cref="Exception">Any exception thrown by DispatchAsync will be marshaled into the response
        /// frame.</exception>
        public ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            // TODO: switch to abstract method (but doesn't work as of .NET 5 RC1).
            Debug.Assert(false);
            return new ValueTask<OutgoingResponseFrame>(OutgoingResponseFrame.WithVoidReturnValue(current));
        }

        // The following are helper classes and methods for generated servants.

        /// <summary>Holds a <see cref="RequestReader{T}"/> for each remote operation with parameter(s) defined in
        /// the pseudo-interface Object.</summary>
        public static class Request
        {
            /// <summary>The <see cref="RequestReader{T}"/> for the parameter of operation ice_isA.</summary>
            /// <summary>Decodes the ice_id operation parameters from an <see cref="IncomingRequestFrame"/>.</summary>
            /// <param name="connection">The used to receive the frame.</param>
            /// <param name="request">The request frame.</param>
            /// <returns>The return value decoded from the frame.</returns>
            public static string IceIsA(Connection connection, IncomingRequestFrame request) =>
                 request.ReadArgs(connection, InputStream.IceReaderIntoString);
        }

        /// <summary>Provides an <see cref="OutgoingResponseFrame"/> factory method for each non-void remote operation
        /// defined in the pseudo-interface Object.</summary>
        public static class Response
        {
            /// <summary>Creates an <see cref="OutgoingResponseFrame"/> for operation ice_id.</summary>
            /// <param name="current">Holds decoded header data and other information about the current request.</param>
            /// <param name="returnValue">The return value to write into the new frame.</param>
            /// <returns>A new <see cref="OutgoingResponseFrame"/>.</returns>
            public static OutgoingResponseFrame IceId(Current current, string returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    OutputStream.IceWriterFromString);

            /// <summary>Creates an <see cref="OutgoingResponseFrame"/> for operation ice_ids.</summary>
            /// <param name="current">Holds decoded header data and other information about the current request.</param>
            /// <param name="returnValue">The return value to write into the new frame.</param>
            /// <returns>A new <see cref="OutgoingResponseFrame"/>.</returns>
            public static OutgoingResponseFrame IceIds(Current current, IEnumerable<string> returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    (ostr, returnValue) => ostr.WriteSequence(returnValue, OutputStream.IceWriterFromString));

            /// <summary>Creates an <see cref="OutgoingResponseFrame"/> for operation ice_isA.</summary>
            /// <param name="current">Holds decoded header data and other information about the current request.</param>
            /// <param name="returnValue">The return value to write into the new frame.</param>
            /// <returns>A new <see cref="OutgoingResponseFrame"/>.</returns>
            public static OutgoingResponseFrame IceIsA(Current current, bool returnValue) =>
                OutgoingResponseFrame.WithReturnValue(
                    current,
                    compress: false,
                    format: default,
                    returnValue,
                    OutputStream.IceWriterFromBool);
        }

        /// <summary>Returns the Slice type ID of the most-derived interface supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is cancelled.
        /// </param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public ValueTask<string> IceIdAsync(Current current, CancellationToken cancel) => new("::Ice::Object");

        /// <summary>Returns the Slice type IDs of the interfaces supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in alphabetical order.</returns>
        public ValueTask<IEnumerable<string>> IceIdsAsync(Current current, CancellationToken cancel) =>
            new(new string[] { "::Ice::Object" });

        /// <summary>Tests whether this object supports the specified Slice interface.</summary>
        /// <param name="typeId">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>True if this object implements the interface specified by typeId.</returns>
        public async ValueTask<bool> IceIsAAsync(string typeId, Current current, CancellationToken cancel)
        {
            var array = (string[])await IceIdsAsync(current, cancel).ConfigureAwait(false);
            return Array.BinarySearch(array, typeId, StringComparer.Ordinal) >= 0;
        }

        /// <summary>Tests whether this object can be reached.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        public ValueTask IcePingAsync(Current current, CancellationToken cancel) => default;

        /// <summary>The generated code calls this method to ensure that when an operation is _not_ declared
        /// idempotent, the request is not marked idempotent. If the request is marked idempotent, it means the caller
        /// incorrectly believes this operation is idempotent.</summary>
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

        /// <summary>Dispatches an ice_id request.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>The response frame.</returns>
        protected async ValueTask<OutgoingResponseFrame> IceDIceIdAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            request.ReadEmptyArgs();
            string returnValue = await IceIdAsync(current, cancel).ConfigureAwait(false);
            return Response.IceId(current, returnValue);
        }

        /// <summary>Dispatches an ice_ids request.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>The response frame.</returns>
        protected async ValueTask<OutgoingResponseFrame> IceDIceIdsAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            request.ReadEmptyArgs();
            IEnumerable<string> returnValue = await IceIdsAsync(current, cancel).ConfigureAwait(false);
            return Response.IceIds(current, returnValue);
        }

        /// <summary>Dispatches an ice_isA request.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>The response frame.</returns>
        protected async ValueTask<OutgoingResponseFrame> IceDIceIsAAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            string id = Request.IceIsA(current.Connection, request);
            bool returnValue = await IceIsAAsync(id, current, cancel).ConfigureAwait(false);
            return Response.IceIsA(current, returnValue);
        }

        /// <summary>Dispatches an ice_ping request.</summary>
        /// <param name="request">The request frame.</param>
        /// <param name="current">The current object for the dispatch.</param>
        /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is canceled.
        /// </param>
        /// <returns>The response frame.</returns>
        protected async ValueTask<OutgoingResponseFrame> IceDIcePingAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            request.ReadEmptyArgs();
            await IcePingAsync(current, cancel).ConfigureAwait(false);
            return OutgoingResponseFrame.WithVoidReturnValue(current);
        }
    }
}
