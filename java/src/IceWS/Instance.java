// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

class Instance extends IceInternal.ProtocolInstance
{
    Instance(Ice.Communicator communicator, short type, String protocol)
    {
        super(communicator, type, protocol);
    }
}
