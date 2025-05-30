// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

// temporary work-around for issue #816
func twoways(_ helper: TestHelper, _ p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    try await testLiterals(helper: helper, prx: p)

    try await testProxyMethods(helper: helper, prx: p)

    try await testVoid(helper: helper, prx: p)

    try await testNumerics(helper: helper, prx: p)

    try await testStrings(helper: helper, prx: p)

    try await testEnums(helper: helper, prx: p)
    try await testClasses(helper: helper, prx: p)
    try await testStructs(helper: helper, prx: p)

    try await testSequences(helper: helper, prx: p)
    try await testDictionaries(helper: helper, prx: p)

    try await testContext(helper: helper, prx: p)

    try await testIdempotent(helper: helper, prx: p)

    do {
        try await test(p.opByte1(0xFF) == 0xFF)
        try await test(p.opShort1(0x7FFF) == 0x7FFF)
        try await test(p.opInt1(0x7FFF_FFFF) == 0x7FFF_FFFF)
        try await test(p.opLong1(0x7FFF_FFFF_FFFF_FFFF) == 0x7FFF_FFFF_FFFF_FFFF)
        try await test(p.opFloat1(1.0) == 1.0)
        try await test(p.opDouble1(1.0) == 1.0)
        try await test(p.opString1("opString1") == "opString1")
        try await test(p.opStringS1([]).count == 0)
        try await test(p.opByteBoolD1([:]).count == 0)
        try await test(p.opStringS2([]).count == 0)
        try await test(p.opByteBoolD2([:]).count == 0)

        let d = uncheckedCast(prx: p, type: MyDerivedClassPrx.self)
        var s = MyStruct1()
        s.tesT = "MyStruct1.s"
        s.myClass = nil
        s.myStruct1 = "MyStruct1.myStruct1"
        s = try await d.opMyStruct1(s)
        try test(s.tesT == "MyStruct1.s")
        try test(s.myClass == nil)
        try test(s.myStruct1 == "MyStruct1.myStruct1")
        var c = MyClass1()
        c.tesT = "MyClass1.testT"
        c.myClass = nil
        c.myClass1 = "MyClass1.myClass1"
        c = try await d.opMyClass1(c)!
        try test(c.tesT == "MyClass1.testT")
        try test(c.myClass == nil)
        try test(c.myClass1 == "MyClass1.myClass1")
    }
}

func testLiterals(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let literals = try await p.opStringLiterals()

    try test(s0 == "\\" && s0 == sw0 && s0 == literals[0] && s0 == literals[11])

    try test(s1 == "A" && s1 == sw1 && s1 == literals[1] && s1 == literals[12])

    try test(s2 == "Ice" && s2 == sw2 && s2 == literals[2] && s2 == literals[13])

    try test(s3 == "A21" && s3 == sw3 && s3 == literals[3] && s3 == literals[14])

    try test(s4 == "\\u0041 \\U00000041" && s4 == sw4 && s4 == literals[4] && s4 == literals[15])

    try test(s5 == "\u{00FF}" && s5 == sw5 && s5 == literals[5] && s5 == literals[16])

    try test(s6 == "\u{03FF}" && s6 == sw6 && s6 == literals[6] && s6 == literals[17])

    try test(s7 == "\u{05F0}" && s7 == sw7 && s7 == literals[7] && s7 == literals[18])

    try test(s8 == "\u{10000}" && s8 == sw8 && s8 == literals[8] && s8 == literals[19])

    try test(s9 == "\u{1F34C}" && s9 == sw9 && s9 == literals[9] && s9 == literals[20])

    try test(s10 == "\u{0DA7}" && s10 == sw10 && s10 == literals[10] && s10 == literals[21])

    try test(
        ss0 == "\'\"\u{003f}\\\u{0007}\u{0008}\u{000c}\n\r\t\u{000b}\u{0006}" && ss0 == ss1
            && ss0 == ss2 && ss0 == literals[22] && ss0 == literals[23] && ss0 == literals[24])

    try test(ss3 == "\\\\U\\u\\" && ss3 == literals[25])

    try test(ss4 == "\\A\\" && ss4 == literals[26])

    try test(ss5 == "\\u0041\\" && ss5 == literals[27])

    try test(
        su0 == su1 && su0 == su2 && su0 == literals[28] && su0 == literals[29] && su0 == literals[30])
}

func testProxyMethods(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    try await p.ice_ping()

    try test(ice_staticId(MyClassPrx.self) == "::Test::MyClass")
    try test(ice_staticId(Ice.ObjectPrx.self) == "::Ice::Object")

    try await test(p.ice_isA(id: ice_staticId(MyClassPrx.self)))
    try await test(p.ice_id() == ice_staticId(MyDerivedClassPrx.self))

    let ids = try await p.ice_ids()
    try test(ids.count == 3)
    try test(ids[0] == "::Ice::Object")
    try test(ids[1] == "::Test::MyClass")
    try test(ids[2] == "::Test::MyDerivedClass")
}

func testVoid(helper: TestHelper, prx p: MyClassPrx) async throws {
    try await p.opVoid()
}

