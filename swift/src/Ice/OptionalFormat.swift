// Copyright (c) ZeroC, Inc.

/// The optional format, used for marshaling optional fields and arguments. It describes how the data is marshaled and
/// how it can be skipped by the unmarshaling code if the optional isn't known to the receiver.
public enum OptionalFormat: UInt8 {
    /// Fixed 1-byte encoding.
    case F1 = 0

    /// Fixed 2-byte encoding.
    case F2 = 1

    /// Fixed 4-byte encoding.
    case F4 = 2

    /// Fixed 8-byte encoding.
    case F8 = 3

    /// "Size encoding" using either 1 or 5 bytes. Used by enums, class identifiers, etc.
    case Size = 4

    /// Variable "size encoding" using either 1 or 5 bytes followed by data.
    /// Used by strings, fixed-size structs, and containers whose size can be computed prior to marshaling.
    case VSize = 5

    /// Fixed "size encoding" using 4 bytes followed by data.
    /// Used by variable-size structs and containers whose sizes can't be computed prior to unmarshaling.
    case FSize = 6

    /// Class instance. No longer supported.
    case Class = 7

    init?(fixedSize: Int) {
        switch fixedSize {
        case 1:
            self.init(rawValue: 0)
        case 2:
            self.init(rawValue: 1)
        case 4:
            self.init(rawValue: 2)
        case 8:
            self.init(rawValue: 3)
        default:
            return nil
        }
    }
}
