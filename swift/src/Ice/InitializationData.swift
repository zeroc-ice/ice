//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Dispatch

public struct InitializationData {
    public init() {}

    public var properties: Properties?
    public var logger: Logger?
    public var adminDispatchQueue: DispatchQueue?
    public var classResolverPrefix: String?
}
