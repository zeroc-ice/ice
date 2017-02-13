// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Glacier2;

/**
 * A callback class to get notifications of status changes in the Glacier2 session.
 * All callbacks on the <code>Callback</code> interface occur in the main swing thread.
 */
public interface SessionCallback
{
    /**
     * Notifies the application that the communicator was created.
     *
     * @param session The Glacier2 session.
     */
    void createdCommunicator(SessionHelper session);

    /**
     * Notifies the application that the Glacier2 session has been established.
     *  
     * @param session The established session.
     *
     * @throws SessionNotExistException If the session no longer exists.
     */
    void connected(SessionHelper session)
        throws SessionNotExistException;

    /**
     * Notifies the application that the Glacier2 session has been disconnected.
     *
     * @param session The disconnected session.
     */
    void disconnected(SessionHelper session);

    /**
     * Notifies the application that the Glacier2 session establishment failed. 
     * 
     * @param session The session reporting the connection
     * failure.
     * @param ex The exception.
     */
    void connectFailed(SessionHelper session, Throwable ex);
}
