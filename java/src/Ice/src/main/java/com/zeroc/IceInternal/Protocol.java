// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.EncodingVersion;
import com.zeroc.Ice.ProtocolVersion;

final public class Protocol
{
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
    public final static int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    public final static byte magic[] = { 0x49, 0x63, 0x65, 0x50 };      // 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    public final static byte protocolMajor = 1;
    public final static byte protocolMinor = 0;
    public final static byte protocolEncodingMajor = 1;
    public final static byte protocolEncodingMinor = 0;

    public final static byte encodingMajor = 1;
    public final static byte encodingMinor = 1;

    //
    // The Ice protocol message types
    //
    public final static byte requestMsg = 0;
    public final static byte requestBatchMsg = 1;
    public final static byte replyMsg = 2;
    public final static byte validateConnectionMsg = 3;
    public final static byte closeConnectionMsg = 4;

    public final static byte[] requestHdr =
    {
        Protocol.magic[0],
        Protocol.magic[1],
        Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor,
        Protocol.protocolMinor,
        Protocol.protocolEncodingMajor,
        Protocol.protocolEncodingMinor,
        Protocol.requestMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0  // Request ID (placeholder).
    };

    public final static byte[] requestBatchHdr =
    {
        Protocol.magic[0],
        Protocol.magic[1],
        Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor,
        Protocol.protocolMinor,
        Protocol.protocolEncodingMajor,
        Protocol.protocolEncodingMinor,
        Protocol.requestBatchMsg,
        0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0  // Number of requests in batch (placeholder).
    };

    public final static byte[] replyHdr =
    {
        Protocol.magic[0],
        Protocol.magic[1],
        Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor,
        Protocol.protocolMinor,
        Protocol.protocolEncodingMajor,
        Protocol.protocolEncodingMinor,
        Protocol.replyMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

    static final public ProtocolVersion currentProtocol = new ProtocolVersion(protocolMajor, protocolMinor);
    static final public EncodingVersion currentProtocolEncoding = new EncodingVersion(protocolEncodingMajor,
                                                                                      protocolEncodingMinor);

    static final public EncodingVersion currentEncoding = new EncodingVersion(encodingMajor, encodingMinor);

    static public void
    checkSupportedProtocol(ProtocolVersion v)
    {
        if(v.major != currentProtocol.major || v.minor > currentProtocol.minor)
        {
            throw new com.zeroc.Ice.UnsupportedProtocolException("", v, currentProtocol);
        }
    }

    static public void
    checkSupportedProtocolEncoding(EncodingVersion v)
    {
        if(v.major != currentProtocolEncoding.major || v.minor > currentProtocolEncoding.minor)
        {
            throw new com.zeroc.Ice.UnsupportedEncodingException("", v, currentProtocolEncoding);
        }
    }

    static public void
    checkSupportedEncoding(EncodingVersion v)
    {
        if(v.major != currentEncoding.major || v.minor > currentEncoding.minor)
        {
            throw new com.zeroc.Ice.UnsupportedEncodingException("", v, currentEncoding);
        }
    }

    //
    // Either return the given protocol if not compatible, or the greatest
    // supported protocol otherwise.
    //
    static public ProtocolVersion
    getCompatibleProtocol(ProtocolVersion v)
    {
        if(v.major != currentProtocol.major)
        {
            return v; // Unsupported protocol, return as is.
        }
        else if(v.minor < currentProtocol.minor)
        {
            return v; // Supported protocol.
        }
        else
        {
            //
            // Unsupported but compatible, use the currently supported
            // protocol, that's the best we can do.
            //
            return currentProtocol;
        }
    }

    //
    // Either return the given encoding if not compatible, or the greatest
    // supported encoding otherwise.
    //
    static public EncodingVersion
    getCompatibleEncoding(EncodingVersion v)
    {
        if(v.major != currentEncoding.major)
        {
            return v; // Unsupported encoding, return as is.
        }
        else if(v.minor < currentEncoding.minor)
        {
            return v; // Supported encoding.
        }
        else
        {
            //
            // Unsupported but compatible, use the currently supported
            // encoding, that's the best we can do.
            //
            return currentEncoding;
        }
    }

    static public boolean
    isSupported(EncodingVersion version, EncodingVersion supported)
    {
        return version.major == supported.major && version.minor <= supported.minor;
    }

    public static final int OPTIONAL_END_MARKER            = 0xFF;

    public static final byte FLAG_HAS_TYPE_ID_STRING       = (byte)(1<<0);
    public static final byte FLAG_HAS_TYPE_ID_INDEX        = (byte)(1<<1);
    public static final byte FLAG_HAS_TYPE_ID_COMPACT      = (byte)(1<<1 | 1<<0);
    public static final byte FLAG_HAS_OPTIONAL_MEMBERS     = (byte)(1<<2);
    public static final byte FLAG_HAS_INDIRECTION_TABLE    = (byte)(1<<3);
    public static final byte FLAG_HAS_SLICE_SIZE           = (byte)(1<<4);
    public static final byte FLAG_IS_LAST_SLICE            = (byte)(1<<5);
}
