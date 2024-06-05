// Copyright (c) ZeroC, Inc.

import Foundation

/// Provides access to an IAP endpoint information.
public protocol IAPEndpointInfo: EndpointInfo {
    /// The accessory manufacturer or empty to not match against a manufacturer.
    var manufacturer: String { get set }
    /// The accessory model number or empty to not match against a model number.
    var modelNumber: String { get set }
    /// The accessory name or empty to not match against the accessory name.
    var name: String { get set }
    /// The protocol supported by the accessory.
    var `protocol`: String { get set }
}
