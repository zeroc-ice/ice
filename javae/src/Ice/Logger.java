// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface Logger
{
    void print(String message);

    void trace(String category, String message);

    void warning(String message);

    void error(String message);
}
