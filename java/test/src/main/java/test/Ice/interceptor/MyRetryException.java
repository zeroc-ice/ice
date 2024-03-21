//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

public class MyRetryException extends com.zeroc.Ice.LocalException
{
    public MyRetryException() {}

    @Override public String ice_id() { return "::MyRetryException"; }
}
