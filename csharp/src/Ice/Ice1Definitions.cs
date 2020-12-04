// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    // Definitions for the ice1 protocol.
    internal static class Ice1Definitions
    {
        // ice1 frame types:
        internal enum FrameType : byte
        {
            Request = 0,
            RequestBatch = 1,
            Reply = 2,
            ValidateConnection = 3,
            CloseConnection = 4
        }

        // The encoding of the header for ice1 frames. It is nominally 1.0, but in practice it is identical to 1.1
        // for the subset of the encoding used by the ice1 headers.
        internal static readonly Encoding Encoding = Encoding.V11;

        // Size of an ice1 frame header:
        // Magic number (4 bytes)
        // Protocol bytes (4 bytes)
        // Frame type (Byte)
        // Compression status (Byte)
        // Frame size (Int - 4 bytes)
        internal const int HeaderSize = 14;

        // The magic number at the front of each frame.
        internal static readonly byte[] Magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        // 4-bytes after magic that provide the protocol version (always 1.0 for an ice1 frame) and the
        // encoding of the frame header (always set to 1.0 with the an ice1 frame, even though we use 1.1).
        internal static readonly byte[] ProtocolBytes = new byte[] { 1, 0, 1, 0 };

        internal static readonly List<ArraySegment<byte>> CloseConnectionFrame = new List<ArraySegment<byte>>
        {
                new byte[]
                {
                    Magic[0], Magic[1], Magic[2], Magic[3],
                    ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
                    (byte)FrameType.CloseConnection,
                    0, // Compression status.
                    HeaderSize, 0, 0, 0 // Frame size.
                }
        };

        internal static readonly byte[] RequestHeaderPrologue = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte)FrameType.Request,
            0, // Compression status.
            0, 0, 0, 0, // Frame size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] ResponseHeaderPrologue = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
            (byte)FrameType.Reply,
            0, // Compression status.
            0, 0, 0, 0 // Frame size (placeholder).
        };

        internal static readonly List<ArraySegment<byte>> ValidateConnectionFrame = new List<ArraySegment<byte>>
        {
            new byte[]
            {
                Magic[0], Magic[1], Magic[2], Magic[3],
                ProtocolBytes[0], ProtocolBytes[1], ProtocolBytes[2], ProtocolBytes[3],
                (byte)FrameType.ValidateConnection,
                0, // Compression status.
                HeaderSize, 0, 0, 0 // Frame size.
            }
        };

        // Verify that the first 8 bytes correspond to Magic + ProtocolBytes
        internal static void CheckHeader(ReadOnlySpan<byte> header)
        {
            Debug.Assert(header.Length == 14);
            if (header[0] != Magic[0] || header[1] != Magic[1] || header[2] != Magic[2] || header[3] != Magic[3])
            {
                throw new InvalidDataException(
                    $"received incorrect magic bytes in header of ice1 frame: {BytesToString(header.Slice(0, 4))}");
            }

            header = header[4..];

            if (header[0] != ProtocolBytes[0] || header[1] != ProtocolBytes[1])
            {
                throw new InvalidDataException(
                    $"received ice1 protocol frame with protocol set to {header[0]}.{header[1]}");
            }

            if (header[2] != ProtocolBytes[2] || header[3] != ProtocolBytes[3])
            {
                throw new InvalidDataException(
                    $"received ice1 protocol frame with protocol encoding set to {header[2]}.{header[3]}");
            }
        }

        internal static string GetFacet(string[] facetPath)
        {
            if (facetPath.Length > 1)
            {
                throw new InvalidDataException($"read ice1 facet path with {facetPath.Length} elements");
            }
            return facetPath.Length == 1 ? facetPath[0] : "";
        }

        internal static RetryPolicy GetRetryPolicy(IncomingResponseFrame response, Reference reference)
        {
            Debug.Assert(response.Encoding == Encoding.V11);
            if (response.ResultType == ResultType.Failure)
            {
                var replyStatus = (ReplyStatus)response.Payload[0]; // can be reassigned below

                InputStream? istr = null;
                if (response.Protocol == Protocol.Ice1)
                {
                    if (replyStatus != ReplyStatus.UserException)
                    {
                        istr = new InputStream(response.Payload.Slice(1), Encoding.V11);
                    }
                }
                else
                {
                    istr = new InputStream(response.Payload.Slice(1),
                                           Ice2Definitions.Encoding,
                                           reference: reference,
                                           startEncapsulation: true);

                    replyStatus = istr.ReadReplyStatus();
                    if (replyStatus == ReplyStatus.UserException)
                    {
                        istr = null; // we are not reading this user exception here
                    }
                }

                if (istr?.ReadIce1SystemException(replyStatus) is ObjectNotExistException one)
                {
                    // 1.1 System exceptions
                    if (reference.RouterInfo != null && one.Origin!.Value.Operation == "ice_add_proxy")
                    {
                        // If we have a router, an ObjectNotExistException with an operation name
                        // "ice_add_proxy" indicates to the client that the router isn't aware of the proxy
                        // (for example, because it was evicted by the router). In this case, we must
                        // *always* retry, so that the missing proxy is added to the router.
                        reference.RouterInfo.ClearCache(reference);
                        return RetryPolicy.AfterDelay(TimeSpan.Zero);
                    }
                    else if (reference.IsIndirect)
                    {
                        if (reference.IsWellKnown)
                        {
                            reference.LocatorInfo?.ClearCache(reference);
                        }
                        return RetryPolicy.OtherReplica;
                    }
                }
            }
            return RetryPolicy.NoRetry;
        }

        /// <summary>Reads a facet in the old ice1 format from the stream.</summary>
        /// <param name="istr">The stream to read from.</param>
        /// <returns>The facet read from the stream.</returns>
        internal static string ReadIce1Facet(this InputStream istr)
        {
            Debug.Assert(istr.Encoding == Encoding);
            return GetFacet(istr.ReadArray(1, InputStream.IceReaderIntoString));
        }

        /// <summary>Reads an ice1 system exception encoded based on the provided reply status.</summary>
        /// <param name="istr">The stream to read from.</param>
        /// <param name="replyStatus">The reply status.</param>
        /// <returns>The exception read from the stream.</returns>
        internal static RemoteException ReadIce1SystemException(this InputStream istr, ReplyStatus replyStatus)
        {
            Debug.Assert(istr.Encoding == Encoding);
            Debug.Assert((byte)replyStatus > (byte)ReplyStatus.UserException);

            RemoteException systemException;

            switch (replyStatus)
            {
                case ReplyStatus.FacetNotExistException:
                case ReplyStatus.ObjectNotExistException:
                case ReplyStatus.OperationNotExistException:
                    var identity = new Identity(istr);
                    string facet = istr.ReadIce1Facet();
                    string operation = istr.ReadString();

                    if (replyStatus == ReplyStatus.OperationNotExistException)
                    {
                        systemException = new OperationNotExistException(
                            message: null,
                            new RemoteExceptionOrigin(identity, facet, operation));
                    }
                    else
                    {
                        systemException = new ObjectNotExistException(
                            message: null,
                            new RemoteExceptionOrigin(identity, facet, operation));
                    }
                    break;

                default:
                    systemException = new UnhandledException(istr.ReadString());
                    break;
            }

            systemException.ConvertToUnhandled = true;
            return systemException;
        }

        /// <summary>Writes a facet as a facet path.</summary>
        /// <param name="ostr">The stream.</param>
        /// <param name="facet">The facet to write to the stream.</param>
        internal static void WriteIce1Facet(this OutputStream ostr, string facet)
        {
            Debug.Assert(ostr.Encoding == Encoding);

            // The old facet-path style used by the ice1 protocol.
            if (facet.Length == 0)
            {
                ostr.WriteSize(0);
            }
            else
            {
                ostr.WriteSize(1);
                ostr.WriteString(facet);
            }
        }

        /// <summary>Writes a request header body without constructing an Ice1RequestHeaderBody instance. This
        /// implementation is slightly more efficient than the generated code because it avoids the allocation of a
        /// string[] to write the facet and the allocation of a Dictionary{string, string} to write the context.
        /// </summary>
        internal static void WriteIce1RequestHeaderBody(
            this OutputStream ostr,
            Identity identity,
            string facet,
            string operation,
            bool idempotent,
            IReadOnlyDictionary<string, string> context)
        {
            Debug.Assert(ostr.Encoding == Encoding);
            identity.IceWrite(ostr);
            ostr.WriteIce1Facet(facet);
            ostr.WriteString(operation);
            ostr.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);
            ostr.WriteDictionary(context, OutputStream.IceWriterFromString, OutputStream.IceWriterFromString);
        }

        private static string BytesToString(ReadOnlySpan<byte> bytes) => BitConverter.ToString(bytes.ToArray());
    }
}
