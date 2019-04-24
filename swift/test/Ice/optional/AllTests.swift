//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

class TestValueReader: Ice.Value {
    public override func _iceRead(from istr: Ice.InputStream) throws {
        istr.startValue()
        _ = try istr.startSlice()
        try istr.endSlice()
        _ = try istr.endValue(preserve: false)
    }
}

class BValueReader: Ice.Value {
    public override func _iceRead(from istr: Ice.InputStream) throws {
        istr.startValue()
        // ::Test::B
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        // ::Test::A
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        _ = try istr.endValue(preserve: false)
    }
}

class CValueReader: Ice.Value {
    public override func _iceRead(from istr: Ice.InputStream) throws {
        istr.startValue()
        // ::Test::C
        _ = try istr.startSlice()
        try istr.skipSlice()
        // ::Test::B
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        // ::Test::A
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        _ = try istr.endValue(preserve: false)
    }
}

class DValueWriter: Ice.Value {
    public override func _iceWrite(to ostr: Ice.OutputStream) {
        ostr.startValue(data: nil)
        // ::Test::D
        ostr.startSlice(typeId: "::Test::D", compactId: -1, last: false)
        let s = "test"
        ostr.write(s)
        _ = ostr.writeOptional(tag: 1, format: .FSize)
        let o = ["test1", "test2", "test3", "test4"]
        let pos = ostr.startSize()
        ostr.write(o)
        ostr.endSize(position: pos)
        let a = A()
        a.mc = 18
        _ = ostr.writeOptional(tag: 1000, format: .Class)
        ostr.write(a)
        ostr.endSlice()
        // ::Test::B
        ostr.startSlice(typeId: B.ice_staticId(), compactId: -1, last: false)
        let v: Int32 = 14
        ostr.write(v)
        ostr.endSlice()
        // ::Test::A
        ostr.startSlice(typeId: A.ice_staticId(), compactId: -1, last: true)
        ostr.write(v)
        ostr.endSlice()
        ostr.endValue()
    }
}

class DValueReader: Ice.Value {

    var a: A?
    var helper: TestHelper?

    required init() {
        super.init()
    }

    init(helper: TestHelper) {
        self.helper = helper
    }

    public override func _iceRead(from istr: Ice.InputStream) throws {
        istr.startValue()
        // ::Test::D
        _ = try istr.startSlice()
        let s: String = try istr.read()
        try helper!.test(s == "test")
        try helper!.test(istr.readOptional(tag: 1, expectedFormat: .FSize))
        try istr.skip(4)
        let o: [String] = try istr.read()
        try helper!.test(o.count == 4 &&
                           o[0] == "test1" &&
                           o[1] == "test2" &&
                           o[2] == "test3" &&
                           o[3] == "test4")
        try istr.read(tag: 1000, value: A.self) { self.a = $0}
        try istr.endSlice()
        // ::Test::B
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        // ::Test::A
        _ = try istr.startSlice()
        _ = try istr.read() as Int32
        try istr.endSlice()
        _ = try istr.endValue(preserve: false)
    }

    func check() throws {
        try helper!.test(a!.mc == 18)
    }
}

class FValueReader: Ice.Value {

    public required init() {
        _f = F()
        super.init()
    }
    public override func _iceRead(from istr: Ice.InputStream) throws {
        _f = F()
        _ = istr.startValue()
        _ = try istr.startSlice()
        // Don't read af on purpose
        //in.read(1, _f.af);
        try istr.endSlice()
        _ = try istr.startSlice()
        try istr.read(A.self) { self._f.ae = $0 }
        try istr.endSlice()
        _ = try istr.endValue(preserve: false)
    }

    public func getF() -> F? {
        return _f
    }

    var _f: F
}

class FactoryI {

    init(helper: TestHelper) {
        _enabled = false
        _helper = helper
    }

    func create(_ typeId: String) -> Ice.Value? {
        guard _enabled else {
            return nil
        }
        switch typeId {
        case OneOptional.ice_staticId():
            return TestValueReader()
        case MultiOptional.ice_staticId():
            return TestValueReader()
        case B.ice_staticId():
            return BValueReader()
        case C.ice_staticId():
            return CValueReader()
        case "::Test::D":
            return DValueReader(helper: _helper)
        case "::Test::F":
            return FValueReader()
        default:
            return nil
        }
    }

    func setEnabled(enabled: Bool) {
        _enabled = enabled
    }

    var _enabled: Bool
    let _helper: TestHelper
}

