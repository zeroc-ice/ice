// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
