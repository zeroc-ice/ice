//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {

        PromiseKit.conf.Q.map = .global()
        PromiseKit.conf.Q.return = .global()
        PromiseKit.conf.logHandler = { _ in }

        let output = getWriter()

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        output.write("testing stringToProxy... ")
        let basePrx = try communicator.stringToProxy("Test:\(self.getTestEndpoint(num: 0)) -t 2000")!
        output.writeLine("ok")

        output.write("testing checked cast... ")
        let testPrx = try checkedCast(prx: basePrx, type: TestIntfPrx.self)!
        try test(testPrx == basePrx)
        output.writeLine("ok")

        output.write("base as Object... ")
        do {
            let o = try testPrx.SBaseAsObject()
            let sb = o as! SBase
            try test(sb.ice_id() == "::Test::SBase")
            try test(sb.sb == "SBase.sb")
        }
        output.writeLine("ok")

        output.write("base as Object (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.SBaseAsObjectAsync()
            }.map { o in
                let sb = o as! SBase
                try self.test(sb.ice_id() == "::Test::SBase")
                try self.test(sb.sb == "SBase.sb")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("base as base... ")
        do {
            let sb = try testPrx.SBaseAsSBase()!
            try test(sb.sb == "SBase.sb")
        }
        output.writeLine("ok")

        output.write("base as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.SBaseAsSBaseAsync()
            }.done { sb in
                try self.test(sb!.sb == "SBase.sb")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("base with known derived as base... ")
        do {
            let sb = try testPrx.SBSKnownDerivedAsSBase()!
            try test(sb.sb == "SBSKnownDerived.sb")
            let sbskd = sb as! SBSKnownDerived
            try test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        }
        output.writeLine("ok")

        output.write("base with known derived as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.SBSKnownDerivedAsSBaseAsync()
            }.done { sb in
                try self.test(sb!.sb == "SBSKnownDerived.sb")
                let sbskd = sb as! SBSKnownDerived
                try self.test(sbskd.sbskd == "SBSKnownDerived.sbskd")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("base with known derived as known derived... ")
        do {
            let sbskd = try testPrx.SBSKnownDerivedAsSBSKnownDerived()!
            try test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        }
        output.writeLine("ok")

        output.write("base with known derived as known derived (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.SBSKnownDerivedAsSBSKnownDerivedAsync()
            }.done { sbskd in
                try self.test(sbskd!.sbskd == "SBSKnownDerived.sbskd")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("base with unknown derived as base... ")
        do {
            let sb = try testPrx.SBSUnknownDerivedAsSBase()!
            try test(sb.sb == "SBSUnknownDerived.sb")
        }

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            do {
                let sb = try testPrx.SBSUnknownDerivedAsSBaseCompact()!
                try test(sb.sb == "SBSUnknownDerived.sb")
            }
        } else {
            do {
                //
                // This test fails when using the compact format because the instance cannot
                // be sliced to a known type.
                //
                _ = try testPrx.SBSUnknownDerivedAsSBaseCompact()
                try test(false)
            } catch is Ice.NoValueFactoryException { // Expected.
            }
        }
        output.writeLine("ok")

        output.write("base with unknown derived as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.SBSUnknownDerivedAsSBaseAsync()
            }.done { sb in
                try self.test(sb!.sb == "SBSUnknownDerived.sb")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            //
            // This test succeeds for the 1.0 encoding.
            //
            try Promise<Void> { seal in
                firstly {
                    testPrx.SBSUnknownDerivedAsSBaseCompactAsync()
                }.done { sb in
                    try self.test(sb!.sb == "SBSUnknownDerived.sb")
                    seal.fulfill(())
                }.catch { e in
                    seal.reject(e)
                }
            }.wait()
        } else {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            try Promise<Void> { seal in
                firstly {
                    testPrx.SBSUnknownDerivedAsSBaseCompactAsync()
                }.done { _ in
                    try self.test(false)
                }.catch { ex in
                    do {
                        try self.test(ex is Ice.NoValueFactoryException)
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()
        }
        output.writeLine("ok")

        output.write("unknown with Object as Object... ")
        do {
            let o = try testPrx.SUnknownAsObject()!
            try test(testPrx.ice_getEncodingVersion() != Ice.Encoding_1_0)
            try test(o is Ice.UnknownSlicedValue)
            try test((o as! Ice.UnknownSlicedValue).ice_id() == "::Test::SUnknown")
            try test((o as! Ice.UnknownSlicedValue).ice_getSlicedData() != nil)
            try testPrx.checkSUnknown(o)
        } catch is Ice.NoValueFactoryException {
            try test(testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0)
        }
        output.writeLine("ok")

        output.write("unknown with Object as Object (AMI)... ")
        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try Promise<Void> { seal in
                firstly {
                    testPrx.SUnknownAsObjectAsync()
                }.done { _ in
                    try self.test(false)
                }.catch { ex in
                    do {
                        try self.test(ex is Ice.NoValueFactoryException)
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()
        } else {
            try Promise<Void> { seal in
                firstly {
                    testPrx.SUnknownAsObjectAsync()
                }.done { o in
                    if let unknown = o as? Ice.UnknownSlicedValue {
                        try self.test(unknown.ice_id() == "::Test::SUnknown")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                }.catch { e in
                    seal.reject(e)
                }
            }.wait()
        }
        output.writeLine("ok")

        output.write("one-element cycle... ")
        do {
            let b = try testPrx.oneElementCycle()!
            try test(b.ice_id() == "::Test::B")
            try test(b.sb == "B1.sb")
            try test(b.pb === b)
        }
        output.writeLine("ok")

        output.write("one-element cycle (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.oneElementCycleAsync()
            }.done { b in
                try self.test(b!.ice_id() == "::Test::B")
                try self.test(b!.sb == "B1.sb")
                try self.test(b!.pb === b)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("two-element cycle... ")
        do {
            let b1 = try testPrx.twoElementCycle()!
            try test(b1.ice_id() == "::Test::B")
            try test(b1.sb == "B1.sb")
            let b2 = b1.pb!
            try test(b2.ice_id() == "::Test::B")
            try test(b2.sb == "B2.sb")
            try test(b2.pb === b1)
        }
        output.writeLine("ok")

        output.write("two-element cycle (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.twoElementCycleAsync()
            }.done { o in
                let b1 = o!
                try self.test(b1.ice_id() == "::Test::B")
                try self.test(b1.sb == "B1.sb")

                let b2 = b1.pb!
                try self.test(b2.ice_id() == "::Test::B")
                try self.test(b2.sb == "B2.sb")
                try self.test(b2.pb === b1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("known derived pointer slicing as base... ")
        do {
            let b1 = try testPrx.D1AsB()!
            try test(b1.ice_id() == "::Test::D1")
            try test(b1.sb == "D1.sb")
            try test(b1.pb !== nil)
            try test(b1.pb !== b1)
            if let d1 = b1 as? D1 {
                try test(d1 !== nil)
                try test(d1.sd1 == "D1.sd1")
                try test(d1.pd1 !== nil)
                try test(d1.pd1 !== b1)
                try test(b1.pb === d1.pd1)
            } else {
                try test(false)
            }

            let b2 = b1.pb!
            try test(b2.pb === b1)
            try test(b2.sb == "D2.sb")
            try test(b2.ice_id() == "::Test::B")
        }
        output.writeLine("ok")

        output.write("known derived pointer slicing as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.D1AsBAsync()
            }.done { o in
                let b1 = o!

                try self.test(b1.ice_id() == "::Test::D1")
                try self.test(b1.sb == "D1.sb")
                try self.test(b1.pb !== nil)
                try self.test(b1.pb !== b1)

                if let d1 = b1 as? D1 {
                    try self.test(d1 !== nil)
                    try self.test(d1.sd1 == "D1.sd1")
                    try self.test(d1.pd1 !== nil)
                    try self.test(d1.pd1 !== b1)
                    try self.test(b1.pb === d1.pd1)
                } else {
                    try self.test(false)
                }

                let b2 = b1.pb!
                try self.test(b2 !== nil)
                try self.test(b2.pb === b1)
                try self.test(b2.sb == "D2.sb")
                try self.test(b2.ice_id() == "::Test::B")
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("known derived pointer slicing as derived... ")
        do {
            let d1 = try testPrx.D1AsD1()!
            try test(d1.ice_id() == "::Test::D1")
            try test(d1.sb == "D1.sb")
            try test(d1.pb !== nil)
            try test(d1.pb !== d1)

            let b2 = d1.pb!
            try test(b2.ice_id() == "::Test::B")
            try test(b2.sb == "D2.sb")
            try test(b2.pb === d1)
        }
        output.writeLine("ok")

        output.write("known derived pointer slicing as derived (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                try testPrx.D1AsD1Async()
            }.done { o in
                let d1 = o!
                try self.test(d1.ice_id() == "::Test::D1")
                try self.test(d1.sb == "D1.sb")
                try self.test(d1.pb !== nil)
                try self.test(d1.pb !== d1)

                let b2 = d1.pb!
                try self.test(b2.ice_id() == "::Test::B")
                try self.test(b2.sb == "D2.sb")
                try self.test(b2.pb === d1)

                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("unknown derived pointer slicing as base... ")
        do {
            let b2 = try testPrx.D2AsB()!
            try test(b2.ice_id() == "::Test::B")
            try test(b2.sb == "D2.sb")
            try test(b2.pb !== nil)
            try test(b2.pb !== b2)

            let b1 = b2.pb!
            try test(b1.ice_id() == "::Test::D1")
            try test(b1.sb == "D1.sb")
            try test(b1.pb === b2)
            if let d1 = b1 as? D1 {
                try test(d1.sd1 == "D1.sd1")
                try test(d1.pd1 === b2)
            } else {
                try test(false)
            }
        }
        output.writeLine("ok")

        output.write("unknown derived pointer slicing as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.D2AsBAsync()
            }.done { o in
                let b2 = o!

                try self.test(b2.ice_id() == "::Test::B")
                try self.test(b2.sb == "D2.sb")
                try self.test(b2.pb !== nil)
                try self.test(b2.pb !== b2)

                let b1 = b2.pb!

                try self.test(b1.ice_id() == "::Test::D1")
                try self.test(b1.sb == "D1.sb")
                try self.test(b1.pb === b2)

                if let d1 = b1 as? D1 {
                    try self.test(d1.sd1 == "D1.sd1")
                    try self.test(d1.pd1 === b2)
                } else {
                    try self.test(false)
                }
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("param ptr slicing with known first... ")
        do {
            let (b1, b2) = try testPrx.paramTest1()
            try test(b1 !== nil)
            try test(b1!.ice_id() == "::Test::D1")
            try test(b1!.sb == "D1.sb")
            try test(b1!.pb === b2)
            let d1 = b1 as! D1
            try test(d1.sd1 == "D1.sd1")
            try test(d1.pd1 === b2)

            try test(b2 !== nil)
            // No factory, must be sliced
            try test(b2!.ice_id() == "::Test::B")
            try test(b2!.sb == "D2.sb")
            try test(b2!.pb === b1)
        }
        output.writeLine("ok")

        output.write("param ptr slicing with known first (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.paramTest1Async()
            }.done { o1, o2 in
                try self.test(o1 != nil && o2 != nil)
                let b1 = o1!
                let b2 = o2!
                try self.test(b1.ice_id() == "::Test::D1")
                try self.test(b1.sb == "D1.sb")
                try self.test(b1.pb === b2)
                let d1 = b1 as! D1
                try self.test(d1.sd1 == "D1.sd1")
                try self.test(d1.pd1 === b2)
                // No factory, must be sliced
                try self.test(b2.ice_id() == "::Test::B")
                try self.test(b2.sb == "D2.sb")
                try self.test(b2.pb === b1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("param ptr slicing with unknown first... ")
        do {
            let (o2, o1) = try testPrx.paramTest2()
            try test(o1 != nil && o2 != nil)

            let b1 = o1!
            let b2 = o2!

            try test(b1.ice_id() == "::Test::D1")
            try test(b1.sb == "D1.sb")
            try test(b1.pb === b2)

            if let d1 = b1 as? D1 {
                try test(d1.sd1 == "D1.sd1")
                try test(d1.pd1 === b2)
            } else {
                try test(false)
            }
            // No factory, must be sliced
            try test(b2.ice_id() == "::Test::B")
            try test(b2.sb == "D2.sb")
            try test(b2.pb === b1)
        }
        output.writeLine("ok")

        output.write("param ptr slicing with unknown first (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.paramTest2Async()
            }.done { o2, o1 in
                try self.test(o2 != nil && o1 != nil)
                let b1 = o1!
                let b2 = o2!

                try self.test(b1.ice_id() == "::Test::D1")
                try self.test(b1.sb == "D1.sb")
                try self.test(b1.pb === b2)
                if let d1 = b1 as? D1 {
                    try self.test(d1.sd1 == "D1.sd1")
                    try self.test(d1.pd1 === b2)
                } else {
                    try self.test(false)
                }
                // No factory, must be sliced
                try self.test(b2.ice_id() == "::Test::B")
                try self.test(b2.sb == "D2.sb")
                try self.test(b2.pb === b1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("return value identity with known first... ")
        do {
            let (ret, p1, p2) = try testPrx.returnTest1()
            try test(ret === p1)
        }
        output.writeLine("ok")

        output.write("return value identity with known first (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.returnTest1Async()
            }.done { r, p1, p2 in
                try self.test(r === p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("return value identity with unknown first... ")
        do {
            let (ret, p1, p2) = try testPrx.returnTest2()
            try test(ret === p1)
        }
        output.writeLine("ok")

        output.write("return value identity with unknown first (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.returnTest2Async()
            }.done { r, p1, p2 in
                try self.test(r === p1)
                seal.fulfill(())
            }.catch { e in
                seal.reject(e)
            }
        }.wait()
        output.writeLine("ok")

        output.write("return value identity for input params known first... ")
        do {
            let d1 = D1()
            d1.sb = "D1.sb"
            d1.sd1 = "D1.sd1"
            let d3 = D3()
            d3.pb = d1
            d3.sb = "D3.sb"
            d3.sd3 = "D3.sd3"
            d3.pd3 = d1
            d1.pb = d3
            d1.pd1 = d3

            let b1 = try testPrx.returnTest3(p1: d1, p2: d3)!
            try test(b1.sb == "D1.sb")
            try test(b1.ice_id() == "::Test::D1")

            if let p1 = b1 as? D1 {
                try test(p1.sd1 == "D1.sd1")
                try test(p1.pd1 === b1.pb)
            } else {
                try test(false)
            }

            let b2 = b1.pb!
            try test(b2.sb == "D3.sb")
            // Sliced by server
            try test(b2.ice_id() == "::Test::B")
            try test(b2.pb === b1)
            if let _ = b2 as? D3 {
                try test(false)
            }

            try test(b1 !== d1)
            try test(b1 !== d3)
            try test(b2 !== d1)
            try test(b2 !== d3)
        }
        output.writeLine("ok")

        output.write("return value identity for input params known first (AMI)... ")
        do {
            let d1 = D1()
            d1.sb = "D1.sb"
            d1.sd1 = "D1.sd1"
            let d3 = D3()
            d3.pb = d1
            d3.sb = "D3.sb"
            d3.sd3 = "D3.sd3"
            d3.pd3 = d1
            d1.pb = d3
            d1.pd1 = d3

            try Promise<Void> { seal in
                firstly {
                    testPrx.returnTest3Async(p1: d1, p2: d3)
                }.done { b in
                    try self.test(b != nil)
                    let b1 = b!

                    try self.test(b1.sb == "D1.sb")
                    try self.test(b1.ice_id() == "::Test::D1")

                    if let p1 = b1 as? D1 {
                        try self.test(p1.sd1 == "D1.sd1")
                        try self.test(p1.pd1 === b1.pb)
                    } else {
                        try self.test(false)
                    }

                    let b2 = b1.pb!
                    try self.test(b2 != nil)
                    try self.test(b2.sb == "D3.sb")
                    // Sliced by server
                    try self.test(b2.ice_id() == "::Test::B")
                    try self.test(b2.pb === b1)
                    if let _ = b2 as? D3 {
                        try self.test(false)
                    }

                    try self.test(b1 !== d1)
                    try self.test(b1 !== d3)
                    try self.test(b2 !== d1)
                    try self.test(b2 !== d3)

                    seal.fulfill(())
                }.catch { e in
                    seal.reject(e)
                }
            }.wait()
            output.writeLine("ok")
        }
        try testPrx.shutdown()
    }
}
