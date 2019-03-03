//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args _: [String]) throws {
        let writer = getWriter()
        do {
            writer.write(data: "testing default values... ")
            let v = Struct1()
            try test(value: !v.boolFalse)
            try test(value: v.boolTrue)
            try test(value: v.b == 254)
            try test(value: v.s == 16000)
            try test(value: v.i == 3)
            try test(value: v.l == 4)
            try test(value: v.f == Float32(5.1))
            try test(value: v.d == Double(6.2))
            try test(value: v.str == "foo \\ \"bar\n \r\n\t\u{000b}\u{000C}\u{0007}\u{0008}? \u{0007} \u{0007}")
            try test(value: v.c1 == Color.red)
            try test(value: v.c2 == Color.green)
            try test(value: v.c3 == Color.blue)
            try test(value: v.nc1 == NestedColor.red)
            try test(value: v.nc2 == NestedColor.green)
            try test(value: v.nc3 == NestedColor.blue)
            try test(value: v.noDefault == "")
            try test(value: v.zeroI == 0)
            try test(value: v.zeroL == 0)
            try test(value: v.zeroF == 0)
            try test(value: v.zeroDotF == 0)
            try test(value: v.zeroD == 0)
            try test(value: v.zeroDotD == 0)
            writer.writeLine(data: "ok")
        }
    }
}
