// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

public final class DummyLogger implements com.zeroc.Ice.Logger
{
    @Override
    public void print(String message)
    {
        _logger.print(message);
    }

    @Override
    public void trace(String category, String message)
    {
         _logger.trace(category, message);
    }

    @Override
    public void warning(String message)
    {
        if(!message.contains("test.Ice.exceptions.ThrowerI.throwAssertException") &&
           !message.contains("test.Ice.exceptions.AMDThrowerI.throwAssertException_async"))
        {
            _logger.warning(message);
        }
    }

    @Override
    public void error(String message)
    {
        if(!message.contains("test.Ice.exceptions.ThrowerI.throwAssertException") &&
           !message.contains("test.Ice.exceptions.AMDThrowerI.throwAssertException_async"))
        {
            _logger.error(message);
        }
    }

    @Override
    public String getPrefix()
    {
        return "";
    }

    @Override
    public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
    {
        return new DummyLogger();
    }

    private com.zeroc.Ice.Logger _logger = new com.zeroc.Ice.LoggerI("", "");
}
