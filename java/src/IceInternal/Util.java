// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Util
{
    public static Instance
    getInstance(Ice.Communicator communicator)
    {
        Ice.CommunicatorI p = (Ice.CommunicatorI)communicator;
        return p.getInstance();
    }

    public static ProtocolPluginFacade
    getProtocolPluginFacade(Ice.Communicator communicator)
    {
        return new ProtocolPluginFacadeI(communicator);
    }
}
