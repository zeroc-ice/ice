// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