func testNumerics(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let (r, b) = try await p.opByte(p1: 0xFF, p2: 0x0F)
        try test(b == 0xF0)
        try test(r == 0xFF)
    }

    do {
        let (r, b) = try await p.opBool(p1: true, p2: false)
        try test(b)
        try test(!r)
    }

    do {
        var (r, s, i, l) = try await p.opShortIntLong(p1: 10, p2: 11, p3: 12)
        try test(s == 10)
        try test(i == 11)
        try test(l == 12)
        try test(r == 12)

        (r, s, i, l) = try await p.opShortIntLong(p1: Int16.min, p2: Int32.min, p3: Int64.min)
        try test(s == Int16.min)
        try test(i == Int32.min)
        try test(l == Int64.min)
        try test(r == Int64.min)

        (r, s, i, l) = try await p.opShortIntLong(p1: Int16.max, p2: Int32.max, p3: Int64.max)
        try test(s == Int16.max)
        try test(i == Int32.max)
        try test(l == Int64.max)
        try test(r == Int64.max)
    }

    do {
        var (r, f, d) = try await p.opFloatDouble(p1: 3.14, p2: 1.1e10)
        try test(f == 3.14)
        try test(d == 1.1e10)
        try test(r == 1.1e10)

        (r, f, d) = try await p.opFloatDouble(p1: Float.ulpOfOne, p2: Double.leastNormalMagnitude)
        try test(f == Float.ulpOfOne)
        try test(d == Double.leastNormalMagnitude)
        try test(r == Double.leastNormalMagnitude)

        (r, f, d) = try await p.opFloatDouble(
            p1: Float.greatestFiniteMagnitude, p2: Double.greatestFiniteMagnitude)
        try test(f == Float.greatestFiniteMagnitude)
        try test(d == Double.greatestFiniteMagnitude)
        try test(r == Double.greatestFiniteMagnitude)
    }
}

func testStrings(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let (r, s) = try await p.opString(p1: "hello", p2: "world")
        try test(s == "world hello")
        try test(r == "hello world")
    }
}

func testEnums(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let (r, e) = try await p.opMyEnum(MyEnum.enum2)
        try test(e == MyEnum.enum2)
        try test(r == MyEnum.enum3)
    }

}

func testClasses(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        var (r, c1, c2) = try await p.opMyClass(p)

        try test(c1!.ice_getIdentity() == Ice.Identity(name: "test"))
        try test(c2!.ice_getIdentity() == Ice.stringToIdentity("noSuchIdentity"))
        try test(r!.ice_getIdentity() == Ice.Identity(name: "test"))
        try await r!.opVoid()
        try await c1!.opVoid()

        do {
            try await c2!.opVoid()
            try test(false)
        } catch is Ice.ObjectNotExistException {}

        (r, c1, c2) = try await p.opMyClass(nil)
        try test(c1 == nil)
        try test(c2 != nil)
        try await r!.opVoid()
    }
}

func testStructs(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        var si1 = Structure()
        si1.p = p
        si1.e = MyEnum.enum3
        si1.s = AnotherStruct()
        si1.s.s = "abc"
        var si2 = Structure()
        si2.p = nil
        si2.e = MyEnum.enum2
        si2.s = AnotherStruct()
        si2.s.s = "def"

        let (rso, so) = try await p.opStruct(p1: si1, p2: si2)
        try test(rso.p == nil)
        try test(rso.e == MyEnum.enum2)
        try test(rso.s.s == "def")
        try test(so.p == p)
        try test(so.e == MyEnum.enum3)
        try test(so.s.s == "a new string")
        try await so.p!.opVoid()
    }
}

