// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

/**
 * Handles fatal errors encountered by the background save evictor.
 */
public interface FatalErrorCallback
{
    /**
     * Called when the background save evictor encounters a fatal error.
     *
     * @param evictor The evictor that detected the error.
     * @param communicator The communicator associated with the evictor.
     * @param ex The exception that caused the error.
     */
    void handleError(Evictor evictor, Ice.Communicator communicator, RuntimeException ex);
}
