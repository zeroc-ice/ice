// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface Hold
    {
        /// Puts the adapter on hold, and optionally reactivates it.
        /// @param delay: when negative, puts the adapter on hold indefinitely. When 0, puts the adapter on hold and
        /// immediately reactivates it. When greater than 0, starts a background task that sleeps for delay ms,
        /// puts the adapter on hold and then immediately reactivates it.
        void putOnHold(int delay);

        /// In a background task, call waitForHold on the adapter, followed by a call to activate.
        void waitForHold();

        /// Shuts down the server.
        void shutdown();
    }
}
