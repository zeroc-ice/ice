// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Ice.Logger;

public class LoggerI implements Ice.Logger
{
    public void
    print(String message)
    {
        System.out.println("PRINT: " + message);
    }
    
    public void
    trace(String category, String message)
    {
        System.out.println("TRACE(" + category + "): " + message);
    }
    
    public void
    warning(String message)
    {
        System.out.println("WARNING: " + message);
    }

    public void
    error(String message)
    {
        System.out.println("ERROR: " + message);
    }

    public Ice.Logger
    cloneWithPrefix(String prefix)
    {
        return new LoggerI();
    }
}
