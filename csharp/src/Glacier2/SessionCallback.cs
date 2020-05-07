//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Glacier2
{
    /// <summary>A callback class to get notifications of status changes in the Glacier2 session. All callbacks on the
    /// SessionCallback interface occur in the main swing thread.</summary>
    public interface ISessionCallback
    {
        /// <summary>Notifies the application that the Glacier2 session has been established.</summary>
        /// <param name="session">The established session.</param>
        void Connected(SessionHelper session);

        /// <summary>Notifies the application that the Glacier2 session establishment failed.</summary>
        /// <param name="session">The session reporting the connection failure.</param>
        /// <param name="ex">The exception.</param>
        void ConnectFailed(SessionHelper session, Exception ex);

        /// <summary>Notifies the application that the communicator was created.</summary>
        /// <param name="session">The Glacier2 session.</param>
        void CreatedCommunicator(SessionHelper session);

        /// <summary>Notifies the application that the Glacier2 session has been disconnected.</summary>
        /// <param name="session">The disconnected session.</param>
        void Disconnected(SessionHelper session);
    }
}
