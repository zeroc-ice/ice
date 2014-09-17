// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



public class LoggerI implements Ice.Logger
{
    @Override
    public void
    print(String message)
    {
        System.out.println("PRINT: " + message);
    }
    
    @Override
    public void
    trace(String category, String message)
    {
        System.out.println("TRACE(" + category + "): " + message);
    }
    
    @Override
    public void
    warning(String message)
    {
        System.out.println("WARNING: " + message);
    }

    @Override
    public void
    error(String message)
    {
        System.out.println("ERROR: " + message);
    }

    @Override
    public String
    getPrefix()
    {
        return "";
    }

    @Override
    public Ice.Logger
    cloneWithPrefix(String prefix)
    {
        return new LoggerI();
    }
}
