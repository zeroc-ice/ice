//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

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

    init() {
        _enabled = false
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
            return DValueReader()
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
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        let factory = FactoryI()
        try communicator.getValueFactoryManager().add(factory: { id in factory.create(id) }, id: "")

        let output = self.getWriter()
        output.write("testing stringToProxy... ")
        let ref = "initial:\(self.getTestEndpoint(num: 0))"
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

        output.writeLine("ok")

        /*output.Write("testing marshaling of large containers with fixed size elements... ");
        output.Flush();
        Test.MultiOptional mc = new Test.MultiOptional();

        mc.bs = new byte[1000];
        mc.shs = new short[300];

        mc.fss = new Test.FixedStruct[300];
        for(int i = 0; i < 300; ++i)
        {
            mc.fss.Value[i] = new Test.FixedStruct();
        }

        mc.ifsd = new Dictionary<int, Test.FixedStruct>();
        for(int i = 0; i < 300; ++i)
        {
            mc.ifsd.Value.Add(i, new Test.FixedStruct());
        }

        mc =(Test.MultiOptional)initial.pingPong(mc);
        test(mc.bs.Value.Length == 1000);
        test(mc.shs.Value.Length == 300);
        test(mc.fss.Value.Length == 300);
        test(mc.ifsd.Value.Count == 300);

        factory.setEnabled(true);
        os = new Ice.OutputStream(communicator);
        os.startEncapsulation();
        os.writeValue(mc);
        os.endEncapsulation();
        inEncaps = os.finished();
        test(initial.ice_invoke("pingPong", Ice.OperationMode.Normal, inEncaps, out outEncaps));
        @in = new Ice.InputStream(communicator, outEncaps);
        @in.startEncapsulation();
        @in.readValue(cb.invoke);
        @in.endEncapsulation();
        test(cb.obj != null && cb.obj is TestValueReader);
        factory.setEnabled(false);*/

        output.writeLine("ok")

        try initial.shutdown()
    }
}
