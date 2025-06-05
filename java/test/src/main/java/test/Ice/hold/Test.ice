// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.hold.Test"]
module Test
{
    interface Hold
    {
        /// Puts the adapter on hold, and optionally reactivates it.
        /// @param delay When less than 0, puts the adapter on hold indefinitely. When 0, puts the adapter on hold and
        /// immediately reactivates it. When greater than 0, starts a background task that sleeps for delay
        /// milliseconds, puts the adapter on hold and then immediately reactivates it.
        void putOnHold(int delay);

        /// Starts a background task that calls waitForHold and activate on the adapter.
        void waitForHold();

        /// Saves value as the last value.
        /// @param value The new value.
        /// @param expected The current value as expected by the caller.
        void setOneway(int value, int expected);

        /// Saves value as the last value after a delay.
        /// @param value The new value.
        /// @param delay The delay in milliseconds.
        /// @return The previous value.
        int set(int value, int delay);

        /// Shuts down the server.
        void shutdown();
    }
}