func testSequences(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let bsi1 = ByteSeq([0x01, 0x11, 0x12, 0x22])
        let bsi2 = ByteSeq([0xF1, 0xF2, 0xF3, 0xF4])

        let (rso, bso) = try await p.opByteS(p1: bsi1, p2: bsi2)
        try test(bso.count == 4)
        try test(bso[0] == 0x22)
        try test(bso[1] == 0x12)
        try test(bso[2] == 0x11)
        try test(bso[3] == 0x01)
        try test(rso.count == 8)
        try test(rso[0] == 0x01)
        try test(rso[1] == 0x11)
        try test(rso[2] == 0x12)
        try test(rso[3] == 0x22)
        try test(rso[4] == 0xF1)
        try test(rso[5] == 0xF2)
        try test(rso[6] == 0xF3)
        try test(rso[7] == 0xF4)
    }

    do {
        let bsi1: [Bool] = [true, true, false]
        let bsi2 = [false]

        let (rso, bso) = try await p.opBoolS(p1: bsi1, p2: bsi2)
        try test(bso.count == 4)
        try test(bso[0])
        try test(bso[1])
        try test(!bso[2])
        try test(!bso[3])
        try test(rso.count == 3)
        try test(!rso[0])
        try test(rso[1])
        try test(rso[2])
    }

    do {
        let ssi: [Int16] = [1, 2, 3]
        let isi: [Int32] = [5, 6, 7, 8]
        let lsi: [Int64] = [10, 30, 20]

        let (rso, sso, iso, lso) = try await p.opShortIntLongS(p1: ssi, p2: isi, p3: lsi)
        try test(sso.count == 3)
        try test(sso[0] == 1)
        try test(sso[1] == 2)
        try test(sso[2] == 3)
        try test(iso.count == 4)
        try test(iso[0] == 8)
        try test(iso[1] == 7)
        try test(iso[2] == 6)
        try test(iso[3] == 5)
        try test(lso.count == 6)
        try test(lso[0] == 10)
        try test(lso[1] == 30)
        try test(lso[2] == 20)
        try test(lso[3] == 10)
        try test(lso[4] == 30)
        try test(lso[5] == 20)
        try test(rso.count == 3)
        try test(rso[0] == 10)
        try test(rso[1] == 30)
        try test(rso[2] == 20)
    }

    do {
        let fsi: [Float] = [3.14, 1.11]
        let dsi: [Double] = [1.1e10, 1.2e10, 1.3e10]

        let (rso, fso, dso) = try await p.opFloatDoubleS(p1: fsi, p2: dsi)
        try test(fso.count == 2)
        try test(fso[0] == 3.14)
        try test(fso[1] == 1.11)
        try test(dso.count == 3)
        try test(dso[0] == 1.3e10)
        try test(dso[1] == 1.2e10)
        try test(dso[2] == 1.1e10)
        try test(rso.count == 5)
        try test(rso[0] == 1.1e10)
        try test(rso[1] == 1.2e10)
        try test(rso[2] == 1.3e10)
        try test(Float(rso[3]) == 3.14)
        try test(Float(rso[4]) == 1.11)
    }

    do {
        let ssi1 = ["abc", "de", "fghi"]
        let ssi2 = ["xyz"]

        let (rso, sso) = try await p.opStringS(p1: ssi1, p2: ssi2)
        try test(sso.count == 4)
        try test(sso[0] == "abc")
        try test(sso[1] == "de")
        try test(sso[2] == "fghi")
        try test(sso[3] == "xyz")
        try test(rso.count == 3)
        try test(rso[0] == "fghi")
        try test(rso[1] == "de")
        try test(rso[2] == "abc")
    }

    do {
        let s11 = ByteSeq([0x01, 0x11, 0x12])
        let s12 = ByteSeq([0xFF])
        let bsi1 = [s11, s12]

        let s21 = ByteSeq([0x0E])
        let s22 = ByteSeq([0xF2, 0xF1])
        let bsi2 = [s21, s22]

        let (rso, bso) = try await p.opByteSS(p1: bsi1, p2: bsi2)
        try test(bso.count == 2)
        try test(bso[0].count == 1)
        try test(bso[0][0] == 0xFF)
        try test(bso[1].count == 3)
        try test(bso[1][0] == 0x01)
        try test(bso[1][1] == 0x11)
        try test(bso[1][2] == 0x12)
        try test(rso.count == 4)
        try test(rso[0].count == 3)
        try test(rso[0][0] == 0x01)
        try test(rso[0][1] == 0x11)
        try test(rso[0][2] == 0x12)
        try test(rso[1].count == 1)
        try test(rso[1][0] == 0xFF)
        try test(rso[2].count == 1)
        try test(rso[2][0] == 0x0E)
        try test(rso[3].count == 2)
        try test(rso[3][0] == 0xF2)
        try test(rso[3][1] == 0xF1)
    }

    do {
        let s11 = [true]
        let s12 = [false]
        let s13 = [true, true]
        let bsi1 = [s11, s12, s13]

        let s21 = [false, false, true]
        let bsi2 = [s21]

        let (rso, bso) = try await p.opBoolSS(p1: bsi1, p2: bsi2)
        try test(bso.count == 4)
        try test(bso[0].count == 1)
        try test(bso[0][0])
        try test(bso[1].count == 1)
        try test(!bso[1][0])
        try test(bso[2].count == 2)
        try test(bso[2][0])
        try test(bso[2][1])
        try test(bso[3].count == 3)
        try test(!bso[3][0])
        try test(!bso[3][1])
        try test(bso[3][2])
        try test(rso.count == 3)
        try test(rso[0].count == 2)
        try test(rso[0][0])
        try test(rso[0][1])
        try test(rso[1].count == 1)
        try test(!rso[1][0])
        try test(rso[2].count == 1)
        try test(rso[2][0])
    }

    do {
        let s11: [Int16] = [1, 2, 5]
        let s12: [Int16] = [13]
        let s13: [Int16] = []
        let ssi = [s11, s12, s13]

        let i11: [Int32] = [24, 98]
        let i12: [Int32] = [42]
        let isi = [i11, i12]

        let l11: [Int64] = [496, 1729]
        let lsi = [l11]

        let (rso, sso, iso, lso) = try await p.opShortIntLongSS(p1: ssi, p2: isi, p3: lsi)

        try test(rso.count == 1)
        try test(rso[0].count == 2)
        try test(rso[0][0] == 496)
        try test(rso[0][1] == 1729)
        try test(sso.count == 3)
        try test(sso[0].count == 3)
        try test(sso[0][0] == 1)
        try test(sso[0][1] == 2)
        try test(sso[0][2] == 5)
        try test(sso[1].count == 1)
        try test(sso[1][0] == 13)
        try test(sso[2].count == 0)
        try test(iso.count == 2)
        try test(iso[0].count == 1)
        try test(iso[0][0] == 42)
        try test(iso[1].count == 2)
        try test(iso[1][0] == 24)
        try test(iso[1][1] == 98)
        try test(lso.count == 2)
        try test(lso[0].count == 2)
        try test(lso[0][0] == 496)
        try test(lso[0][1] == 1729)
        try test(lso[1].count == 2)
        try test(lso[1][0] == 496)
        try test(lso[1][1] == 1729)
    }

    do {
        let f11: [Float] = [3.14]
        let f12: [Float] = [1.11]
        let f13: [Float] = []
        let fsi = [f11, f12, f13]

        let d11: [Double] = [1.1e10, 1.2e10, 1.3e10]
        let dsi = [d11]

        let (rso, fso, dso) = try await p.opFloatDoubleSS(p1: fsi, p2: dsi)
        try test(fso.count == 3)
        try test(fso[0].count == 1)
        try test(fso[0][0] == 3.14)
        try test(fso[1].count == 1)
        try test(fso[1][0] == 1.11)
        try test(fso[2].count == 0)
        try test(dso.count == 1)
        try test(dso[0].count == 3)
        try test(dso[0][0] == 1.1e10)
        try test(dso[0][1] == 1.2e10)
        try test(dso[0][2] == 1.3e10)
        try test(rso.count == 2)
        try test(rso[0].count == 3)
        try test(rso[0][0] == 1.1e10)
        try test(rso[0][1] == 1.2e10)
        try test(rso[0][2] == 1.3e10)
        try test(rso[1].count == 3)
        try test(rso[1][0] == 1.1e10)
        try test(rso[1][1] == 1.2e10)
        try test(rso[1][2] == 1.3e10)
    }

    do {
        let s11 = ["abc"]
        let s12: [String] = ["de", "fghi"]
        let ssi1 = [s11, s12]

        let s21: [String] = []
        let s22: [String] = []
        let s23 = ["xyz"]
        let ssi2 = [s21, s22, s23]

        let (rso, sso) = try await p.opStringSS(p1: ssi1, p2: ssi2)
        try test(sso.count == 5)
        try test(sso[0].count == 1)
        try test(sso[0][0] == "abc")
        try test(sso[1].count == 2)
        try test(sso[1][0] == "de")
        try test(sso[1][1] == "fghi")
        try test(sso[2].count == 0)
        try test(sso[3].count == 0)
        try test(sso[4].count == 1)
        try test(sso[4][0] == "xyz")
        try test(rso.count == 3)
        try test(rso[0].count == 1)
        try test(rso[0][0] == "xyz")
        try test(rso[1].count == 0)
        try test(rso[2].count == 0)
    }

    do {
        let s111 = ["abc", "de"]
        let s112 = ["xyz"]
        let ss11 = [s111, s112]
        let s121 = ["hello"]
        let ss12 = [s121]
        let sssi1 = [ss11, ss12]

        let s211 = ["", ""]
        let s212 = ["abcd"]
        let ss21 = [s211, s212]
        let s221 = [""]
        let ss22 = [s221]
        let ss23 = [[String]]()
        let sssi2 = [ss21, ss22, ss23]

        let (rsso, ssso) = try await p.opStringSSS(p1: sssi1, p2: sssi2)
        try test(ssso.count == 5)
        try test(ssso[0].count == 2)
        try test(ssso[0][0].count == 2)
        try test(ssso[0][1].count == 1)
        try test(ssso[1].count == 1)
        try test(ssso[1][0].count == 1)
        try test(ssso[2].count == 2)
        try test(ssso[2][0].count == 2)
        try test(ssso[2][1].count == 1)
        try test(ssso[3].count == 1)
        try test(ssso[3][0].count == 1)
        try test(ssso[4].count == 0)
        try test(ssso[0][0][0] == "abc")
        try test(ssso[0][0][1] == "de")
        try test(ssso[0][1][0] == "xyz")
        try test(ssso[1][0][0] == "hello")
        try test(ssso[2][0][0] == "")
        try test(ssso[2][0][1] == "")
        try test(ssso[2][1][0] == "abcd")
        try test(ssso[3][0][0] == "")

        try test(rsso.count == 3)
        try test(rsso[0].count == 0)
        try test(rsso[1].count == 1)
        try test(rsso[1][0].count == 1)
        try test(rsso[2].count == 2)
        try test(rsso[2][0].count == 2)
        try test(rsso[2][1].count == 1)
        try test(rsso[1][0][0] == "")
        try test(rsso[2][0][0] == "")
        try test(rsso[2][0][1] == "")
        try test(rsso[2][1][0] == "abcd")
    }

    do {
        let lengths: [Int32] = [0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000]

        for l in 0..<lengths.count {
            var s = [Int32]()
            for i in 0..<lengths[l] {
                s.append(i)
            }

            let r = try await p.opIntS(s)
            try test(r.count == lengths[l])
            for j in 0..<r.count {
                try test(r[j] == -j)
            }
        }
    }
}

