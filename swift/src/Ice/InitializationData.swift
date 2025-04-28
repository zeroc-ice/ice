// Copyright (c) ZeroC, Inc.

/// Settings optionally supplied during communicator initialization.
public struct InitializationData {
    /// The properties for the communicator.
    public var properties: Properties?

    /// The logger for the communicator.
    public var logger: Logger?

    public var sliceLoader: SliceLoader?

    public init() {}
}
