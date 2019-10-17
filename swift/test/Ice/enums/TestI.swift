//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

class TestI: TestIntf {
    func opByte(b1: ByteEnum, current _: Ice.Current) throws -> (returnValue: ByteEnum, b2: ByteEnum) {
        return (b1, b1)
    }

    func opShort(s1: ShortEnum, current _: Ice.Current) throws -> (returnValue: ShortEnum, s2: ShortEnum) {
        return (s1, s1)
    }

    func opInt(i1: IntEnum, current _: Ice.Current) throws -> (returnValue: IntEnum, i2: IntEnum) {
        return (i1, i1)
    }

    func opSimple(s1: SimpleEnum, current _: Ice.Current) throws -> (returnValue: SimpleEnum, s2: SimpleEnum) {
        return (s1, s1)
    }

    func opByteSeq(b1: [ByteEnum], current _: Ice.Current) throws -> (returnValue: [ByteEnum], b2: [ByteEnum]) {
        return (b1, b1)
    }

    func opShortSeq(s1: [ShortEnum], current _: Ice.Current) throws -> (returnValue: [ShortEnum], s2: [ShortEnum]) {
        return (s1, s1)
    }

    func opIntSeq(i1: [IntEnum], current _: Ice.Current) throws -> (returnValue: [IntEnum], i2: [IntEnum]) {
        return (i1, i1)
    }

    func opSimpleSeq(s1: [SimpleEnum], current _: Ice.Current) throws -> (returnValue: [SimpleEnum], s2: [SimpleEnum]) {
        return (s1, s1)
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
