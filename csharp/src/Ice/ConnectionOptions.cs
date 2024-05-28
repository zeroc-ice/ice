// Copyright (c) ZeroC, Inc. All rights reserved.

#nullable enable

namespace Ice;

internal sealed record class ConnectionOptions
{
    internal TimeSpan connectTimeout { get; init; }
    internal TimeSpan closeTimeout { get; init; }
    internal TimeSpan idleTimeout { get; init; }
    internal bool enableIdleCheck { get; init; }
    internal TimeSpan inactivityTimeout { get; init; }
}
