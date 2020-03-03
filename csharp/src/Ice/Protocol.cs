//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    public static class Protocol
    {
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
            bool idempotent = requestFrame.ReadOperationMode() != OperationMode.Normal;
            Dictionary<string, string> context = requestFrame.ReadContext();
            Ice.Encoding encoding = requestFrame.StartEncapsulation();

            return new Ice.Current(adapter, identity, facet, operation, idempotent, context,
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
        internal const int HeaderSize = 14;

        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] Magic = new byte[] { 0x49, 0x63, 0x65, 0x50 }; // 'I', 'c', 'e', 'P'

        //
        // The current Ice protocol and encoding version
        //
        internal const byte ProtocolMajor = 1;
        internal const byte ProtocolMinor = 0;
        internal const byte ProtocolEncodingMajor = 1;
        internal const byte ProtocolEncodingMinor = 0;

        internal const byte EncodingMajor = 1;
        internal const byte EncodingMinor = 1;

        public const byte OPTIONAL_END_MARKER = 0xFF;

        public const byte FLAG_HAS_TYPE_ID_STRING = 1 << 0;
        public const byte FLAG_HAS_TYPE_ID_INDEX = 1 << 1;
        public const byte FLAG_HAS_TYPE_ID_COMPACT = (1 << 1) | (1 << 0);
        public const byte FLAG_HAS_OPTIONAL_MEMBERS = 1 << 2;
        public const byte FLAG_HAS_INDIRECTION_TABLE = 1 << 3;
        public const byte FLAG_HAS_SLICE_SIZE = 1 << 4;
        public const byte FLAG_IS_LAST_SLICE = 1 << 5;

        //
        // The Ice protocol message types
        //
        internal const byte RequestMessage = 0;
        internal const byte RequestBatchMessage = 1;
        internal const byte ReplyMessage = 2;
        internal const byte ValidateConnectionMessage = 3;
        internal const byte CloseConnectionMessage = 4;

        internal static readonly byte[] RequestHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            RequestMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Request ID (placeholder).
        };

        internal static readonly byte[] RequestBatchHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            RequestBatchMessage,
            0, // Compression status.
            0, 0, 0, 0, // Message size (placeholder).
            0, 0, 0, 0 // Number of requests in batch (placeholder).
        };

        internal static readonly byte[] ReplyHeader = new byte[]
        {
            Magic[0], Magic[1], Magic[2], Magic[3],
            ProtocolMajor, ProtocolMinor,
            ProtocolEncodingMajor, ProtocolEncodingMinor,
            ReplyMessage,
            0, // Compression status.
            0, 0, 0, 0 // Message size (placeholder).
        };

        internal static void
        CheckSupportedProtocol(Ice.ProtocolVersion v)
        {
            if (v.Major != ProtocolMajor || v.Minor > ProtocolMinor)
            {
                throw new Ice.UnsupportedProtocolException("", v, Ice.Util.CurrentProtocol);
            }
        }

        public static void
        CheckSupportedProtocolEncoding(Ice.Encoding v)
        {
            if (v.Major != ProtocolEncodingMajor || v.Minor > ProtocolEncodingMinor)
            {
                throw new Ice.UnsupportedEncodingException("", v, Ice.Util.CurrentProtocolEncoding);
            }
        }

        internal static void
        CheckSupportedEncoding(Ice.Encoding v)
        {
            if (v.Major != EncodingMajor || v.Minor > EncodingMinor)
            {
                throw new Ice.UnsupportedEncodingException("", v, Ice.Util.CurrentEncoding);
            }
        }

        //
        // Either return the given protocol if not compatible, or the greatest
        // supported protocol otherwise.
        //
        internal static Ice.ProtocolVersion
        GetCompatibleProtocol(Ice.ProtocolVersion v)
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
        internal static Ice.Encoding
        GetCompatibleEncoding(Ice.Encoding v)
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
        IsSupported(Ice.ProtocolVersion version, Ice.ProtocolVersion supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;

        internal static bool
        IsSupported(Ice.Encoding version, Ice.Encoding supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }

}
