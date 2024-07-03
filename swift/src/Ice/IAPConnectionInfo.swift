// Copyright (c) ZeroC, Inc.

import Foundation

/// Provides access to the connection details of an IAP connection.
public protocol IAPConnectionInfo: ConnectionInfo {
    /// The accessory name.
    var name: String { get set }
    /// The accessory manufacturer.
    var manufacturer: String { get set }
    /// The accessory model number.
    var modelNumber: String { get set }
    /// The accessory firmare revision.
    var firmwareRevision: String { get set }
    /// The accessory hardware revision.
    var hardwareRevision: String { get set }
    /// The protocol used by the accessory.
    var `protocol`: String { get set }
}
