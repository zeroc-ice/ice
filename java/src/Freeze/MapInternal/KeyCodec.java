// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

interface KeyCodec<K>
{
    public abstract byte[] encodeKey(K k, Ice.Communicator communicator, Ice.EncodingVersion encoding);
    public abstract K decodeKey(byte[] b, Ice.Communicator communicator, Ice.EncodingVersion encoding);
}
