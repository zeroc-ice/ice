// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interceptor;

public class MySystemException extends com.zeroc.Ice.SystemException
{
    public MySystemException()
    {
    }

    @Override
    public String ice_id()
    {
        return "::MySystemException";
    }
}
