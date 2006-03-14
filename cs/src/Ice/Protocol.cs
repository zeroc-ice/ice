// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
	internal static readonly byte[] magic
	    = new byte[] { (byte)0x49, (byte)0x63, (byte)0x65, (byte)0x50 }; // 'I', 'c', 'e', 'P'
	
	//
	// The current Ice protocol and encoding version
	//
	internal const byte protocolMajor = 1;
	internal const byte protocolMinor = 0;
	internal const byte encodingMajor = 1;
	internal const byte encodingMinor = 0;
	
	//
	// The Ice protocol message types
	//
	internal const byte requestMsg = 0;
	internal const byte requestBatchMsg = 1;
	internal const byte replyMsg = 2;
	internal const byte validateConnectionMsg = 3;
	internal const byte closeConnectionMsg = 4;
    }

}