func allTests(helper: TestHelper) {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let factory = FactoryI(helper: helper)
    try communicator.getValueFactoryManager().add(factory: { id in factory.create(id) }, id: "")

    let output = helper.getWriter()
    output.write("testing stringToProxy... ")
    let ref = "initial:\(helper.getTestEndpoint(num: 0))"
    let base = try communicator.stringToProxy(ref)!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let initial = try checkedCast(prx: base, type: InitialPrx.self)!
    try test(initial == base)
    output.writeLine("ok")

    output.write("testing optional data members... ")

    let oo1 = OneOptional()
    try test(oo1.a == nil)
    oo1.a = 15
    try test(oo1.a! == 15)

    let oo2 = OneOptional(a: 16)
    try test(oo2.a! == 16)

    let mo1 = MultiOptional()
    mo1.a = 15
    mo1.b = true
    mo1.c = 19
    mo1.d = 78
    mo1.e = 99
    mo1.f = 5.5
    mo1.g = 1.0
    mo1.h = "test"
    mo1.i = .MyEnumMember
    mo1.j = try communicator.stringToProxy("test")
    mo1.k = mo1
    mo1.bs = [5]
    mo1.ss = ["test", "test2"]
    mo1.iid = [4: 3]
    mo1.sid = ["test": 10]
    var fs = FixedStruct()
    fs.m = 78
    mo1.fs = fs
    var vs = VarStruct()
    vs.m = "hello"
    mo1.vs = vs

    mo1.shs = [1]
    mo1.es = [.MyEnumMember, .MyEnumMember]
    mo1.fss = [fs]
    mo1.vss = [vs]
    mo1.oos = [oo1]
    mo1.oops = [try communicator.stringToProxy("test")]

    mo1.ied = [4: .MyEnumMember]
    mo1.ifsd = [4: fs]
    mo1.ivsd = [5: vs]
    mo1.iood = [5: OneOptional(a: 15)]
    mo1.ioopd = [5: try communicator.stringToProxy("test")]

    mo1.bos = [false, true, false]

    try test(mo1.a! == 15)
    try test(mo1.b!)
    try test(mo1.c! == 19)
    try test(mo1.d! == 78)
    try test(mo1.e! == 99)
    try test(mo1.f! == 5.5)
    try test(mo1.g! == 1.0)
    try test(mo1.h! == "test")
    try test(mo1.i! == .MyEnumMember)
    try test(mo1.j! == communicator.stringToProxy("test"))
    try test(mo1.k! === mo1)
    try test(mo1.bs! == [5])
    try test(mo1.ss! == ["test", "test2"])
    try test(mo1.iid![4]! == 3)
    try test(mo1.sid!["test"]! == 10)
    try test(mo1.fs! == FixedStruct(m: 78))
    try test(mo1.vs! == VarStruct(m: "hello"))

    try test(mo1.shs![0] == 1)
    try test(mo1.es![0] == .MyEnumMember &&
               mo1.es![1] == .MyEnumMember)

    try test(mo1.fss![0] == FixedStruct(m: 78))
    try test(mo1.vss![0] == VarStruct(m: "hello"))
    try test(mo1.oos![0] === oo1)
    try test(mo1.oops![0] == communicator.stringToProxy("test"))

    try test(mo1.ied![4] == .MyEnumMember)
    try test(mo1.ifsd![4] == FixedStruct(m: 78))
    try test(mo1.ivsd![5] == VarStruct(m: "hello"))
    try test(mo1.iood![5]!!.a! == 15)
    try test(mo1.ioopd![5]! == communicator.stringToProxy("test"))

    try test(mo1.bos == [false, true, false])

    output.writeLine("ok")

    output.write("testing marshaling... ")

    if let oo4 = try initial.pingPong(OneOptional()) as? OneOptional {
        try test(oo4.a == nil)
    } else {
        try test(false)
    }

    if let oo5 = try initial.pingPong(oo1) as? OneOptional {
        try test(oo1.a == oo5.a)
    } else {
        try test(false)
    }

    if let mo4 = try initial.pingPong(MultiOptional()) as? MultiOptional {
        try test(mo4.a == nil)
        try test(mo4.b == nil)
        try test(mo4.c == nil)
        try test(mo4.d == nil)
        try test(mo4.e == nil)
        try test(mo4.f == nil)
        try test(mo4.g == nil)
        try test(mo4.h == nil)
        try test(mo4.i == nil)
        try test(mo4.j == nil)
        try test(mo4.k == nil)
        try test(mo4.bs == nil)
        try test(mo4.ss == nil)
        try test(mo4.iid == nil)
        try test(mo4.sid == nil)
        try test(mo4.fs == nil)
        try test(mo4.vs == nil)

        try test(mo4.shs == nil)
        try test(mo4.es == nil)
        try test(mo4.fss == nil)
        try test(mo4.vss == nil)
        try test(mo4.oos == nil)
        try test(mo4.oops == nil)

        try test(mo4.ied == nil)
        try test(mo4.ifsd == nil)
        try test(mo4.ivsd == nil)
        try test(mo4.iood == nil)
        try test(mo4.ioopd == nil)

        try test(mo4.bos == nil)

        try test(mo4.ser == nil)
    } else {
        try test(false)
    }

    let mo6 = MultiOptional()
    let mo8 = MultiOptional()

    if let mo5 = try initial.pingPong(mo1) as? MultiOptional {
        try test(mo5.a == mo1.a)
        try test(mo5.b == mo1.b)
        try test(mo5.c == mo1.c)
        try test(mo5.d == mo1.d)
        try test(mo5.e == mo1.e)
        try test(mo5.f == mo1.f)
        try test(mo5.g == mo1.g)
        try test(mo5.h == mo1.h)
        try test(mo5.i == mo1.i)
        try test(mo5.j == mo1.j)
        try test(mo5.k === mo5)
        try test(mo5.bs == mo1.bs)
        try test(mo5.ss == mo1.ss)
        try test(mo5.iid![4] == 3)
        try test(mo5.sid!["test"] == 10)
        try test(mo5.fs == mo1.fs)
        try test(mo5.vs == mo1.vs)
        try test(mo5.shs == mo1.shs)
        try test(mo5.es![0] == .MyEnumMember &&
                   mo1.es![1] == .MyEnumMember)
        try test(mo5.fss![0] == FixedStruct(m: 78))
        try test(mo5.vss![0] == VarStruct(m: "hello"))
        try test(mo5.oos![0]!.a! == 15)
        try test(mo5.oops![0] == communicator.stringToProxy("test"))

        try test(mo5.ied![4] == .MyEnumMember)
        try test(mo5.ifsd![4] == FixedStruct(m: 78))
        try test(mo5.ivsd![5] == VarStruct(m: "hello"))
        try test(mo5.iood![5]!!.a == 15)
        try test(mo5.ioopd![5]! == communicator.stringToProxy("test"))

        try test(mo5.bos == [false, true, false])

        // Clear the first half of the optional members
        mo6.b = mo5.b
        mo6.d = mo5.d
        mo6.f = mo5.f
        mo6.h = mo5.h
        mo6.j = mo5.j
        mo6.bs = mo5.bs
        mo6.iid = mo5.iid
        mo6.fs = mo5.fs
        mo6.shs = mo5.shs
        mo6.fss = mo5.fss
        mo6.oos = mo5.oos
        mo6.ifsd = mo5.ifsd
        mo6.iood = mo5.iood
        mo6.bos = mo5.bos

        // Clear the second half of the optional members
        mo8.a = mo5.a
        mo8.c = mo5.c
        mo8.e = mo5.e
        mo8.g = mo5.g
        mo8.i = mo5.i
        mo8.k = mo8
        mo8.ss = mo5.ss
        mo8.sid = mo5.sid
        mo8.vs = mo5.vs

        mo8.es = mo5.es
        mo8.vss = mo5.vss
        mo8.oops = mo5.oops

        mo8.ied = mo5.ied
        mo8.ivsd = mo5.ivsd
        mo8.ioopd = mo5.ioopd
    } else {
        try test(false)
    }

    if let mo7 = try initial.pingPong(mo6) as? MultiOptional {
        try test(mo7.a == nil)
        try test(mo7.b == mo1.b)
        try test(mo7.c == nil)
        try test(mo7.d == mo1.d)
        try test(mo7.e == nil)
        try test(mo7.f == mo1.f)
        try test(mo7.g == nil)
        try test(mo7.h == mo1.h)
        try test(mo7.i == nil)
        try test(mo7.j == mo1.j)
        try test(mo7.k == nil)
        try test(mo7.bs == mo1.bs)
        try test(mo7.ss == nil)
        try test(mo7.iid![4] == 3)
        try test(mo7.sid == nil)
        try test(mo7.fs == mo1.fs)
        try test(mo7.vs == nil)

        try test(mo7.shs == mo1.shs)
        try test(mo7.es == nil)
        try test(mo7.fss![0] == FixedStruct(m: 78))
        try test(mo7.vss == nil)
        try test(mo7.oos![0]!.a == 15)
        try test(mo7.oops == nil)

        try test(mo7.ied == nil)
        try test(mo7.ifsd![4] == FixedStruct(m: 78))
        try test(mo7.ivsd == nil)
        try test(mo7.iood![5]!!.a == 15)
        try test(mo7.ioopd == nil)

        try test(mo7.bos == [false, true, false])
        try test(mo7.ser == nil)
    } else {
        try test(false)
    }

    if let mo9 = try initial.pingPong(mo8) as? MultiOptional {
        try test(mo9.a == mo1.a)
        try test(mo9.b == nil)
        try test(mo9.c == mo1.c)
        try test(mo9.d == nil)
        try test(mo9.e == mo1.e)
        try test(mo9.f == nil)
        try test(mo9.g == mo1.g)
        try test(mo9.h == nil)
        try test(mo9.i == mo1.i)
        try test(mo9.j == nil)
        try test(mo9.k === mo9)
        try test(mo9.bs == nil)
        try test(mo9.ss == mo1.ss)
        try test(mo9.iid == nil)
        try test(mo9.sid!["test"] == 10)
        try test(mo9.fs == nil)
        try test(mo9.vs == mo1.vs)

        try test(mo9.shs == nil)
        try test(mo9.es![0] == .MyEnumMember &&
                   mo9.es![1] == .MyEnumMember)
        try test(mo9.fss == nil)
        try test(mo9.vss![0] == VarStruct(m: "hello"))
        try test(mo9.oos == nil)
        try test(mo9.oops![0] == communicator.stringToProxy("test"))

        try test(mo9.ied![4] == .MyEnumMember)
        try test(mo9.ifsd == nil)
        try test(mo9.ivsd![5] == VarStruct(m: "hello"))
        try test(mo9.iood == nil)
        try test(mo9.ioopd![5]! == communicator.stringToProxy("test"))

        try test(mo9.bos == nil)
    } else {
        try test(false)
    }

    do {
        let owc1 = OptionalWithCustom()
        owc1.l = [SmallStruct(m: 5), SmallStruct(m: 6), SmallStruct(m: 7)]
        owc1.s = ClassVarStruct(a: 5)
        if let owc2 = try initial.pingPong(owc1) as? OptionalWithCustom {
            try test(owc2.l != nil)
            try test(owc1.l == owc2.l)
            try test(owc2.s != nil)
            try test(owc2.s!.a == 5)
        } else {
            try test(false)
        }
    }

    //
    // Send a request using blobjects. Upon receival, we don't read
    // any of the optional members. This ensures the optional members
    // are skipped even if the receiver knows nothing about them.
    //
    factory.setEnabled(enabled: true)
    do {
        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(oo1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "pingPong",
                                            mode: Ice.OperationMode.Normal,
                                            inEncaps: inEncaps)
        try test(result.ok)

        let istr = InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()

        var v: Ice.Value?
        try istr.read { v = $0  }
        try istr.endEncapsulation()
        try test(v != nil && v is TestValueReader)
    }

    do {
        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(mo1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "pingPong",
                                            mode: .Normal,
                                            inEncaps: inEncaps)
        try test(result.ok)
        let istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var v: Ice.Value?
        try istr.read { v = $0 }
        try istr.endEncapsulation()
        try test(v != nil && v is TestValueReader)
    }
    factory.setEnabled(enabled: false)

    //
    // Use the 1.0 encoding with operations whose only class parameters are optional.
    //
    do {
        var oo: OneOptional? = OneOptional(a: 53)
        try initial.sendOptionalClass(req: true, o: oo)
        let initial2 = initial.ice_encodingVersion(Ice.Encoding_1_0)
        try initial2.sendOptionalClass(req: true, o: oo)

        oo = try initial.returnOptionalClass(true)
        try test(oo != nil)
        oo = try initial2.returnOptionalClass(true)
        try test(oo == nil)

        let recursive1 = [Recursive()]
        let recursive2 = [Recursive()]
        recursive1[0].value = recursive2
        let outer = Recursive()
        outer.value = recursive1
        _ = try initial.pingPong(outer)

        var g: G! = G()
        g.gg1Opt = G1(a: "gg1Opt")
        g.gg2 = G2(a: 10)
        g.gg2Opt = G2(a: 20)
        g.gg1 = G1(a: "gg1")
        g = try initial.opG(g)
        try test("gg1Opt" == g.gg1Opt!.a)
        try test(10 == g.gg2!.a)
        try test(20 == g.gg2Opt!.a)
        try test("gg1" == g.gg1!.a)

        try initial.opVoid()

        let ostr = OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        _ = ostr.writeOptional(tag: 1, format: .F4)
        ostr.write(Int32(15))
        _ = ostr.writeOptional(tag: 1, format: .VSize)
        ostr.write("test")
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opVoid", mode: .Normal, inEncaps: inEncaps)
        try test(result.ok)
    }
    output.writeLine("ok")

    output.write("testing marshaling of large containers with fixed size elements... ")
    do {
        var mc = MultiOptional()

        mc.bs = Ice.ByteSeq(repeating: 0, count: 1000)
        mc.shs = Ice.ShortSeq(repeating: 0, count: 300)
        mc.fss = FixedStructSeq(repeating: FixedStruct(), count: 300)

        mc.ifsd = IntFixedStructDict()
        for i: Int32 in 0..<300 {
            mc.ifsd![i] = FixedStruct()
        }

        mc = try initial.pingPong(mc) as! MultiOptional
        try test(mc.bs?.count == 1000)
        try test(mc.shs?.count == 300)
        try test(mc.fss?.count == 300)
        try test(mc.ifsd?.count == 300)

        factory.setEnabled(enabled: true)
        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(mc)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "pingPong", mode: .Normal, inEncaps: inEncaps)
        try test(result.ok)
        let istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var v: Value?
        try istr.read { v = $0 }
        try istr.endEncapsulation()
        try test(v != nil && v is TestValueReader)
        factory.setEnabled(enabled: false)
    }
    output.writeLine("ok")

    output.write("testing tag marshaling... ")
    do {
        let b = B()
        var b2 = try initial.pingPong(b) as! B
        try test(b2.ma == nil)
        try test(b2.mb == nil)
        try test(b2.mc == nil)

        b.ma = 10
        b.mb = 11
        b.mc = 12
        b.md = 13

        b2 = try initial.pingPong(b) as! B
        try test(b2.ma! == 10)
        try test(b2.mb! == 11)
        try test(b2.mc! == 12)
        try test(b2.md! == 13)

        factory.setEnabled(enabled: true)
        let ostr = Ice.OutputStream(communicator: communicator)
        _ = ostr.startEncapsulation()
        ostr.write(b)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "pingPong", mode: .Normal, inEncaps: inEncaps)
        try test(result.ok)
        let istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var v: Value?
        try istr.read { v = $0 }
        try istr.endEncapsulation()
        try test(v != nil)
        factory.setEnabled(enabled: false)
    }
    output.writeLine("ok")

    output.write("testing marshalling of objects with optional objects...")
    do {
        let f = F()

        f.af = A()
        f.ae = f.af

        var rf = try initial.pingPong(f) as! F
        try test(rf.ae === rf.af)

        factory.setEnabled(enabled: true)
        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(f)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        var v: Value?
        try istr.read { v = $0 }
        try istr.endEncapsulation()
        factory.setEnabled(enabled: false)
        rf = (v as! FValueReader).getF()!
        try test(rf.ae != nil && rf.af == nil)
    }
    output.writeLine("ok")

    output.write("testing optional with default values... ")
    do {
        var wd = try initial.pingPong(WD()) as! WD
        try test(wd.a == 5)
        try test(wd.s == "test")
        wd.a = nil
        wd.s = nil
        wd = try initial.pingPong(wd) as! WD
        try test(wd.a == nil)
        try test(wd.s == nil)
    }
    output.writeLine("ok")

    if communicator.getProperties().getPropertyAsInt("Ice.Default.SlicedFormat") > 0 {
        output.write("testing marshaling with unknown class slices... ")
        do {
            let c = C()
            c.ss = "test"
            c.ms = "testms"
            var ostr = Ice.OutputStream(communicator: communicator)
            ostr.startEncapsulation()
            ostr.write(c)
            ostr.endEncapsulation()
            var inEncaps = ostr.finished()
            factory.setEnabled(enabled: true)
            var result = try initial.ice_invoke(operation: "pingPong", mode: .Normal, inEncaps: inEncaps)
            try test(result.ok)
            var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
            _ = try istr.startEncapsulation()
            var v: Ice.Value?
            try istr.read { v = $0 }
            try istr.endEncapsulation()
            try test(v != nil && v is CValueReader)
            factory.setEnabled(enabled: false)

            factory.setEnabled(enabled: true)
            ostr = Ice.OutputStream(communicator: communicator)
            ostr.startEncapsulation()
            let d = DValueWriter()
            ostr.write(d)
            ostr.endEncapsulation()
            inEncaps = ostr.finished()
            result = try initial.ice_invoke(operation: "pingPong", mode: .Normal, inEncaps: inEncaps)
            try test(result.ok)
            istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
            _ = try istr.startEncapsulation()
            v = nil
            try istr.read { v = $0 }
            try istr.endEncapsulation()
            try test(v != nil && v is DValueReader)
            try (v as! DValueReader).check()
            factory.setEnabled(enabled: false)
        }
        output.writeLine("ok")

        output.write("testing optionals with unknown classes...")
        do {
            let a = A()

            let ostr = Ice.OutputStream(communicator: communicator)
            ostr.startEncapsulation()
            ostr.write(a)
            _ = ostr.writeOptional(tag: 1, format: .Class)
            ostr.write(DValueWriter())
            ostr.endEncapsulation()
            let inEncaps = ostr.finished()
            let result = try initial.ice_invoke(operation: "opClassAndUnknownOptional",
                                                mode: .Normal,
                                                inEncaps: inEncaps)
            try test(result.ok)

            let istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
            _ = try istr.startEncapsulation()
            try istr.endEncapsulation()
        }
        output.writeLine("ok")
    }

    output.write("testing optional parameters... ")
    do {
        var p1: UInt8?
        var p2: UInt8?
        var p3: UInt8?

        (p2, p3) = try initial.opByte(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opByte(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 56
        (p2, p3) = try initial.opByte(p1)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opByteAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opByte(56)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opByteAsync(56)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opByte(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opByte", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F1))
        try test(istr.read() as UInt8 == 56)
        try test(istr.readOptional(tag: 3, expectedFormat: .F1))
        try test(istr.read() as UInt8 == 56)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Bool?
        var p3: Bool?
        var p2: Bool?

        (p2, p3) = try initial.opBool(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opBool(nil)
        try test(p2 == nil && p3 == nil)

        p1 = true
        (p2, p3) = try initial.opBool(p1)
        try test(p2 == true && p3 == true)

        try Promise<Void> { seal in
            firstly {
                initial.opBoolAsync(p1)
            }.done { p2, p3 in
                try test(p2 == true && p3 == true)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opBool(true)
        try test(p2 == true && p3 == true)

        try Promise<Void> { seal in
            firstly {
                initial.opBoolAsync(true)
            }.done { p2, p3 in
                try test(p2 == true && p3 == true)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opBool(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opBool", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F1))
        try test(istr.read() as Bool == true)
        try test(istr.readOptional(tag: 3, expectedFormat: .F1))
        try test(istr.read() as Bool == true)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Int16?
        var p2: Int16?
        var p3: Int16?
        (p2, p3) = try initial.opShort(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opShort(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 56
        (p2, p3) = try initial.opShort(p1)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opShortAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opShort(p1)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opShortAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opShort(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opShort", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F2))
        try test(istr.read() as Int16 == 56)
        try test(istr.readOptional(tag: 3, expectedFormat: .F2))
        try test(istr.read() as Int16 == 56)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Int32?
        var p2: Int32?
        var p3: Int32?
        (p2, p3) = try initial.opInt(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opInt(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 56
        (p2, p3) = try initial.opInt(p1)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opIntAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opInt(56)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opIntAsync(56)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opInt(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opInt", mode: Ice.OperationMode.Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F4))
        try test(istr.read() as Int32 == 56)
        try test(istr.readOptional(tag: 3, expectedFormat: .F4))
        try test(istr.read() as Int32 == 56)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Int64?
        var p2: Int64?
        var p3: Int64?
        (p2, p3) = try initial.opLong(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opLong(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 56
        (p2, p3) = try initial.opLong(p1)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opLongAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opLong(56)
        try test(p2 == 56 && p3 == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opLongAsync(56)
            }.done { p2, p3 in
                try test(p2 == 56 && p3 == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opLong(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 1, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opLong", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 2, expectedFormat: .F8))
        try test(istr.read() as Int64 == 56)
        try test(istr.readOptional(tag: 3, expectedFormat: .F8))
        try test(istr.read() as Int64 == 56)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Float?
        var p2: Float?
        var p3: Float?
        (p2, p3) = try initial.opFloat(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opFloat(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 1.0
        (p2, p3) = try initial.opFloat(p1)
        try test(p2 == 1.0 && p3 == 1.0)

        try Promise<Void> { seal in
            firstly {
                initial.opFloatAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 1.0 && p3 == 1.0)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFloat(1.0)
        try test(p2 == 1.0 && p3 == 1.0)

        try Promise<Void> { seal in
            firstly {
                initial.opFloatAsync(1.0)
            }.done { p2, p3 in
                try test(p2 == 1.0 && p3 == 1.0)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFloat(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opFloat", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F4))
        try test(istr.read() as Float == 1.0)
        try test(istr.readOptional(tag: 3, expectedFormat: .F4))
        try test(istr.read() as Float == 1.0)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Double?
        var p2: Double?
        var p3: Double?
        (p2, p3) = try initial.opDouble(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opDouble(nil)
        try test(p2 == nil && p3 == nil)

        p1 = 1.0
        (p2, p3) = try initial.opDouble(p1)
        try test(p2 == 1.0 && p3 == 1.0)

        try Promise<Void> { seal in
            firstly {
                initial.opDoubleAsync(p1)
            }.done { p2, p3 in
                try test(p2 == 1.0 && p3 == 1.0)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opDouble(1.0)
        try test(p2 == 1.0 && p3 == 1.0)

        try Promise<Void> { seal in
            firstly {
                initial.opDoubleAsync(1.0)
            }.done { p2, p3 in
                try test(p2 == 1.0 && p3 == 1.0)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opDouble(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opDouble", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .F8))
        try test(istr.read() as Double == 1.0)
        try test(istr.readOptional(tag: 3, expectedFormat: .F8))
        try test(istr.read() as Double == 1.0)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: String?
        var p2: String?
        var p3: String?
        (p2, p3) = try initial.opString(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opString(nil)
        try test(p2 == nil && p3 == nil)

        p1 = "test"
        (p2, p3) = try initial.opString(p1)
        try test(p2 == "test" && p3 == "test")

        try Promise<Void> { seal in
            firstly {
                initial.opStringAsync(p1)
            }.done { p2, p3 in
                try test(p2 == "test" && p3 == "test")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opString(p1)
        try test(p2 == "test" && p3 == "test")

        try Promise<Void> { seal in
            firstly {
                initial.opStringAsync(p1)
            }.done { p2, p3 in
                try test(p2 == "test" && p3 == "test")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opString(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opString",
                                            mode: .Normal,
                                            inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .VSize))
        try test(istr.read() as String == "test")
        try test(istr.readOptional(tag: 3, expectedFormat: .VSize))
        try test(istr.read() as String == "test")
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: MyEnum?
        var p2: MyEnum?
        var p3: MyEnum?
        (p2, p3) = try initial.opMyEnum(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opMyEnum(nil)
        try test(p2 == nil && p3 == nil)

        p1 = .MyEnumMember
        (p2, p3) = try initial.opMyEnum(p1)
        try test(p2 == .MyEnumMember && p3 == .MyEnumMember)

        try Promise<Void> { seal in
            firstly {
                initial.opMyEnumAsync(p1)
            }.done {p2, p3 in
                try test(p2 == .MyEnumMember && p3 == .MyEnumMember)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opMyEnum(p1)
        try test(p2 == .MyEnumMember && p3 == .MyEnumMember)

        try Promise<Void> { seal in
            firstly {
                initial.opMyEnumAsync(.MyEnumMember)
            }.done { p2, p3 in
                try test(p2 == .MyEnumMember && p3 == .MyEnumMember)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opMyEnum(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opMyEnum", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .Size))
        try test(istr.read() as MyEnum == .MyEnumMember)
        try test(istr.readOptional(tag: 3, expectedFormat: .Size))
        try test(istr.read() as MyEnum == .MyEnumMember)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: SmallStruct?
        var p2: SmallStruct?
        var p3: SmallStruct?
        (p2, p3) = try initial.opSmallStruct(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opSmallStruct(nil)
        try test(p2 == nil && p3 == nil)

        p1 = SmallStruct(m: 56)
        (p2, p3) = try initial.opSmallStruct(p1)
        try test(p2!.m == 56 && p3!.m == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opSmallStructAsync(p1)
            }.done { p2, p3 in
                try test(p2!.m == 56 && p3!.m == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opSmallStruct(SmallStruct(m: 56))
        try test(p2!.m == 56 && p3!.m == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opSmallStructAsync(SmallStruct(m: 56))
            }.done { p2, p3 in
                try test(p2!.m == 56 && p3!.m == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opSmallStruct(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opSmallStruct", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var s: SmallStruct = try istr.read(tag: 1)!
        try test(s.m == 56)
        s = try istr.read(tag: 3)!
        try test(s.m == 56)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: FixedStruct?
        var p2: FixedStruct?
        var p3: FixedStruct?

        (p2, p3) = try initial.opFixedStruct(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3)  = try initial.opFixedStruct(nil)
        try test(p2 == nil && p3 == nil)

        p1 = FixedStruct(m: 56)
        (p2, p3) = try initial.opFixedStruct(p1)
        try test(p2!.m == 56 && p3!.m == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opFixedStructAsync(p1)
            }.done { p2, p3 in
                try test(p2!.m == 56 && p3!.m == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFixedStruct(FixedStruct(m: 56))
        try test(p2!.m == 56 && p3!.m == 56)

        try Promise<Void> { seal in
            firstly {
                initial.opFixedStructAsync(FixedStruct(m: 56))
            }.done { p2, p3 in
                try test(p2!.m == 56 && p3!.m == 56)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFixedStruct(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opFixedStruct", mode: .Normal, inEncaps: inEncaps)

        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var s: FixedStruct = try istr.read(tag: 1)!
        try test(s.m == 56)
        s = try istr.read(tag: 3)!
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()

    }

    do {
        var p1: VarStruct?
        var p2: VarStruct?
        var p3: VarStruct?

        (p2, p3) = try initial.opVarStruct(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opVarStruct(nil)
        try test(p2 == nil && p3 == nil)

        p1 = VarStruct(m: "test")
        (p2, p3) = try initial.opVarStruct(p1)
        try test(p2!.m == "test" && p3!.m == "test")

        // Test null struct
        (p2, p3) = try initial.opVarStruct(nil)
        try test(p2 == nil && p3 == nil)

        try Promise<Void> { seal in
            firstly {
                initial.opVarStructAsync(p1)
            }.done { p2, p3 in
                try test(p2!.m == "test" && p3!.m == "test")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opVarStruct(VarStruct(m: "test"))
        try test(p2!.m == "test" && p3!.m == "test")

        try Promise<Void> { seal in
            firstly {
                initial.opVarStructAsync(VarStruct(m: "test"))
            }.done { p2, p3 in
                try test(p2!.m == "test" && p3!.m == "test")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opVarStruct(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opVarStruct", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        var v: VarStruct = try istr.read(tag: 1)!
        try test(v.m == "test")
        v = try istr.read(tag: 3)!
        try test(v.m == "test")
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: OneOptional?
        var p2: OneOptional?
        var p3: OneOptional?
        (p2, p3) = try initial.opOneOptional(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opOneOptional(nil)
        try test(p2 == nil && p3 == nil)

        p1 = OneOptional(a: 58)
        (p2, p3) = try initial.opOneOptional(p1)
        try test(p2!.a! == 58 && p3!.a! == 58)

        try Promise<Void> { seal in
            firstly {
                initial.opOneOptionalAsync(p1)
            }.done { p2, p3 in
                try test(p2!.a! == 58 && p3!.a! == 58)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opOneOptional(OneOptional(a: 58))
        try test(p2!.a! == 58 && p3!.a! == 58)

        try Promise<Void> { seal in
            firstly {
                initial.opOneOptionalAsync(OneOptional(a: 58))
            }.done { p2, p3 in
                try test(p2!.a! == 58 && p3!.a! == 58)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opOneOptional(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opOneOptional", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.readOptional(tag: 1, expectedFormat: .Class))
        var v1: Ice.Value?
        try istr.read { v1 = $0 }
        try test(istr.readOptional(tag: 3, expectedFormat: .Class))
        var v2: Ice.Value?
        try istr.read { v2 = $0 }
        try istr.endEncapsulation()
        try test((v1 as! OneOptional).a! == 58 && (v2 as! OneOptional).a == 58)

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: Ice.ObjectPrx?
        var p2: Ice.ObjectPrx?
        var p3: Ice.ObjectPrx?
        (p2, p3) = try initial.opOneOptionalProxy(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opOneOptionalProxy(nil)
        try test(p2 == nil && p3 == nil)

        p1 = try communicator.stringToProxy("test")
        (p2, p3) = try initial.opOneOptionalProxy(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opOneOptionalProxyAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opOneOptionalProxy(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        _ = ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opOneOptionalProxy", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        p2 = try istr.read(tag: 1)
        try test(p2 == p1)
        p3 = try istr.read(tag: 3)
        try test(p3 == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [UInt8]?
        var p2: [UInt8]?
        var p3: [UInt8]?

        (p2, p3) = try initial.opByteSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opByteSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opByteSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [UInt8](repeating: 56, count: 100)
        (p2, p3) = try initial.opByteSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opByteSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opByteSeq([UInt8](repeating: 56, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opByteSeqAsync([UInt8](repeating: 56, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opByteSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opByteSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Bool]?
        var p2: [Bool]?
        var p3: [Bool]?

        (p2, p3) = try initial.opBoolSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opBoolSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opBoolSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Bool](repeating: true, count: 100)
        (p2, p3) = try initial.opBoolSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opBoolSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opBoolSeq([Bool](repeating: true, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opBoolSeqAsync([Bool](repeating: true, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opBoolSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opBoolSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Int16]?
        var p2: [Int16]?
        var p3: [Int16]?

        (p2, p3) = try initial.opShortSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opShortSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opShortSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Int16](repeating: 56, count: 100)
        (p2, p3) = try initial.opShortSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opShortSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opShortSeq([Int16](repeating: 56, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opShortSeqAsync([Int16](repeating: 56, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opShortSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opShortSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Int32]?
        var p2: [Int32]?
        var p3: [Int32]?

        (p2, p3) = try initial.opIntSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opIntSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opIntSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Int32](repeating: 56, count: 100)
        (p2, p3) = try initial.opIntSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opIntSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntSeq([Int32](repeating: 56, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opIntSeqAsync([Int32](repeating: 56, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opIntSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Int64]?
        var p2: [Int64]?
        var p3: [Int64]?

        (p2, p3) = try initial.opLongSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opLongSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opLongSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Int64](repeating: 56, count: 100)
        (p2, p3) = try initial.opLongSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opLongSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opLongSeq([Int64](repeating: 56, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opLongSeqAsync([Int64](repeating: 56, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opLongSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opLongSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Float]?
        var p2: [Float]?
        var p3: [Float]?

        (p2, p3) = try initial.opFloatSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opFloatSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opFloatSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Float](repeating: 1.0, count: 100)
        (p2, p3) = try initial.opFloatSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opFloatSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFloatSeq([Float](repeating: 1.0, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opFloatSeqAsync([Float](repeating: 1.0, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFloatSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opFloatSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Double]?
        var p2: [Double]?
        var p3: [Double]?

        (p2, p3) = try initial.opDoubleSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opDoubleSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opDoubleSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [Double](repeating: 1.0, count: 100)
        (p2, p3) = try initial.opDoubleSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opDoubleSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opDoubleSeq([Double](repeating: 1.0, count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opDoubleSeqAsync([Double](repeating: 1.0, count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opDoubleSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opDoubleSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [String]?
        var p2: [String]?
        var p3: [String]?

        (p2, p3) = try initial.opStringSeq(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opStringSeq(nil)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opStringSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [String](repeating: "test", count: 100)
        (p2, p3) = try initial.opStringSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opStringSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opStringSeq([String](repeating: "test", count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opStringSeqAsync([String](repeating: "test", count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opStringSeq(nil)
        try test(p2 == nil && p3 == nil); // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opStringSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(istr.read(tag: 1) == p1)
        try test(istr.read(tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: SmallStructSeq?
        var p2: SmallStructSeq?
        var p3: SmallStructSeq?
        (p2, p3) = try initial.opSmallStructSeq(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opSmallStructSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = SmallStructSeq(repeating: SmallStruct(), count: 100)
        (p2, p3) = try initial.opSmallStructSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opSmallStructSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opSmallStructSeq(SmallStructSeq(repeating: SmallStruct(), count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opSmallStructSeqAsync(SmallStructSeq(repeating: SmallStruct(), count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opSmallStructSeq(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        SmallStructSeqHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opSmallStructSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(SmallStructSeqHelper.read(from: istr, tag: 1) == p1)
        try test(SmallStructSeqHelper.read(from: istr, tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: FixedStructSeq?
        var p2: FixedStructSeq?
        var p3: FixedStructSeq?
        (p2, p3) = try initial.opFixedStructSeq(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opFixedStructSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = FixedStructSeq(repeating: FixedStruct(), count: 100)
        (p2, p3) = try initial.opFixedStructSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opFixedStructSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFixedStructSeq(FixedStructSeq(repeating: FixedStruct(), count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opFixedStructSeqAsync(FixedStructSeq(repeating: FixedStruct(), count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opFixedStructSeq(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        FixedStructSeqHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opFixedStructSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(FixedStructSeqHelper.read(from: istr, tag: 1) == p1)
        try test(FixedStructSeqHelper.read(from: istr, tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: VarStructSeq?
        var p2: VarStructSeq?
        var p3: VarStructSeq?
        (p2, p3) = try initial.opVarStructSeq(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opVarStructSeq(nil)
        try test(p2 == nil && p3 == nil)

        p1 = VarStructSeq(repeating: VarStruct(), count: 100)
        (p2, p3) = try initial.opVarStructSeq(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opVarStructSeqAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opVarStructSeq(VarStructSeq(repeating: VarStruct(), count: 100))
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opVarStructSeqAsync(VarStructSeq(repeating: VarStruct(), count: 100))
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opVarStructSeq(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        VarStructSeqHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opVarStructSeq", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(VarStructSeqHelper.read(from: istr, tag: 1) == p1)
        try test(VarStructSeqHelper.read(from: istr, tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [Int32: Int32]?
        var p2: [Int32: Int32]?
        var p3: [Int32: Int32]?
        (p2, p3) = try initial.opIntIntDict(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opIntIntDict(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [1: 2, 2: 3]
        (p2, p3) = try initial.opIntIntDict(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opIntIntDictAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntIntDict([1: 2, 2: 3])
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opIntIntDictAsync([1: 2, 2: 3])
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntIntDict(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        IntIntDictHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opIntIntDict", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(IntIntDictHelper.read(from: istr, tag: 1) == p1)
        try test(IntIntDictHelper.read(from: istr, tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }

    do {
        var p1: [String: Int32]?
        var p2: [String: Int32]?
        var p3: [String: Int32]?

        (p2, p3) = try initial.opStringIntDict(p1)
        try test(p2 == nil && p3 == nil)

        (p2, p3) = try initial.opStringIntDict(nil)
        try test(p2 == nil && p3 == nil)

        p1 = ["1": 1, "2": 2]
        (p2, p3) = try initial.opStringIntDict(p1)
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opStringIntDictAsync(p1)
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opStringIntDict(["1": 1, "2": 2])
        try test(p2 == p1 && p3 == p1)

        try Promise<Void> { seal in
            firstly {
                initial.opStringIntDictAsync(["1": 1, "2": 2])
            }.done { p2, p3 in
                try test(p2 == p1 && p3 == p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opStringIntDict(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        var ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        StringIntDictHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        var inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opStringIntDict", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try test(StringIntDictHelper.read(from: istr, tag: 1) == p1)
        try test(StringIntDictHelper.read(from: istr, tag: 3) == p1)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()

        let f = F()
        f.af = A()
        f.af!.requiredA = 56
        f.ae = f.af

        ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        ostr.write(tag: 1, value: f)
        ostr.write(tag: 2, value: f.ae)
        ostr.endEncapsulation()
        inEncaps = ostr.finished()

        istr = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try istr.startEncapsulation()
        var a: Value?
        try istr.read(tag: 2) { a = $0 }
        try istr.endEncapsulation()
        try test(a != nil && (a as! A).requiredA == 56)
    }

    do {
        var p1: [Int32: OneOptional?]?
        var p2: [Int32: OneOptional?]?
        var p3: [Int32: OneOptional?]?

        (p2, p3) = try initial.opIntOneOptionalDict(p1)
        try test(p2 == nil && p3 == nil)
        (p2, p3) = try initial.opIntOneOptionalDict(nil)
        try test(p2 == nil && p3 == nil)

        p1 = [1: OneOptional(a: 58), 2: OneOptional(a: 59)]
        (p2, p3) = try initial.opIntOneOptionalDict(p1)
        try test(p2![1]!!.a == 58 && p3![1]!!.a == 58)

        try Promise<Void> { seal in
            firstly {
                initial.opIntOneOptionalDictAsync(p1)
            }.done { p2, p3 in
                try test(p2![1]!!.a == 58 && p3![1]!!.a == 58)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntOneOptionalDict([1: OneOptional(a: 58), 2: OneOptional(a: 59)])
        try test(p2![1]!!.a == 58 && p3![1]!!.a == 58)

        try Promise<Void> { seal in
            firstly {
                initial.opIntOneOptionalDictAsync([1: OneOptional(a: 58), 2: OneOptional(a: 59)])
            }.done { p2, p3 in
                try test(p2![1]!!.a == 58 && p3![1]!!.a == 58)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        (p2, p3) = try initial.opIntOneOptionalDict(nil)
        try test(p2 == nil && p3 == nil) // Ensure out parameter is cleared.

        let ostr = Ice.OutputStream(communicator: communicator)
        ostr.startEncapsulation()
        IntOneOptionalDictHelper.write(to: ostr, tag: 2, value: p1)
        ostr.endEncapsulation()
        let inEncaps = ostr.finished()
        let result = try initial.ice_invoke(operation: "opIntOneOptionalDict", mode: .Normal, inEncaps: inEncaps)
        var istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        p2 = try IntOneOptionalDictHelper.read(from: istr, tag: 1)
        try test(p2![1]!!.a == 58)
        p3 = try IntOneOptionalDictHelper.read(from: istr, tag: 3)
        try test(p3![1]!!.a == 58)
        try istr.endEncapsulation()

        istr = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try istr.startEncapsulation()
        try istr.endEncapsulation()
    }
    output.writeLine("ok")

    output.write("testing exception optionals... ")
    do {
        try initial.opOptionalException(a: nil, b: nil, o: nil)
    } catch let ex as OptionalException {
        try test(ex.a == nil)
        try test(ex.b == nil)
        try test(ex.o == nil)
    }

    do {
        try initial.opOptionalException(a: 30, b: "test", o: OneOptional(a: 53))
    } catch let ex as OptionalException {
        try test(ex.a == 30)
        try test(ex.b == "test")
        try test(ex.o!.a == 53)
    }

    do {
        //
        // Use the 1.0 encoding with an exception whose only class members are optional.
        //
        let initial2 = initial.ice_encodingVersion(Ice.Encoding_1_0)
        try initial2.opOptionalException(a: 30, b: "test", o: OneOptional(a: 53))
    } catch let ex as OptionalException {
        try test(ex.a == nil)
        try test(ex.b == nil)
        try test(ex.o == nil)
    }

    do {
        try initial.opDerivedException(a: nil, b: nil, o: nil)
    } catch let ex as DerivedException {
        try test(ex.a == nil)
        try test(ex.b == nil)
        try test(ex.o == nil)
        try test(ex.ss == nil)
        try test(ex.o2 == nil)
    }

    do {
        try initial.opDerivedException(a: 30, b: "test2", o: OneOptional(a: 53))
    } catch let ex as DerivedException {
        try test(ex.a == 30)
        try test(ex.b == "test2")
        try test(ex.o!.a == 53)
        try test(ex.ss == "test2")
        try test(ex.o2!.a == 53)
    }

    do {
        try initial.opRequiredException(a: nil, b: nil, o: nil)
    } catch let ex as RequiredException {
        try test(ex.a == nil)
        try test(ex.b == nil)
        try test(ex.o == nil)
        try test(ex.ss == "test")
        try test(ex.o2 == nil)
    }

    do {
        try initial.opRequiredException(a: 30, b: "test2", o: OneOptional(a: 53))
    } catch let ex as RequiredException {
        try test(ex.a == 30)
        try test(ex.b == "test2")
        try test(ex.o!.a == 53)
        try test(ex.ss == "test2")
        try test(ex.o2!.a == 53)
    }
    output.writeLine("ok")

}
