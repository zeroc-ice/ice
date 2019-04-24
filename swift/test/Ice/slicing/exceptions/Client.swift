//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let output = getWriter()

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        output.write("testing stringToProxy... ")
        let ref = "Test:\(self.getTestEndpoint(num: 0)) -t 2000"
        let base = try communicator.stringToProxy(ref)!
        output.writeLine("ok")

        output.write("testing checked cast... ")
        let testPrx = try checkedCast(prx: base, type: TestIntfPrx.self)!
        try test(testPrx == base)
        output.writeLine("ok")

        output.write("base... ")
        do {
            try testPrx.baseAsBase()
            try test(false)
        } catch let b as Base {
            try test(b.b == "Base.b")
        }
        output.writeLine("ok")

        output.write("base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.baseAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let b = e as? Base {
                        try self.test(b.b == "Base.b")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of unknown derived... ")
        do {
            try testPrx.unknownDerivedAsBase()
            try test(false)
        } catch let b as Base {
            try test(b.b == "UnknownDerived.b")
        }
        output.writeLine("ok")

        output.write("slicing of unknown derived (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.unknownDerivedAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let b = e as? Base {
                        try self.test(b.b == "UnknownDerived.b")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("non-slicing of known derived as base... ")
        do {
            try testPrx.knownDerivedAsBase()
            try test(false)
        } catch let k as KnownDerived {
            try test(k.b == "KnownDerived.b")
            try test(k.kd == "KnownDerived.kd")
        }
        output.writeLine("ok")

        output.write("non-slicing of known derived as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownDerivedAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let k = e as? KnownDerived {
                        try self.test(k.b == "KnownDerived.b")
                        try self.test(k.kd == "KnownDerived.kd")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("non-slicing of known derived as derived... ")
        do {
            try testPrx.knownDerivedAsKnownDerived()
            try test(false)
        } catch let k as KnownDerived {
            try test(k.b == "KnownDerived.b")
            try test(k.kd == "KnownDerived.kd")
        }
        output.writeLine("ok")

        output.write("non-slicing of known derived as derived (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownDerivedAsKnownDerivedAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let k = e as? KnownDerived {
                        try self.test(k.b == "KnownDerived.b")
                        try self.test(k.kd == "KnownDerived.kd")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of unknown intermediate as base... ")
        do {
            try testPrx.unknownIntermediateAsBase()
            try test(false)
        } catch let b as Base {
            try test(b.b == "UnknownIntermediate.b")
        }
        output.writeLine("ok")

        output.write("slicing of unknown intermediate as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.unknownIntermediateAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let b = e as? Base {
                        try self.test(b.b == "UnknownIntermediate.b")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of known intermediate as base... ")
        do {
            try testPrx.knownIntermediateAsBase()
            try test(false)
        } catch let ki as KnownIntermediate {
            try test(ki.b == "KnownIntermediate.b")
            try test(ki.ki == "KnownIntermediate.ki")
        }
        output.writeLine("ok")

        output.write("slicing of known intermediate as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownIntermediateAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ki = e as? KnownIntermediate {
                        try self.test(ki.b == "KnownIntermediate.b")
                        try self.test(ki.ki == "KnownIntermediate.ki")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of known most derived as base... ")
        do {
            try testPrx.knownMostDerivedAsBase()
            try test(false)
        } catch let kmd as KnownMostDerived {
            try test(kmd.b == "KnownMostDerived.b")
            try test(kmd.ki == "KnownMostDerived.ki")
            try test(kmd.kmd == "KnownMostDerived.kmd")
        }
        output.writeLine("ok")

        output.write("slicing of known most derived as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownMostDerivedAsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let kmd = e as? KnownMostDerived {
                        try self.test(kmd.b == "KnownMostDerived.b")
                        try self.test(kmd.ki == "KnownMostDerived.ki")
                        try self.test(kmd.kmd == "KnownMostDerived.kmd")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("non-slicing of known intermediate as intermediate... ")
        do {
            try testPrx.knownIntermediateAsKnownIntermediate()
            try test(false)
        } catch let ki as KnownIntermediate {
            try test(ki.b == "KnownIntermediate.b")
            try test(ki.ki == "KnownIntermediate.ki")
        }
        output.writeLine("ok")

        output.write("non-slicing of known intermediate as intermediate (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownIntermediateAsKnownIntermediateAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ki = e as? KnownIntermediate {
                        try self.test(ki.b == "KnownIntermediate.b")
                        try self.test(ki.ki == "KnownIntermediate.ki")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("non-slicing of known most derived as intermediate... ")
        do {
            try testPrx.knownMostDerivedAsKnownIntermediate()
            try test(false)
        } catch let kmd as KnownMostDerived {
            try test(kmd.b == "KnownMostDerived.b")
            try test(kmd.ki == "KnownMostDerived.ki")
            try test(kmd.kmd == "KnownMostDerived.kmd")
        }
        output.writeLine("ok")

        output.write("non-slicing of known most derived as intermediate (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownMostDerivedAsKnownIntermediateAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let kmd = e as? KnownMostDerived {
                        try self.test(kmd.b == "KnownMostDerived.b")
                        try self.test(kmd.ki == "KnownMostDerived.ki")
                        try self.test(kmd.kmd == "KnownMostDerived.kmd")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("non-slicing of known most derived as most derived... ")
        do {
            try testPrx.knownMostDerivedAsKnownMostDerived()
            try test(false)
        } catch let kmd as KnownMostDerived {
            try test(kmd.b == "KnownMostDerived.b")
            try test(kmd.ki == "KnownMostDerived.ki")
            try test(kmd.kmd == "KnownMostDerived.kmd")
        }
        output.writeLine("ok")

        output.write("non-slicing of known most derived as most derived (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.knownMostDerivedAsKnownMostDerivedAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let kmd = e as? KnownMostDerived {
                        try self.test(kmd.b == "KnownMostDerived.b")
                        try self.test(kmd.ki == "KnownMostDerived.ki")
                        try self.test(kmd.kmd == "KnownMostDerived.kmd")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of unknown most derived, known intermediate as base... ")
        do {
            try testPrx.unknownMostDerived1AsBase()
            try test(false)
        } catch let ki as KnownIntermediate {
            try test(ki.b == "UnknownMostDerived1.b")
            try test(ki.ki == "UnknownMostDerived1.ki")
        }
        output.writeLine("ok")

        output.write("slicing of unknown most derived, known intermediate as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.unknownMostDerived1AsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ki = e as? KnownIntermediate {
                        try self.test(ki.b == "UnknownMostDerived1.b")
                        try self.test(ki.ki == "UnknownMostDerived1.ki")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of unknown most derived, known intermediate as intermediate... ")
        do {
            try testPrx.unknownMostDerived1AsKnownIntermediate()
            try test(false)
        } catch let ki as KnownIntermediate {
            try test(ki.b == "UnknownMostDerived1.b")
            try test(ki.ki == "UnknownMostDerived1.ki")
        }
        output.writeLine("ok")

        output.write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.unknownMostDerived1AsKnownIntermediateAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ki = e as? KnownIntermediate {
                        try self.test(ki.b == "UnknownMostDerived1.b")
                        try self.test(ki.ki == "UnknownMostDerived1.ki")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("slicing of unknown most derived, unknown intermediate thrown as base... ")
        do {
            try testPrx.unknownMostDerived2AsBase()
            try test(false)
        } catch let b as Base {
            try test(b.b == "UnknownMostDerived2.b")
        }
        output.writeLine("ok")

        output.write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ")
        try Promise<Void> { seal in
            firstly {
                testPrx.unknownMostDerived2AsBaseAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let b = e as? Base {
                        try self.test(b.b == "UnknownMostDerived2.b")
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("unknown most derived in compact format... ")
        do {
            try testPrx.unknownMostDerived2AsBaseCompact()
            try test(false)
        } catch is Base {
            //
            // For the 1.0 encoding, the unknown exception is sliced to Base.
            //
            try test(testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0)
        } catch is Ice.UnknownUserException {
            //
            // A MarshalException is raised for the compact format because the
            // most-derived type is unknown and the exception cannot be sliced.
            //
            try test(testPrx.ice_getEncodingVersion() != Ice.Encoding_1_0)
        } catch is Ice.OperationNotExistException {
        }
        output.writeLine("ok")

        output.write("preserved exceptions... ")
        do {
            try testPrx.unknownPreservedAsBase()
            try test(false)
        } catch let ex as Base {
            if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
                try test(ex.ice_getSlicedData() == nil)
            } else {
                let slicedData = ex.ice_getSlicedData()!
                try test(slicedData.slices.count == 2)
                try test(slicedData.slices[1].typeId == "::Test::SPreserved1")
                try test(slicedData.slices[0].typeId == "::Test::SPreserved2")
            }
        }

        do {
            try testPrx.unknownPreservedAsKnownPreserved()
            try test(false)
        } catch let ex as KnownPreserved {
            try test(ex.kp == "preserved")
            if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
                try test(ex.ice_getSlicedData() == nil)
            } else {
                let slicedData = ex.ice_getSlicedData()!
                try test(slicedData.slices.count == 2)
                try test(slicedData.slices[1].typeId == "::Test::SPreserved1")
                try test(slicedData.slices[0].typeId == "::Test::SPreserved2")
            }
        }

        // TODO enable once we have finish ported server side
        /* do {
            let adapter = try communicator.createObjectAdapter("")
            let relay = uncheckedCast(prx: adapter.addWithUUID(RelayI()),
                                      type: RelayPrx.self)
            try adapter.activate()
            try testPrx.ice_getConnection()!.setAdapter(adapter)

            do {
                try testPrx.relayKnownPreservedAsBase(relay)
                try test(false)
            } catch let ex as KnownPreservedDerived {
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
            } catch is Ice.OperationNotExistException {
            }

            do {
                try testPrx.relayKnownPreservedAsKnownPreserved(relay)
                try test(false)
            } catch let ex as KnownPreservedDerived {
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
            } catch is Ice.OperationNotExistException {
            }

            do {
                try testPrx.relayUnknownPreservedAsBase(relay)
                try test(false)
            } catch let ex as Preserved2 {
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
                try test(ex.p1!.ice_id() == PreservedClass.ice_staticId())
                let pc = ex.p1 as? PreservedClass
                try test(pc!.bc == "bc")
                try test(pc!.pc == "pc")
                try test(ex.p2 === ex.p1)
            } catch let ex as KnownPreservedDerived {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                try test(testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0)
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
            } catch is Ice.OperationNotExistException {
            }

            do {
                try testPrx.relayUnknownPreservedAsKnownPreserved(relay)
                try test(false)
            } catch let ex as Preserved2 {
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
                try test(ex.p1!.ice_id() == PreservedClass.ice_staticId())
                let pc = ex.p1 as? PreservedClass
                try test(pc!.bc == "bc")
                try test(pc!.pc == "pc")
                try test(ex.p2 === ex.p1)
            } catch let ex as KnownPreservedDerived {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                try test(testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0)
                try test(ex.b == "base")
                try test(ex.kp == "preserved")
                try test(ex.kpd == "derived")
            } catch is Ice.OperationNotExistException {
            }

            adapter.destroy();
        } */
        output.writeLine("ok")
        try testPrx.shutdown()
    }
}
