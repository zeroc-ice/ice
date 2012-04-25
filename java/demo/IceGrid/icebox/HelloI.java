// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    HelloI(String serviceName)
    {
        _serviceName = serviceName;
    }

    public void
    sayHello(Ice.Current current)
    {
        System.out.println("Hello from " + _serviceName);
    }

    private String _serviceName;
}
