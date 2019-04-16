//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

extension G {
    var _GDescription: String {
        return ":\nG: \(data)"
    }
}

extension H {
    var _HDescription: String {
        return ":\nH: \(data)"
    }
}

class EmptyI: Empty {}
