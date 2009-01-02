// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class Ex
{
    public static void throwUOE(String expectedType, String actualType)
    {
        throw new Ice.UnexpectedObjectException(
                    "expected element of type `" + expectedType + "' but received '" + actualType,
                    actualType, expectedType);
    }

}
