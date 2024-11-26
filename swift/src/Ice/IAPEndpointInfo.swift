// Copyright (c) ZeroC, Inc.

import Foundation

/// Provides access to an IAP endpoint information.
public final class IAPEndpointInfo: EndpointInfo {
    /// The accessory manufacturer or empty to not match against a manufacturer.
    public let manufacturer: String
    /// The accessory model number or empty to not match against a model number.
    public let modelNumber: String
    /// The accessory name or empty to not match against the accessory name.
    public let name: String
    /// The protocol supported by the accessory.
    public let `protocol`: String

    private let _type: Int16
    private let _secure: Bool

    public override func type() -> Int16 {
        _type
    }

    public override func secure() -> Bool {
        _secure
    }

    internal init(
        timeout: Int32, compress: Bool, manufacturer: String, modelNumber: String, name: String, protocol: String,
        type: Int16, secure: Bool
    ) {
        self.manufacturer = manufacturer
        self.modelNumber = modelNumber
        self.name = name
        self.`protocol` = `protocol`
        self._type = type
        self._secure = secure
        super.init(timeout: timeout, compress: compress)
    }
}
