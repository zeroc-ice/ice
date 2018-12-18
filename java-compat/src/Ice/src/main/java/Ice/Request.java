// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

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
