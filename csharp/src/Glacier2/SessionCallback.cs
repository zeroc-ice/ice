// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Glacier2
{
/// <summary>
/// A callback class to get notifications of status changes in the
/// Glacier2 session. All callbacks on the SessionCallback interface
/// occur in the main swing thread.
/// </summary>
public interface SessionCallback
{
    /// <summary>
    /// Notifies the application that the communicator was created.
    /// </summary>
    /// <param name="session">The Glacier2 session.</param>
    void createdCommunicator(SessionHelper session);

    /// <summary>
    /// Notifies the application that the Glacier2 session has
    /// been established.
    /// </summary>
    /// <param name="session">The established session.</param>
    void connected(SessionHelper session);
    
    /// <summary>
    /// Notifies the application that the Glacier2 session has been
    /// disconnected.
    /// </summary>
    /// <param name="session">The disconnected session.</param>
    void disconnected(SessionHelper session);

    /// <summary>
    /// Notifies the application that the Glacier2 session
    /// establishment failed. 
    /// </summary>
    /// <param name="session">The session reporting the connection
    /// failure.</param>
    /// <param name="ex">The exception.</param>
    void connectFailed(SessionHelper session, Exception ex);
}

}
