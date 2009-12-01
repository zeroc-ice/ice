// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public delegate void DispatcherCall();
    
    public interface Dispatcher
    {
        void dispatch(DispatcherCall call, Ice.Connection con);
    }
}