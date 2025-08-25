// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface Hold
    {
        /// Puts the adapter on hold, and optionally reactivates it.
        /// @param delay When less than 0, puts the adapter on hold indefinitely. When 0, puts the adapter on hold and
        /// immediately reactivates it. When greater than 0, starts a background task that sleeps for @p delay many
        /// milliseconds, puts the adapter on hold and then immediately reactivates it.
        void putOnHold(int delay);

        /// Starts a background task that calls `waitForHold` and `activate` on the adapter.
        void waitForHold();

        /// Shuts down the server.
        void shutdown();
    }
}
