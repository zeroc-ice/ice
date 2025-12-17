// Copyright (c) ZeroC, Inc.

/// Represents a set of options that you can specify when initializing a communicator.
public struct InitializationData {
    /// The properties for the communicator.
    /// When non-nil, this corresponds to the object returned by ``Communicator/getProperties()``.
    public var properties: Properties?

    /// The logger for the communicator.
    public var logger: Logger?

    /// The Slice loader, used to unmarshal Slice classes and exceptions.
    public var sliceLoader: SliceLoader?

    /// Creates an InitializationData value.
    ///
    /// - Parameters:
    ///   - properties: The properties for the communicator.
    ///   - logger: The logger for the communicator.
    ///   - sliceLoader: The Slice loader, used to unmarshal Slice classes and exceptions.
    public init(properties: Properties? = nil, logger: Logger? = nil, sliceLoader: SliceLoader? = nil) {
        self.properties = properties
        self.logger = logger
        self.sliceLoader = sliceLoader
    }
}
