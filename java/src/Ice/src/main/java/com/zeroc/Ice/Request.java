// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Interface for incoming requests.
 **/
public interface Request
{
    /**
     * Returns the {@link Current} object for this the request.
     *
     * @return The {@link Current} object for this request.
     **/
    Current getCurrent();
}
