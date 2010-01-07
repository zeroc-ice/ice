// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Glacier2
{
/// <sumary>
/// A callback class to get notifications of status changes in the
/// Glacier2 session. All callbacks on the SessionCallback interface
/// occur in the main swing thread.
/// </sumary>
public interface SessionCallback
{
    /// <sumary>
    /// Notifies the application that the communicator was created.
    /// </sumary>
    /// <param name="session">The Glacier2 session.</param>
    void
    createdCommunicator(SessionHelper session);

    /// <sumary>
    /// Notifies the application that the Glacier2 session has
    /// been established.
    /// </sumary>
    /// <param name="session">The established session.</param>
    void
    connected(SessionHelper session);
    
    /// <sumary>
    /// Notifies the application that the Glacier2 session has been
    /// disconnected.
    /// </sumary>
    /// <param name="session">The disconnected session.</param>
    void
    disconnected(SessionHelper session);

    /// <sumary>
    /// Notifies the application that the Glacier2 session
    /// establishment failed. 
    /// </sumary>
    /// <param name="session">The session reporting the connection
    /// failure.</param>
    /// <param name="ex">The exception.</param>
    void
    connectFailed(SessionHelper session, Exception ex);
}

}
