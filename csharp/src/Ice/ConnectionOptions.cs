// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

internal sealed record class ConnectionOptions(
    TimeSpan connectTimeout,
    TimeSpan closeTimeout,
    TimeSpan idleTimeout,
    bool enableIdleCheck,
    TimeSpan inactivityTimeout,
    int maxDispatches);
