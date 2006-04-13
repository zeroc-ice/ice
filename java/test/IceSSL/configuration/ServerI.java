// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ServerI extends Test._ServerDisp
{
    ServerI(Ice.Communicator communicator)
    {
	_communicator = communicator;
    }

    public void
    destroy()
    {
	_communicator.destroy();
    }

    private Ice.Communicator _communicator;
}
