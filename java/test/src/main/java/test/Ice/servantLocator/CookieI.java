// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.Cookie;

public final class CookieI extends Cookie
{
    @Override
    public String message()
    {
        return "blahblah";
    }
}
