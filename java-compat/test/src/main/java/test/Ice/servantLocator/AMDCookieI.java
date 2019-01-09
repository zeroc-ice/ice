// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.AMD.Test.Cookie;

public final class AMDCookieI extends Cookie
{
    @Override
    public String
    message()
    {
        return "blahblah";
    }
}
