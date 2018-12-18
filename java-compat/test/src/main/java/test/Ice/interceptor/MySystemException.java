// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.interceptor;

public class MySystemException extends Ice.SystemException
{
    public MySystemException()
    {
    }

    @Override
    public String
    ice_id()
    {
        return "::MySystemException";
    }
}
