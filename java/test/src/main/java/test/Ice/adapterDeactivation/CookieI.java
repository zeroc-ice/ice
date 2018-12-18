// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.Cookie;

public final class CookieI extends Cookie
{
    public String message()
    {
        return "blahblah";
    }
}
