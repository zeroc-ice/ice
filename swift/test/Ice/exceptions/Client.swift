//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

extension G {
    var _GDescription: String {
        return "LocalException: G"
    }
}

extension H {
    var _HDescription: String {
        return "LocalException: H"
    }
}

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {

        PromiseKit.conf.Q.map = .global()
        PromiseKit.conf.Q.return = .global()
        PromiseKit.conf.logHandler = { _ in }

        let writer = getWriter()

        var initData = Ice.InitializationData()
        let (properties, _) = try createTestProperties(args: args)
        try properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        try properties.setProperty(key: "Ice.MessageSizeMax", value: "10") // 10KB max
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        try communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                     value: self.getTestEndpoint(num: 0))

        let output = self.getWriter()
        output.write("testing object adapter registration exceptions... ")
        // TODO
        // do {
        //    _ = try communicator.createObjectAdapter("TestAdapter0")
        // } catch is Ice.InitializationException {}

        try communicator.getProperties().setProperty(key: "TestAdapter0.Endpoints", value: "tcp -h *")
        // TODO
        // var first = communicator.createObjectAdapter("TestAdapter0")
        // do {
        //    communicator.createObjectAdapter("TestAdapter0")
        //    try test(false)
        // } catch is Ice.AlreadyRegisteredException {}

        // TODO
        // do {
        //     let second = try communicator.createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011")
        //     try test(false)
        //     //
        //     // Quell mono error that variable second isn't used.
        //     //
        //     try second.deactivate()
        // } catch is Ice.AlreadyRegisteredException {}
        // try first.deactivate()
        output.writeLine("ok")

        output.write("testing servant registration exceptions... ")
        // try communicator.getProperties().setProperty(key: "TestAdapter1.Endpoints", value: "tcp -h *")
        // let adapter = try communicator.createObjectAdapter("TestAdapter1")
        // var obj = EmptyI()
        // try adapter.add(obj, Ice.stringToIdentity("x"))
        // do {
        //     try adapter.add(obj, Ice.stringToIdentity("x"))
        //     try test(false)
        // } catch is Ice.AlreadyRegisteredException {}

        // do {
        //     try adapter.add(obj, Ice.stringToIdentity(""))
        //     try test(false);
        // } catch let e as Ice.IllegalIdentityException {
        //     try test(e.id.name == "")
        // }

        // do {
        //     try adapter.add(nil, Ice.stringToIdentity("x"))
        //     try test(false)
        // } catch is Ice.IllegalServantException{}

        // try adapter.remove(Ice.stringToIdentity("x"))
        // do {
        //     try adapter.remove(Ice.Util.stringToIdentity("x"));
        //     try test(false)
        // } catch is Ice.NotRegisteredException {}
        // try adapter.deactivate()
        output.writeLine("ok")

        output.write("testing servant locator registration exceptions... ")
        // try communicator.getProperties().setProperty(key: "TestAdapter2.Endpoints", value: "tcp -h *")
        // var adapter = try communicator.createObjectAdapter("TestAdapter2")
        // var loc = ServantLocatorI()
        // adapter.addServantLocator(loc, "x")
        // do {
        //     try adapter.addServantLocator(loc, "x")
        //     try test(false)
        // } catch is Ice.AlreadyRegisteredException {}

        // adapter.deactivate()
        output.writeLine("ok")

        output.write("testing object factory registration exception... ")
        try communicator.getValueFactoryManager().add(factory: { _ in nil }, id: "::x")
        do {
            try communicator.getValueFactoryManager().add(factory: { _ in nil }, id: "::x")
            try test(false)
        } catch is Ice.AlreadyRegisteredException {}
        output.writeLine("ok")

        output.write("testing stringToProxy... ")
        let ref = "thrower:\(self.getTestEndpoint(num: 0))"
        let base = try communicator.stringToProxy(ref)!
        output.writeLine("ok")

        output.write("testing checked cast... ")
        let thrower = try checkedCast(prx: base, type: ThrowerPrx.self)!
        try test(thrower == base)
        output.writeLine("ok")

        output.write("catching exact types... ")
        do {
            try thrower.throwAasA(1)
            try test(false)
        } catch let ex as A {
            try test(ex.aMem == 1)
        } catch {
            output.writeLine("\(error)")
            try test(false)
        }

        do {
            try thrower.throwAorDasAorD(1)
            try test(false)
        } catch let ex as A {
            try test(ex.aMem == 1)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwAorDasAorD(-1)
            try test(false)
        } catch let ex as D {
            try test(ex.dMem == -1)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwBasB(a: 1, b: 2)
            try test(false)
        } catch let ex as B {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwCasC(a: 1, b: 2, c: 3)
            try test(false)
        } catch let ex as C {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
            try test(ex.cMem == 3)
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching base types... ")
        do {
            try thrower.throwBasB(a: 1, b: 2)
            try test(false)
        } catch let ex as A {
            try test(ex.aMem == 1)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwCasC(a: 1, b: 2, c: 3)
            try test(false)
        } catch let ex as B {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching derived types... ")
        do {
            try thrower.throwBasA(a: 1, b: 2)
            try test(false)
        } catch let ex as B {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwCasA(a: 1, b: 2, c: 3)
            try test(false)
        } catch let ex as C {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
            try test(ex.cMem == 3)
        } catch {
            try test(false)
        }

        do {
            try thrower.throwCasB(a: 1, b: 2, c: 3)
            try test(false)
        } catch let ex as C {
            try test(ex.aMem == 1)
            try test(ex.bMem == 2)
            try test(ex.cMem == 3)
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        let supportsUndeclaredExceptions = try thrower.supportsUndeclaredExceptions()
        if supportsUndeclaredExceptions {
            output.write("catching unknown user exception... ")

            do {
                try thrower.throwUndeclaredA(1)
                try test(false)
            } catch is Ice.UnknownUserException {
            } catch {
                try test(false)
            }

            do {
                try thrower.throwUndeclaredB(a: 1, b: 2)
                try test(false)
            } catch is Ice.UnknownUserException {
            } catch {
                try test(false)
            }

            do {
                try thrower.throwUndeclaredC(a: 1, b: 2, c: 3)
                try test(false)
            } catch is Ice.UnknownUserException {
            } catch {
                try test(false)
            }
            output.writeLine("ok")
        }

        let conn = try thrower.ice_getConnection()
        if conn != nil {
            output.write("testing memory limit marshal exception...")
            do {
                _ = try thrower.throwMemoryLimitException([UInt8]())
                try test(false)
            } catch is Ice.MemoryLimitException {
            } catch {
                try test(false)
            }

            do {
                _ = try thrower.throwMemoryLimitException([UInt8](repeating: 0, count: 20 * 1024)) // 20KB
                try test(false)
            } catch is Ice.ConnectionLostException {
            } catch is Ice.UnknownLocalException {
                // Expected with JS bidir server
            } catch {
                try test(false)
            }

            do {
                var str = "thrower:\(self.getTestEndpoint(num: 1))"
                let thrower2 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
                do {
                    // 2MB(no limits)
                    _ = try thrower2.throwMemoryLimitException([UInt8](repeating: 0, count: 2 * 1024 * 1024))
                } catch is Ice.MemoryLimitException {}
                str = "thrower:\(self.getTestEndpoint(num: 2))"
                let thrower3 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
                do {
                    _ = try thrower3.throwMemoryLimitException([UInt8](repeating: 0, count: 1024)) // 1KB limit
                    try test(false)
                } catch is Ice.ConnectionLostException {}
            } catch is Ice.ConnectionRefusedException {
                // Expected with JS bidir server
            }
            output.writeLine("ok")
        }

        output.write("catching object not exist exception... ")
        do {
            let id = try Ice.stringToIdentity("does not exist")
            do {
                let thrower2 = try uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
                try thrower2.ice_ping()
                try test(false)
            } catch let ex as Ice.ObjectNotExistException {
                try test(ex.id == id)
            } catch {
                try test(false)
            }
        }
        output.writeLine("ok")

        output.write("catching facet not exist exception... ")
        do {
            let thrower2 = uncheckedCast(prx: thrower, type: ThrowerPrx.self, facet: "no such facet")
            do {
                try thrower2.ice_ping()
                try test(false)
            } catch let ex as Ice.FacetNotExistException {
                try test(ex.facet == "no such facet")
            }
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching operation not exist exception... ")
        do {
            let thrower2 = uncheckedCast(prx: thrower, type: WrongOperationPrx.self)
            try thrower2.noSuchOperation()
            try test(false)
        } catch let ex as Ice.OperationNotExistException {
            try test(ex.operation == "noSuchOperation")
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching unknown local exception... ")
        do {
            try thrower.throwLocalException()
            try test(false)
        } catch is Ice.UnknownLocalException {
        } catch {
            try test(false)
        }

        do {
            try thrower.throwLocalExceptionIdempotent()
            try test(false)
        } catch is Ice.UnknownLocalException {
        } catch is Ice.OperationNotExistException {
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching unknown non-Ice exception... ")
        do {
            try thrower.throwNonIceException()
            try test(false)
        } catch is Ice.UnknownException {
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("testing asynchronous exceptions... ")
        do {
            try thrower.throwAfterResponse()
        } catch {
            try test(false)
        }

        do {
            try thrower.throwAfterException()
            try test(false)
        } catch is A {
        } catch {
            try test(false)
        }
        output.writeLine("ok")

        output.write("catching exact types with AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwAasAAsync(1)
            }.map {
                try self.test(false)
            }.catch { e in
                do {
                    if let exc = e as? A {
                        try self.test(exc.aMem == 1)
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        for i in [1, -1] {
            try Promise<Void> { seal in
                firstly {
                    thrower.throwAorDasAorDAsync(Int32(i))
                }.map {
                    try self.test(false)
                }.catch { e in
                    do {
                        if let ex = e as? A {
                            try self.test(ex.aMem == 1)
                        } else if let ex = e as? D {
                            try self.test(ex.dMem == -1)
                        } else {
                            try self.test(false)
                        }
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()
        }

        try Promise<Void> { seal in
            firstly {
                thrower.throwBasBAsync(a: 1, b: 2)
            }.map {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? B {
                        try self.test(ex.aMem == 1)
                        try self.test(ex.bMem == 2)
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        try Promise<Void> { seal in
            firstly {
                thrower.throwCasCAsync(a: 1, b: 2, c: 3)
            }.map {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? C {
                        try self.test(ex.aMem == 1)
                        try self.test(ex.bMem == 2)
                        try self.test(ex.cMem == 3)
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

        output.write("catching derived types with new AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwBasAAsync(a: 1, b: 2)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? B {
                        try self.test(ex.aMem == 1)
                        try self.test(ex.bMem == 2)
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        try Promise<Void> { seal in
            firstly {
                thrower.throwCasAAsync(a: 1, b: 2, c: 3)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? C {
                        try self.test(ex.aMem == 1)
                        try self.test(ex.bMem == 2)
                        try self.test(ex.cMem == 3)
                    } else {
                        try self.test(false)
                    }
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        try Promise<Void> { seal in
            firstly {
                thrower.throwCasBAsync(a: 1, b: 2, c: 3)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? C {
                        try self.test(ex.aMem == 1)
                        try self.test(ex.bMem == 2)
                        try self.test(ex.cMem == 3)
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

        if supportsUndeclaredExceptions {
            output.write("catching unknown user exception with new AMI mapping... ")

            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredAAsync(1)
                }.done {
                    try self.test(false)
                }.catch { e in
                    if e is Ice.UnknownUserException {
                        seal.fulfill(())
                    } else {
                        seal.reject(e)
                    }
                }
            }.wait()

            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredBAsync(a: 1, b: 2)
                }.done {
                    try self.test(false)
                }.catch { e in
                    if e is Ice.UnknownUserException {
                        seal.fulfill(())
                    } else {
                        seal.reject(e)
                    }
                }
            }.wait()

            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredCAsync(a: 1, b: 2, c: 3)
                }.done {
                    try self.test(false)
                }.catch { e in
                    if e is Ice.UnknownUserException {
                        seal.fulfill(())
                    } else {
                        seal.reject(e)
                    }
                }
            }.wait()

            output.writeLine("ok")
        }

        output.write("catching object not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let  id = try Ice.stringToIdentity("does not exist")
            let thrower2 = try uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
            firstly {
                thrower2.throwAasAAsync(1)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.ObjectNotExistException {
                        try self.test(ex.id == id)
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

        output.write("catching facet not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let thrower2 = uncheckedCast(prx: thrower, type: ThrowerPrx.self, facet: "no such facet")
            firstly {
                thrower2.throwAasAAsync(1)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.FacetNotExistException {
                        try self.test(ex.facet == "no such facet")
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

        output.write("catching operation not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let thrower4 = uncheckedCast(prx: thrower, type: WrongOperationPrx.self)
            firstly {
                thrower4.noSuchOperationAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.OperationNotExistException {
                        try self.test(ex.operation == "noSuchOperation")
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

        output.write("catching unknown local exception with new AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwLocalExceptionAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        try Promise<Void> { seal in
            firstly {
                thrower.throwLocalExceptionIdempotentAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("catching unknown non-Ice exception with new AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwNonIceExceptionAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        if supportsUndeclaredExceptions {
            output.write("catching unknown user exception with new AMI mapping... ")
            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredAAsync(1)
                }.done {
                    try self.test(false)
                }.catch { e in
                    do {
                        try self.test(e is Ice.UnknownUserException)
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()

            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredBAsync(a: 1, b: 2)
                }.done {
                    try self.test(false)
                }.catch { e in
                    do {
                        try self.test(e is Ice.UnknownUserException)
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()

            try Promise<Void> { seal in
                firstly {
                    thrower.throwUndeclaredCAsync(a: 1, b: 2, c: 3)
                }.done {
                    try self.test(false)
                }.catch { e in
                    do {
                        try self.test( e is Ice.UnknownUserException)
                        seal.fulfill(())
                    } catch {
                        seal.reject(error)
                    }
                }
            }.wait()
        }
        output.writeLine("ok")

        output.write("catching object not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let id = try Ice.stringToIdentity("does not exist")
            let thrower2 = try uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
            firstly {
                thrower2.throwAasAAsync(1)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.ObjectNotExistException {
                        try self.test(ex.id == id)
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

        output.write("catching facet not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let thrower2 = uncheckedCast(prx: thrower, type: ThrowerPrx.self, facet: "no such facet")
            firstly {
                thrower2.throwAasAAsync(1)
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.FacetNotExistException {
                        try self.test(ex.facet == "no such facet")
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

        output.write("catching operation not exist exception with new AMI mapping... ")
        try Promise<Void> { seal in
            let thrower4 = uncheckedCast(prx: thrower, type: WrongOperationPrx.self)
            firstly {
                thrower4.noSuchOperationAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    if let ex = e as? Ice.OperationNotExistException {
                        try self.test(ex.operation == "noSuchOperation")
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

        output.write("catching unknown local exception with new AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwLocalExceptionAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()

        try Promise<Void> { seal in
            firstly {
                thrower.throwLocalExceptionIdempotentAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        output.write("catching unknown non-Ice exception with new AMI mapping... ")
        try Promise<Void> { seal in
            firstly {
                thrower.throwNonIceExceptionAsync()
            }.done {
                try self.test(false)
            }.catch { e in
                do {
                    try self.test(e is Ice.UnknownException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")

        try thrower.shutdown()
    }
}
