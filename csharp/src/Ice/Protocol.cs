//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
     // TODO: move class to namespace Ice
    public static class Protocol
    {
        /// <summary>Starts a new response frame and the encapsulation in that frame. This frame encodes a "success"
        /// response.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request currently dispatched.</param>
        /// <param name="format">The Slice format (Compact or Sliced) used by the encapsulation.</param>
        /// <returns>An <see cref="Ice.OutputStream"/> that holds the new frame.</returns>
        public static Ice.OutputStream StartResponseFrame(Ice.Current current, Ice.FormatType? format = null)
        {
            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);
            ostr.WriteByte(ReplyStatus.replyOK);
            ostr.StartEncapsulation(current.Encoding, format);
            return ostr;
        }

        /// <summary>Creates a new response frame that represents "success" with a void return value. The returned
        /// frame is complete and no additional data can be included in its payload.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request currently dispatched.</param>
        /// <returns>An <see cref="Ice.OutputStream"/> that holds the new frame.</returns>
        public static Ice.OutputStream CreateEmptyResponseFrame(Ice.Current current)
        {
            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);
            ostr.WriteByte(ReplyStatus.replyOK);
            ostr.WriteEmptyEncapsulation(current.Encoding);
            return ostr;
        }

        /// <summary>Starts a new response frame and the encapsulation in that frame. This frame encodes a "failure"
        /// response.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request currently dispatched.</param>
        /// <returns>An <see cref="Ice.OutputStream"/> that holds the new frame.</returns>
        public static Ice.OutputStream StartFailureResponseFrame(Ice.Current current)
        {
            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);
            ostr.WriteByte(ReplyStatus.replyUserException);
            // Exceptions are always marshaled in the sliced format:
            ostr.StartEncapsulation(current.Encoding, FormatType.SlicedFormat);
            return ostr;
        }

        ///<summary>Creates a response frame that represents "failure" and contains an exception. The returned frame is
        /// complete and no additional data can be included in its payload.</summary>
        /// <param name="exception">The exception to marshal into the frame.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request currently dispatched.</param>
        /// <returns>An <see cref="Ice.OutputStream"/> that holds the new frame.</returns>
        public static Ice.OutputStream CreateFailureResponseFrame(System.Exception exception, Ice.Current current)
        {
            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);

            try
            {
                throw exception;
            }
            catch (Ice.RequestFailedException ex)
            {
                if (ex.Id.Name == null || ex.Id.Name.Length == 0)
                {
                    ex.Id = current.Id;
                }

                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    ex.Facet = current.Facet;
                }

                if (ex.Operation == null || ex.Operation.Length == 0)
                {
                    ex.Operation = current.Operation;
                }

                if (ex is Ice.ObjectNotExistException)
                {
                    ostr.WriteByte(ReplyStatus.replyObjectNotExist);
                }
                else if (ex is Ice.FacetNotExistException)
                {
                    ostr.WriteByte(ReplyStatus.replyFacetNotExist);
                }
                else if (ex is Ice.OperationNotExistException)
                {
                    ostr.WriteByte(ReplyStatus.replyOperationNotExist);
                }
                else
                {
                    Debug.Assert(false);
                }
                ex.Id.IceWrite(ostr);

                // For compatibility with the old FacetPath.
                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    ostr.WriteStringSeq(Array.Empty<string>());
                }
                else
                {
                    string[] facetPath2 = { ex.Facet };
                    ostr.WriteStringSeq(facetPath2);
                }
                ostr.WriteString(ex.Operation);
            }
            catch (Ice.UnknownLocalException ex)
            {
                ostr.WriteByte(ReplyStatus.replyUnknownLocalException);
                ostr.WriteString(ex.Unknown);
            }
            catch (Ice.UnknownUserException ex)
            {
                ostr.WriteByte(ReplyStatus.replyUnknownUserException);
                ostr.WriteString(ex.Unknown);
            }
            catch (Ice.UnknownException ex)
            {
                ostr.WriteByte(ReplyStatus.replyUnknownException);
                ostr.WriteString(ex.Unknown);
            }
            catch (Ice.UserException ex)
            {
                ostr.WriteByte(ReplyStatus.replyUserException);
                // Exceptions are always marshaled in the sliced format:
                ostr.StartEncapsulation(current.Encoding, FormatType.SlicedFormat);
                ostr.WriteException(ex);
                ostr.EndEncapsulation();
            }
            catch (Ice.Exception ex)
            {
                ostr.WriteByte(ReplyStatus.replyUnknownLocalException);
                ostr.WriteString(ex.ice_id() + "\n" + ex.StackTrace);
            }
            catch (System.Exception ex)
            {
                ostr.WriteByte(ReplyStatus.replyUnknownException);
                ostr.WriteString(ex.ToString());
            }
            return ostr;
        }

        internal static Ice.Current CreateCurrent(int requestId, Ice.InputStream requestFrame,
            Ice.ObjectAdapter adapter, Ice.Connection? connection = null)
        {
            var identity = new Ice.Identity(requestFrame);

            // For compatibility with the old FacetPath.
            string[] facetPath = requestFrame.ReadStringArray();
            if (facetPath.Length > 1)
            {
                throw new Ice.MarshalException();
            }
            string facet = facetPath.Length == 0 ? "" : facetPath[0];
            string operation = requestFrame.ReadString();
            byte mode = requestFrame.ReadByte();
            var context = requestFrame.ReadContext();
            Ice.EncodingVersion encoding = requestFrame.StartEncapsulation();

            return new Ice.Current(adapter, identity, facet, operation, (Ice.OperationMode)mode, context,
                requestId, connection, encoding);
        }

        //
        // Size of the Ice protocol header
        //
        // Magic number (4 bytes)
        // Protocol version major (Byte)
        // Protocol version minor (Byte)
        // Encoding version major (Byte)
        // Encoding version minor (Byte)
        // Message type (Byte)
        // Compression status (Byte)
        // Message size (Int)
        //
        internal const int headerSize = 14;

        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        //
        // The current Ice protocol and encoding version
        //
        internal const byte protocolMajor = 1;
        internal const byte protocolMinor = 0;
        internal const byte protocolEncodingMajor = 1;
        internal const byte protocolEncodingMinor = 0;

        internal const byte encodingMajor = 1;
        internal const byte encodingMinor = 1;

        public const byte OPTIONAL_END_MARKER = 0xFF;

        public const byte FLAG_HAS_TYPE_ID_STRING = (1 << 0);
        public const byte FLAG_HAS_TYPE_ID_INDEX = (1 << 1);
        public const byte FLAG_HAS_TYPE_ID_COMPACT = (1 << 1 | 1 << 0);
        public const byte FLAG_HAS_OPTIONAL_MEMBERS = (1 << 2);
        public const byte FLAG_HAS_INDIRECTION_TABLE = (1 << 3);
        public const byte FLAG_HAS_SLICE_SIZE = (1 << 4);
        public const byte FLAG_IS_LAST_SLICE = (1 << 5);

        //
        // The Ice protocol message types
        //
        internal const byte requestMsg = 0;
        internal const byte requestBatchMsg = 1;
        internal const byte replyMsg = 2;
        internal const byte validateConnectionMsg = 3;
        internal const byte closeConnectionMsg = 4;

        internal static readonly byte[] requestHdr = new byte[]
        {
            magic[0], magic[1], magic[2], magic[3],
            protocolMajor, protocolMinor,
            protocolEncodingMajor, protocolEncodingMinor,
            requestMsg,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] requestBatchHdr = new byte[]
        {
            magic[0], magic[1], magic[2], magic[3],
            protocolMajor, protocolMinor,
            protocolEncodingMajor, protocolEncodingMinor,
            requestBatchMsg,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Number of requests in batch (placeholder).
        };

        internal static readonly byte[] replyHdr = new byte[]
        {
            magic[0], magic[1], magic[2], magic[3],
            protocolMajor, protocolMinor,
            protocolEncodingMajor, protocolEncodingMinor,
            replyMsg,
            0, // Compression status.
            0, 0, 0, 0 // Message size (placeholder).
        };

        internal static void
        checkSupportedProtocol(Ice.ProtocolVersion v)
        {
            if (v.Major != protocolMajor || v.Minor > protocolMinor)
            {
                throw new Ice.UnsupportedProtocolException("", v, Ice.Util.CurrentProtocol);
            }
        }

        public static void
        checkSupportedProtocolEncoding(Ice.EncodingVersion v)
        {
            if (v.Major != protocolEncodingMajor || v.Minor > protocolEncodingMinor)
            {
                throw new Ice.UnsupportedEncodingException("", v, Ice.Util.CurrentProtocolEncoding);
            }
        }

        internal static void
        checkSupportedEncoding(Ice.EncodingVersion v)
        {
            if (v.Major != encodingMajor || v.Minor > encodingMinor)
            {
                throw new Ice.UnsupportedEncodingException("", v, Ice.Util.CurrentEncoding);
            }
        }

        //
        // Either return the given protocol if not compatible, or the greatest
        // supported protocol otherwise.
        //
        internal static Ice.ProtocolVersion
        getCompatibleProtocol(Ice.ProtocolVersion v)
        {
            if (v.Major != Ice.Util.CurrentProtocol.Major)
            {
                return v; // Unsupported protocol, return as is.
            }
            else if (v.Minor < Ice.Util.CurrentProtocol.Minor)
            {
                return v; // Supported protocol.
            }
            else
            {
                //
                // Unsupported but compatible, use the currently supported
                // protocol, that's the best we can do.
                //
                return Ice.Util.CurrentProtocol;
            }
        }

        //
        // Either return the given encoding if not compatible, or the greatest
        // supported encoding otherwise.
        //
        internal static Ice.EncodingVersion
        getCompatibleEncoding(Ice.EncodingVersion v)
        {
            if (v.Major != Ice.Util.CurrentEncoding.Major)
            {
                return v; // Unsupported encoding, return as is.
            }
            else if (v.Minor < Ice.Util.CurrentEncoding.Minor)
            {
                return v; // Supported encoding.
            }
            else
            {
                //
                // Unsupported but compatible, use the currently supported
                // encoding, that's the best we can do.
                //
                return Ice.Util.CurrentEncoding;
            }
        }

        internal static bool
        isSupported(Ice.ProtocolVersion version, Ice.ProtocolVersion supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;

        internal static bool
        isSupported(Ice.EncodingVersion version, Ice.EncodingVersion supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }

}
