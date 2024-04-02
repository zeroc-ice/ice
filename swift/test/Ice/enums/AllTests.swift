//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func allTests(_ helper: TestHelper) throws -> TestIntfPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()
    let obj = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!
    let proxy = uncheckedCast(prx: obj, type: TestIntfPrx.self)

    output.write("testing enum values... ")

    try test(ByteEnum.benum1.rawValue == 0)
    try test(ByteEnum.benum2.rawValue == 1)
    try test(ByteEnum.benum3.rawValue == ByteConst1)
    try test(ByteEnum.benum4.rawValue == ByteConst1 + 1)
    try test(ByteEnum.benum5.rawValue == ShortConst1)
    try test(ByteEnum.benum6.rawValue == ShortConst1 + 1)
    try test(ByteEnum.benum7.rawValue == IntConst1)
    try test(ByteEnum.benum8.rawValue == IntConst1 + 1)
    try test(ByteEnum.benum9.rawValue == LongConst1)
    try test(ByteEnum.benum10.rawValue == LongConst1 + 1)
    try test(ByteEnum.benum11.rawValue == ByteConst2)

    try test(ShortEnum.senum1.rawValue == 3)
    try test(ShortEnum.senum2.rawValue == 4)
    try test(ShortEnum.senum3.rawValue == ByteConst1)
    try test(ShortEnum.senum4.rawValue == ByteConst1 + 1)
    try test(ShortEnum.senum5.rawValue == ShortConst1)
    try test(ShortEnum.senum6.rawValue == ShortConst1 + 1)
    try test(ShortEnum.senum7.rawValue == IntConst1)
    try test(ShortEnum.senum8.rawValue == IntConst1 + 1)
    try test(ShortEnum.senum9.rawValue == LongConst1)
    try test(ShortEnum.senum10.rawValue == LongConst1 + 1)
    try test(ShortEnum.senum11.rawValue == ShortConst2)

    try test(IntEnum.ienum1.rawValue == 0)
    try test(IntEnum.ienum2.rawValue == 1)
    try test(IntEnum.ienum3.rawValue == ByteConst1)
    try test(IntEnum.ienum4.rawValue == ByteConst1 + 1)
    try test(IntEnum.ienum5.rawValue == ShortConst1)
    try test(IntEnum.ienum6.rawValue == ShortConst1 + 1)
    try test(IntEnum.ienum7.rawValue == IntConst1)
    try test(IntEnum.ienum8.rawValue == IntConst1 + 1)
    try test(IntEnum.ienum9.rawValue == LongConst1)
    try test(IntEnum.ienum10.rawValue == LongConst1 + 1)
    try test(IntEnum.ienum11.rawValue == IntConst2)
    try test(IntEnum.ienum12.rawValue == LongConst2)

    try test(SimpleEnum.red.rawValue == 0)
    try test(SimpleEnum.green.rawValue == 1)
    try test(SimpleEnum.blue.rawValue == 2)

    output.writeLine("ok")

    output.write("testing enum streaming... ")

    let encoding_1_0 = communicator.getProperties().getProperty("Ice.Default.EncodingVersion") == "1.0"

    var ostr = Ice.OutputStream(communicator: communicator)
    ostr.write(ByteEnum.benum11)
    var bytes = ostr.finished()
    try test(bytes.count == 1) // ByteEnum should require one byte

    ostr = Ice.OutputStream(communicator: communicator)
    ostr.write(ShortEnum.senum11)
    bytes = ostr.finished()
    try test(bytes.count == (encoding_1_0 ? 2 : 5))

    ostr = Ice.OutputStream(communicator: communicator)
    ostr.write(IntEnum.ienum11)
    bytes = ostr.finished()
    try test(bytes.count == (encoding_1_0 ? 4 : 5))

    ostr = Ice.OutputStream(communicator: communicator)
    ostr.write(SimpleEnum.blue)
    bytes = ostr.finished()
    try test(bytes.count == 1) // SimpleEnum should require one byte

    output.writeLine("ok")

    output.write("testing enum operations... ")

    try test(proxy.opByte(ByteEnum.benum1) == (ByteEnum.benum1, ByteEnum.benum1))

    try test(proxy.opByte(ByteEnum.benum11) == (ByteEnum.benum11, ByteEnum.benum11))

    try test(proxy.opShort(ShortEnum.senum1) == (ShortEnum.senum1, ShortEnum.senum1))
    try test(proxy.opShort(ShortEnum.senum11) == (ShortEnum.senum11, ShortEnum.senum11))

    try test(proxy.opInt(IntEnum.ienum1) == (IntEnum.ienum1, IntEnum.ienum1))
    try test(proxy.opInt(IntEnum.ienum11) == (IntEnum.ienum11, IntEnum.ienum11))
    try test(proxy.opInt(IntEnum.ienum12) == (IntEnum.ienum12, IntEnum.ienum12))

    try test(proxy.opSimple(SimpleEnum.green) == (SimpleEnum.green, SimpleEnum.green))

    output.writeLine("ok")

    output.write("testing enum sequences operations... ")

    do {
        let b1 = [ByteEnum.benum1,
                  ByteEnum.benum2,
                  ByteEnum.benum3,
                  ByteEnum.benum4,
                  ByteEnum.benum5,
                  ByteEnum.benum6,
                  ByteEnum.benum7,
                  ByteEnum.benum8,
                  ByteEnum.benum9,
                  ByteEnum.benum10,
                  ByteEnum.benum11]
        try test(proxy.opByteSeq(b1) == (b1, b1))
    }

    do {
        let s1 = [ShortEnum.senum1,
                  ShortEnum.senum2,
                  ShortEnum.senum3,
                  ShortEnum.senum4,
                  ShortEnum.senum5,
                  ShortEnum.senum6,
                  ShortEnum.senum7,
                  ShortEnum.senum8,
                  ShortEnum.senum9,
                  ShortEnum.senum10,
                  ShortEnum.senum11]

        try test(proxy.opShortSeq(s1) == (s1, s1))
    }

    do {
        let i1 = [IntEnum.ienum1,
                  IntEnum.ienum2,
                  IntEnum.ienum3,
                  IntEnum.ienum4,
                  IntEnum.ienum5,
                  IntEnum.ienum6,
                  IntEnum.ienum7,
                  IntEnum.ienum8,
                  IntEnum.ienum9,
                  IntEnum.ienum10,
                  IntEnum.ienum11]

        try test(proxy.opIntSeq(i1) == (i1, i1))
    }

    do {
        let s1 = [SimpleEnum.red,
                  SimpleEnum.green,
                  SimpleEnum.blue]
        try test(proxy.opSimpleSeq(s1) == (s1, s1))
    }

    output.writeLine("ok")
    return proxy
}
