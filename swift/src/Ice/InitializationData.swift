//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// A structured type that encpasulates data to initialize a communicator.
public struct InitializationData {
    /// The properties for the communicator.
    public var properties: Properties?

    /// The logger for the communicator.
    public var logger: Logger?

    public var classResolverPrefix: [String]?

    public init() {}
}
