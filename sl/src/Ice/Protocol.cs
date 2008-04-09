// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    sealed class Protocol
    {
        //
        // The magic number at the front of each message
        //
        internal static readonly byte[] magic
            = new byte[] { (byte)0x49, (byte)0x48, (byte)0x54, (byte)0x50 }; // 'I', 'H', 'T', 'P'
        
        //
        // The current Ice protocol and encoding version
        //
        internal const byte protocolMajor = 1;
        internal const byte protocolMinor = 0;
        internal const byte encodingMajor = 1;
        internal const byte encodingMinor = 0;
        
        internal static readonly byte[] requestHdr = new byte[]
        {
            IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
            IceInternal.Protocol.magic[3],
            IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
            IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
        };

        internal static readonly byte[] replyHdr = new byte[]
        {
            IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
            IceInternal.Protocol.magic[3],
            IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
            IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
        };
    }
}
