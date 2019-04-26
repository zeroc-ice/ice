//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Dispatch

public struct InitializationData {
    public var properties: Properties?
    public var logger: Logger?
    public var classResolverPrefix: [String]?

    public init() {}
}
