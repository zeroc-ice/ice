// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    public interface Transceiver
    {
	System.Net.Sockets.Socket fd();
	void close();
	void shutdown();
	void write(BasicStream stream, int timeout);
	void read(BasicStream stream, int timeout);
	string type();
    }

}