func testDictionaries(helper: TestHelper, prx p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let di1: [UInt8: Bool] = [10: true, 100: false]
        let di2: [UInt8: Bool] = [10: true, 11: false, 101: true]
        let (ro, `do`) = try await p.opByteBoolD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro[10] == true)
        try test(ro[11] == false)
        try test(ro[100] == false)
        try test(ro[101] == true)
    }

    do {
        let di1: [Int16: Int32] = [110: -1, 1100: 123_123]
        let di2: [Int16: Int32] = [110: -1, 111: -100, 1101: 0]
        let (ro, `do`) = try await p.opShortIntD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro[110] == -1)
        try test(ro[111] == -100)
        try test(ro[1100] == 123_123)
        try test(ro[1101] == 0)
    }

    do {
        let di1: [Int64: Float] = [999_999_110: -1.1, 999_999_111: 123_123.2]
        let di2: [Int64: Float] = [999_999_110: -1.1, 999_999_120: -100.4, 999_999_130: 0.5]
        let (ro, `do`) = try await p.opLongFloatD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro[999_999_110] == -1.1)
        try test(ro[999_999_120] == -100.4)
        try test(ro[999_999_111] == 123_123.2)
        try test(ro[999_999_130] == 0.5)
    }

    do {
        let di1 = ["foo": "abc -1.1", "bar": "abc 123123.2"]
        let di2 = ["foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5"]
        let (ro, `do`) = try await p.opStringStringD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro["foo"] == "abc -1.1")
        try test(ro["FOO"] == "abc -100.4")
        try test(ro["bar"] == "abc 123123.2")
        try test(ro["BAR"] == "abc 0.5")
    }

    do {
        let di1 = ["abc": MyEnum.enum1, "": MyEnum.enum2]
        let di2 = ["abc": MyEnum.enum1, "qwerty": MyEnum.enum3, "Hello!!": MyEnum.enum2]
        let (ro, `do`) = try await p.opStringMyEnumD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro["abc"] == MyEnum.enum1)
        try test(ro["qwerty"] == MyEnum.enum3)
        try test(ro[""] == MyEnum.enum2)
        try test(ro["Hello!!"] == MyEnum.enum2)
    }

    do {
        let di1 = [MyEnum.enum1: "abc"]
        let di2 = [MyEnum.enum2: "Hello!!", MyEnum.enum3: "qwerty"]
        let (ro, `do`) = try await p.opMyEnumStringD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 3)
        try test(ro[MyEnum.enum1] == "abc")
        try test(ro[MyEnum.enum2] == "Hello!!")
        try test(ro[MyEnum.enum3] == "qwerty")
    }

    do {
        let s11 = MyStruct(i: 1, j: 1)
        let s12 = MyStruct(i: 1, j: 2)
        let di1 = [s11: MyEnum.enum1, s12: MyEnum.enum2]

        let s22 = MyStruct(i: 2, j: 2)
        let s23 = MyStruct(i: 2, j: 3)
        let di2 = [s11: MyEnum.enum1, s22: MyEnum.enum3, s23: MyEnum.enum2]

        let (ro, `do`) = try await p.opMyStructMyEnumD(p1: di1, p2: di2)

        try test(`do` == di1)
        try test(ro.count == 4)
        try test(ro[s11] == MyEnum.enum1)
        try test(ro[s12] == MyEnum.enum2)
        try test(ro[s22] == MyEnum.enum3)
        try test(ro[s23] == MyEnum.enum2)
    }

    do {
        let di1: [UInt8: Bool] = [10: true, 100: false]
        let di2: [UInt8: Bool] = [10: true, 11: false, 101: true]
        let di3: [UInt8: Bool] = [100: false, 101: false]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opByteBoolDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0][10]!)
        try test(!ro[0][11]!)
        try test(ro[0][101]!)
        try test(ro[1].count == 2)
        try test(ro[1][10]!)
        try test(!ro[1][100]!)

        try test(`do`.count == 3)
        try test(`do`[0].count == 2)
        try test(!`do`[0][100]!)
        try test(!`do`[0][101]!)
        try test(`do`[1].count == 2)
        try test(`do`[1][10]!)
        try test(!`do`[1][100]!)
        try test(`do`[2].count == 3)
        try test(`do`[2][10]!)
        try test(!`do`[2][11]!)
        try test(`do`[2][101]!)
    }

    do {
        let di1: [Int16: Int32] = [110: -1, 1100: 123_123]
        let di2: [Int16: Int32] = [110: -1, 111: -100, 1101: 0]
        let di3: [Int16: Int32] = [100: -1001]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opShortIntDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0][110] == -1)
        try test(ro[0][111] == -100)
        try test(ro[0][1101] == 0)
        try test(ro[1].count == 2)
        try test(ro[1][110] == -1)
        try test(ro[1][1100] == 123_123)

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0][100] == -1001)
        try test(`do`[1].count == 2)
        try test(`do`[1][110] == -1)
        try test(`do`[1][1100] == 123_123)
        try test(`do`[2].count == 3)
        try test(`do`[2][110] == -1)
        try test(`do`[2][111] == -100)
        try test(`do`[2][1101] == 0)
    }

    do {
        let di1: [Int64: Float] = [999_999_110: -1.1, 999_999_111: 123_123.2]
        let di2: [Int64: Float] = [999_999_110: -1.1, 999_999_120: -100.4, 999_999_130: 0.5]
        let di3: [Int64: Float] = [999_999_140: 3.14]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opLongFloatDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0][999_999_110] == -1.1)
        try test(ro[0][999_999_120] == -100.4)
        try test(ro[0][999_999_130] == 0.5)
        try test(ro[1].count == 2)
        try test(ro[1][999_999_110] == -1.1)
        try test(ro[1][999_999_111] == 123_123.2)

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0][999_999_140] == 3.14)
        try test(`do`[1].count == 2)
        try test(`do`[1][999_999_110] == -1.1)
        try test(`do`[1][999_999_111] == 123_123.2)
        try test(`do`[2].count == 3)
        try test(`do`[2][999_999_110] == -1.1)
        try test(`do`[2][999_999_120] == -100.4)
        try test(`do`[2][999_999_130] == 0.5)
    }

    do {
        let di1 = ["foo": "abc -1.1", "bar": "abc 123123.2"]
        let di2 = ["foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5"]
        let di3 = ["f00": "ABC -3.14"]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opStringStringDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0]["foo"] == "abc -1.1")
        try test(ro[0]["FOO"] == "abc -100.4")
        try test(ro[0]["BAR"] == "abc 0.5")
        try test(ro[1].count == 2)
        try test(ro[1]["foo"] == "abc -1.1")
        try test(ro[1]["bar"] == "abc 123123.2")

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0]["f00"] == "ABC -3.14")
        try test(`do`[1].count == 2)
        try test(`do`[1]["foo"] == "abc -1.1")
        try test(`do`[1]["bar"] == "abc 123123.2")
        try test(`do`[2].count == 3)
        try test(`do`[2]["foo"] == "abc -1.1")
        try test(`do`[2]["FOO"] == "abc -100.4")
        try test(`do`[2]["BAR"] == "abc 0.5")
    }

    do {
        let di1 = ["abc": MyEnum.enum1, "": MyEnum.enum2]
        let di2 = ["abc": MyEnum.enum1, "qwerty": MyEnum.enum3, "Hello!!": MyEnum.enum2]
        let di3 = ["Goodbye": MyEnum.enum1]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opStringMyEnumDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0]["abc"] == MyEnum.enum1)
        try test(ro[0]["qwerty"] == MyEnum.enum3)
        try test(ro[0]["Hello!!"] == MyEnum.enum2)
        try test(ro[1].count == 2)
        try test(ro[1]["abc"] == MyEnum.enum1)
        try test(ro[1][""] == MyEnum.enum2)

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0]["Goodbye"] == MyEnum.enum1)
        try test(`do`[1].count == 2)
        try test(`do`[1]["abc"] == MyEnum.enum1)
        try test(`do`[1][""] == MyEnum.enum2)
        try test(`do`[2].count == 3)
        try test(`do`[2]["abc"] == MyEnum.enum1)
        try test(`do`[2]["qwerty"] == MyEnum.enum3)
        try test(`do`[2]["Hello!!"] == MyEnum.enum2)
    }

    do {
        let di1 = [MyEnum.enum1: "abc"]
        let di2 = [MyEnum.enum2: "Hello!!", MyEnum.enum3: "qwerty"]
        let di3 = [MyEnum.enum1: "Goodbye"]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opMyEnumStringDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 2)
        try test(ro[0][MyEnum.enum2] == "Hello!!")
        try test(ro[0][MyEnum.enum3] == "qwerty")
        try test(ro[1].count == 1)
        try test(ro[1][MyEnum.enum1] == "abc")

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0][MyEnum.enum1] == "Goodbye")
        try test(`do`[1].count == 1)
        try test(`do`[1][MyEnum.enum1] == "abc")
        try test(`do`[2].count == 2)
        try test(`do`[2][MyEnum.enum2] == "Hello!!")
        try test(`do`[2][MyEnum.enum3] == "qwerty")
    }

    do {
        let s11 = MyStruct(i: 1, j: 1)
        let s12 = MyStruct(i: 1, j: 2)
        let di1 = [s11: MyEnum.enum1, s12: MyEnum.enum2]

        let s22 = MyStruct(i: 2, j: 2)
        let s23 = MyStruct(i: 2, j: 3)
        let di2 = [s11: MyEnum.enum1, s22: MyEnum.enum3, s23: MyEnum.enum2]

        let di3 = [s23: MyEnum.enum3]

        let dsi1 = [di1, di2]
        let dsi2 = [di3]

        let (ro, `do`) = try await p.opMyStructMyEnumDS(p1: dsi1, p2: dsi2)

        try test(ro.count == 2)
        try test(ro[0].count == 3)
        try test(ro[0][s11] == MyEnum.enum1)
        try test(ro[0][s22] == MyEnum.enum3)
        try test(ro[0][s23] == MyEnum.enum2)
        try test(ro[1].count == 2)
        try test(ro[1][s11] == MyEnum.enum1)
        try test(ro[1][s12] == MyEnum.enum2)

        try test(`do`.count == 3)
        try test(`do`[0].count == 1)
        try test(`do`[0][s23] == MyEnum.enum3)
        try test(`do`[1].count == 2)
        try test(`do`[1][s11] == MyEnum.enum1)
        try test(`do`[1][s12] == MyEnum.enum2)
        try test(`do`[2].count == 3)
        try test(`do`[2][s11] == MyEnum.enum1)
        try test(`do`[2][s22] == MyEnum.enum3)
        try test(`do`[2][s23] == MyEnum.enum2)
    }

    do {
        let si1 = ByteSeq([0x01, 0x11])
        let si2 = ByteSeq([0x12])
        let si3 = ByteSeq([0xF2, 0xF3])

        let sdi1: [UInt8: ByteSeq] = [0x01: si1, 0x22: si2]
        let sdi2: [UInt8: ByteSeq] = [0xF1: si3]

        let (ro, `do`) = try await p.opByteByteSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[0xF1]!.count == 2)
        try test(`do`[0xF1]![0] == 0xF2)
        try test(`do`[0xF1]![1] == 0xF3)

        try test(ro.count == 3)
        try test(ro[0x01]!.count == 2)
        try test(ro[0x01]![0] == 0x01)
        try test(ro[0x01]![1] == 0x11)
        try test(ro[0x22]!.count == 1)
        try test(ro[0x22]![0] == 0x12)
        try test(ro[0xF1]!.count == 2)
        try test(ro[0xF1]![0] == 0xF2)
        try test(ro[0xF1]![1] == 0xF3)
    }

    do {
        let si1 = [true, false]
        let si2 = [false, true, true]

        let sdi1 = [false: si1, true: si2]
        let sdi2 = [false: si1]

        let (ro, `do`) = try await p.opBoolBoolSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[false]!.count == 2)
        try test(`do`[false]![0])
        try test(!`do`[false]![1])
        try test(ro.count == 2)
        try test(ro[false]!.count == 2)
        try test(ro[false]![0])
        try test(!ro[false]![1])
        try test(ro[true]!.count == 3)
        try test(!ro[true]![0])
        try test(ro[true]![1])
        try test(ro[true]![2])
    }

    do {
        let si1: [Int16] = [1, 2, 3]
        let si2: [Int16] = [4, 5]
        let si3: [Int16] = [6, 7]

        let sdi1: [Int16: [Int16]] = [1: si1, 2: si2]
        let sdi2: [Int16: [Int16]] = [4: si3]

        let (ro, `do`) = try await p.opShortShortSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[4]!.count == 2)
        try test(`do`[4]![0] == 6)
        try test(`do`[4]![1] == 7)

        try test(ro.count == 3)
        try test(ro[1]!.count == 3)
        try test(ro[1]![0] == 1)
        try test(ro[1]![1] == 2)
        try test(ro[1]![2] == 3)
        try test(ro[2]!.count == 2)
        try test(ro[2]![0] == 4)
        try test(ro[2]![1] == 5)
        try test(ro[4]!.count == 2)
        try test(ro[4]![0] == 6)
        try test(ro[4]![1] == 7)
    }

    do {
        let si1: [Int32] = [100, 200, 300]
        let si2: [Int32] = [400, 500]
        let si3: [Int32] = [600, 700]

        let sdi1: [Int32: [Int32]] = [100: si1, 200: si2]
        let sdi2: [Int32: [Int32]] = [400: si3]

        let (ro, `do`) = try await p.opIntIntSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[400]!.count == 2)
        try test(`do`[400]![0] == 600)
        try test(`do`[400]![1] == 700)

        try test(ro.count == 3)
        try test(ro[100]!.count == 3)
        try test(ro[100]![0] == 100)
        try test(ro[100]![1] == 200)
        try test(ro[100]![2] == 300)
        try test(ro[200]!.count == 2)
        try test(ro[200]![0] == 400)
        try test(ro[200]![1] == 500)
        try test(ro[400]!.count == 2)
        try test(ro[400]![0] == 600)
        try test(ro[400]![1] == 700)
    }

    do {
        let si1: [Int64] = [999_999_110, 999_999_111, 999_999_110]
        let si2: [Int64] = [999_999_120, 999_999_130]
        let si3: [Int64] = [999_999_110, 999_999_120]

        let sdi1: [Int64: [Int64]] = [999_999_990: si1, 999_999_991: si2]
        let sdi2: [Int64: [Int64]] = [999_999_992: si3]

        let (ro, `do`) = try await p.opLongLongSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[999_999_992]!.count == 2)
        try test(`do`[999_999_992]![0] == 999_999_110)
        try test(`do`[999_999_992]![1] == 999_999_120)
        try test(ro.count == 3)
        try test(ro[999_999_990]!.count == 3)
        try test(ro[999_999_990]![0] == 999_999_110)
        try test(ro[999_999_990]![1] == 999_999_111)
        try test(ro[999_999_990]![2] == 999_999_110)
        try test(ro[999_999_991]!.count == 2)
        try test(ro[999_999_991]![0] == 999_999_120)
        try test(ro[999_999_991]![1] == 999_999_130)
        try test(ro[999_999_992]!.count == 2)
        try test(ro[999_999_992]![0] == 999_999_110)
        try test(ro[999_999_992]![1] == 999_999_120)
    }

    do {
        let si1: [Float] = [-1.1, 123_123.2, 100.0]
        let si2: [Float] = [42.24, -1.61]
        let si3: [Float] = [-3.14, 3.14]

        let sdi1 = ["abc": si1, "ABC": si2]
        let sdi2 = ["aBc": si3]

        let (ro, `do`) = try await p.opStringFloatSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`["aBc"]!.count == 2)
        try test(`do`["aBc"]![0] == -3.14)
        try test(`do`["aBc"]![1] == 3.14)

        try test(ro.count == 3)
        try test(ro["abc"]!.count == 3)
        try test(ro["abc"]![0] == -1.1)
        try test(ro["abc"]![1] == 123_123.2)
        try test(ro["abc"]![2] == 100.0)
        try test(ro["ABC"]!.count == 2)
        try test(ro["ABC"]![0] == 42.24)
        try test(ro["ABC"]![1] == -1.61)
        try test(ro["aBc"]!.count == 2)
        try test(ro["aBc"]![0] == -3.14)
        try test(ro["aBc"]![1] == 3.14)
    }

    do {
        let si1: [Double] = [1.1e10, 1.2e10, 1.3e10]
        let si2: [Double] = [1.4e10, 1.5e10]
        let si3: [Double] = [1.6e10, 1.7e10]

        let sdi1 = ["Hello!!": si1, "Goodbye": si2]
        let sdi2 = ["": si3]

        let (ro, `do`) = try await p.opStringDoubleSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[""]!.count == 2)
        try test(`do`[""]![0] == 1.6e10)
        try test(`do`[""]![1] == 1.7e10)
        try test(ro.count == 3)
        try test(ro["Hello!!"]!.count == 3)
        try test(ro["Hello!!"]![0] == 1.1e10)
        try test(ro["Hello!!"]![1] == 1.2e10)
        try test(ro["Hello!!"]![2] == 1.3e10)
        try test(ro["Goodbye"]!.count == 2)
        try test(ro["Goodbye"]![0] == 1.4e10)
        try test(ro["Goodbye"]![1] == 1.5e10)
        try test(ro[""]!.count == 2)
        try test(ro[""]![0] == 1.6e10)
        try test(ro[""]![1] == 1.7e10)
    }

    do {
        let si1 = ["abc", "de", "fghi"]
        let si2 = ["xyz", "or"]
        let si3 = ["and", "xor"]

        let sdi1 = ["abc": si1, "def": si2]
        let sdi2 = ["ghi": si3]

        let (ro, `do`) = try await p.opStringStringSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`["ghi"]!.count == 2)
        try test(`do`["ghi"]![0] == "and")
        try test(`do`["ghi"]![1] == "xor")

        try test(ro.count == 3)
        try test(ro["abc"]!.count == 3)
        try test(ro["abc"]![0] == "abc")
        try test(ro["abc"]![1] == "de")
        try test(ro["abc"]![2] == "fghi")
        try test(ro["def"]!.count == 2)
        try test(ro["def"]![0] == "xyz")
        try test(ro["def"]![1] == "or")
        try test(ro["ghi"]!.count == 2)
        try test(ro["ghi"]![0] == "and")
        try test(ro["ghi"]![1] == "xor")
    }

    do {
        let si1 = [MyEnum.enum1, MyEnum.enum1, MyEnum.enum2]
        let si2 = [MyEnum.enum1, MyEnum.enum2]
        let si3 = [MyEnum.enum3, MyEnum.enum3]

        let sdi1 = [MyEnum.enum3: si1, MyEnum.enum2: si2]
        let sdi2 = [MyEnum.enum1: si3]

        let (ro, `do`) = try await p.opMyEnumMyEnumSD(p1: sdi1, p2: sdi2)

        try test(`do`.count == 1)
        try test(`do`[MyEnum.enum1]!.count == 2)
        try test(`do`[MyEnum.enum1]![0] == MyEnum.enum3)
        try test(`do`[MyEnum.enum1]![1] == MyEnum.enum3)
        try test(ro.count == 3)
        try test(ro[MyEnum.enum3]!.count == 3)
        try test(ro[MyEnum.enum3]![0] == MyEnum.enum1)
        try test(ro[MyEnum.enum3]![1] == MyEnum.enum1)
        try test(ro[MyEnum.enum3]![2] == MyEnum.enum2)
        try test(ro[MyEnum.enum2]!.count == 2)
        try test(ro[MyEnum.enum2]![0] == MyEnum.enum1)
        try test(ro[MyEnum.enum2]![1] == MyEnum.enum2)
        try test(ro[MyEnum.enum1]!.count == 2)
        try test(ro[MyEnum.enum1]![0] == MyEnum.enum3)
        try test(ro[MyEnum.enum1]![1] == MyEnum.enum3)
    }
}

