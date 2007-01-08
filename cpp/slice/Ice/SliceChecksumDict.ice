// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICE_CHECKSUM_DICT_ICE
#define ICE_SLICE_CHECKSUM_DICT_ICE

module Ice
{

/**
 * A mapping from type IDs to Slice checksums. The dictionary
 * allows verification at run time that client and server
 * use matching Slice definitions.
 **/
dictionary<string, string> SliceChecksumDict;

};

#endif
