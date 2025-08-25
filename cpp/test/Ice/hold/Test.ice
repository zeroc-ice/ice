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
        ["amd"] void putOnHold(int delay);

        /// Starts a background task that calls `waitForHold` and `activate` on the adapter.
        ["amd"] void waitForHold();

        /// Saves @p value as the last value.
        /// @param value The new value.
        /// @param expected The current value as expected by the caller.
        void setOneway(int value, int expected);

        /// Saves @p value as the last value after a delay.
        /// @param value The new value.
        /// @param delay The delay in milliseconds.
        /// @return The previous value.
        int set(int value, int delay);

        /// Shuts down the server.
        void shutdown();
    }
}