func testContext(helper: TestHelper, prx p: MyClassPrx) async throws {

    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    do {
        let ctx = ["one": "ONE", "two": "TWO", "three": "THREE"]
        do {
            try test(p.ice_getContext().count == 0)
            let r = try await p.opContext()
            try test(r != ctx)
        }

        do {
            let r = try await p.opContext(context: ctx)
            try test(p.ice_getContext().count == 0)
            try test(r == ctx)
        }

        do {
            let p2 = try await checkedCast(prx: p.ice_context(ctx), type: MyClassPrx.self)!
            try test(p2.ice_getContext() == ctx)
            var r = try await p2.opContext()
            try test(r == ctx)
            r = try await p2.opContext(context: ctx)
            try test(r == ctx)
        }
    }

    // By default, there is no implicit context.
    try test(helper.communicator().getImplicitContext() == nil)

    let conn = try await p.ice_getConnection()
    if conn != nil {
        //
        // Test implicit context propagation
        //
        var initData = Ice.InitializationData()
        let properties = helper.communicator().getProperties().clone()
        properties.setProperty(key: "Ice.ImplicitContext", value: "Shared")
        initData.properties = properties

        let ic = try helper.initialize(initData)

        var ctx = ["one": "ONE", "two": "TWO", "three": "THREE"]

        var p3 = try uncheckedCast(
            prx: ic.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!,
            type: MyClassPrx.self)

        guard let implicitContext = ic.getImplicitContext() else {
            try test(false)
            return // never reached
        }

        implicitContext.setContext(ctx)
        try test(implicitContext.getContext() == ctx)
        try await test(p3.opContext() == ctx)

        try test(implicitContext.containsKey("zero") == false)
        let r = implicitContext.put(key: "zero", value: "ZERO")
        try test(r == "")
        try test(implicitContext.get("zero") == "ZERO")
        implicitContext.put(key: "zero", value: "ZERO")  // can discard return value

        ctx = implicitContext.getContext()
        try await test(p3.opContext() == ctx)

        let prxContext = ["one": "UN", "four": "QUATRE"]

        var combined = prxContext
        for (key, value) in ctx where combined[key] == nil {
            combined[key] = value
        }
        try test(combined["one"] == "UN")

        p3 = uncheckedCast(prx: p3.ice_context(prxContext), type: MyClassPrx.self)

        implicitContext.setContext(Ice.Context())
        try await test(p3.opContext() == prxContext)

        implicitContext.setContext(ctx)
        try await test(p3.opContext() == combined)

        ic.destroy()
    }
}

func testIdempotent(helper: TestHelper, prx p: MyClassPrx) async throws {
    try await p.opIdempotent()
}
