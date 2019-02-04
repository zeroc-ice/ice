//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
