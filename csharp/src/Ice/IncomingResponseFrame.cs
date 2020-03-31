//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame
    {
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }

        /// <summary>The frame reply status <see cref="ReplyStatus"/>.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Dictionary<string, string>? Context { get; }

        /// <summary>The payload of this response frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; }

        /// <summary>The frame byte count.</summary>
        public int Size => Payload.Count;

        private readonly Communicator _communicator;

        /// <summary>Reads the return value carried by this response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(InputStreamReader<T> reader)
        {
            switch (ReplyStatus)
            {
                case ReplyStatus.OK:
                {
                    var istr = new InputStream(_communicator, Payload, 1);
                    istr.StartEncapsulation();
                    T ret = reader(istr);
                    istr.EndEncapsulation();
                    return ret;
                }
                case ReplyStatus.UserException:
                {
                    var istr = new InputStream(_communicator, Payload, 1);
                    istr.StartEncapsulation();
                    RemoteException ex = istr.ReadException();
                    istr.EndEncapsulation();
                    throw ex;
                }
                case ReplyStatus.ObjectNotExistException:
                case ReplyStatus.FacetNotExistException:
                case ReplyStatus.OperationNotExistException:
                {
                    throw ReadDispatchException();
                }
                case ReplyStatus.UnknownException:
                case ReplyStatus.UnknownLocalException:
                case ReplyStatus.UnknownUserException:
                {
                    throw new UnhandledException(InputStream.ReadString(Payload.Slice(1)), Identity.Empty, "", "");
                }

                default:
                {
                    Debug.Assert(false);
                    throw new InvalidDataException(
                        $"received ice1 response frame with unknown reply status `{ReplyStatus}'");
                }
            }
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        public void ReadVoidReturnValue()
        {
            switch (ReplyStatus)
            {
                case ReplyStatus.OK:
                {
                    var istr = new InputStream(_communicator, Payload, 1);
                    istr.StartEncapsulation();
                    istr.EndEncapsulation();
                    break;
                }
                case ReplyStatus.UserException:
                {
                    var istr = new InputStream(_communicator, Payload, 1);
                    istr.StartEncapsulation();
                    RemoteException ex = istr.ReadException();
                    istr.EndEncapsulation();
                    throw ex;
                }
                case ReplyStatus.ObjectNotExistException:
                case ReplyStatus.FacetNotExistException:
                case ReplyStatus.OperationNotExistException:
                {
                    throw ReadDispatchException();
                }
                case ReplyStatus.UnknownException:
                case ReplyStatus.UnknownLocalException:
                case ReplyStatus.UnknownUserException:
                {
                    throw ReadUnhandledException();
                }
            }
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="payload">The frame raw data as an array segment.</param>
        public IncomingResponseFrame(Communicator communicator, ArraySegment<byte> payload)
        {
            _communicator = communicator;
            byte replyStatus = payload[0];
            if (replyStatus < 0 || replyStatus > 7)
            {
                throw new InvalidDataException(
                    $"received ice1 response frame with unknown reply status `{replyStatus}'");
            }
            ReplyStatus = (ReplyStatus)replyStatus;
            Payload = payload;
            if (ReplyStatus == ReplyStatus.UserException || ReplyStatus == ReplyStatus.OK)
            {
                Encoding = new Encoding(payload[5], payload[6]);
            }
            else
            {
                Encoding = Encoding.V1_1;
            }
        }

        internal UnhandledException ReadUnhandledException() =>
            new UnhandledException(InputStream.ReadString(Payload.Slice(1)), Identity.Empty, "", "");

        internal DispatchException ReadDispatchException()
        {
            var istr = new InputStream(_communicator, Payload, 1);
            var identity = new Identity(istr);

            // For compatibility with the old FacetPath.
            string[] facetPath = istr.ReadStringArray();
            string facet;
            if (facetPath.Length > 0)
            {
                if (facetPath.Length > 1)
                {
                    throw new InvalidDataException($"invalid facet path length: {facetPath.Length}");
                }
                facet = facetPath[0];
            }
            else
            {
                facet = "";
            }

            string operation = istr.ReadString();

            if (ReplyStatus == ReplyStatus.OperationNotExistException)
            {
                return new OperationNotExistException(identity, facet, operation);
            }
            else
            {
                return new ObjectNotExistException(identity, facet, operation);
            }
        }
    }
}
