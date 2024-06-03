//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// Helper for encoding of optional member or parameter.
public enum OptionalFormat: UInt8 {
    case F1 = 0
    case F2 = 1
    case F4 = 2
    case F8 = 3
    case Size = 4
    case VSize = 5
    case FSize = 6
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
