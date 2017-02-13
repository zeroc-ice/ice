// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// Delegate interface implemented by TcpTransceiver or IceSSL.TransceiverI or any endpoint that WS can
// delegate to.
//
public interface WSTransceiverDelegate
{
    Ice.ConnectionInfo getWSInfo(java.util.Map<String, String> headers);
};
