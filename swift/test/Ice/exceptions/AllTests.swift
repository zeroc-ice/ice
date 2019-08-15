//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class ServantLocatorI: Ice.ServantLocator {
    func locate(_: Current) throws -> (returnValue: Disp?, cookie: AnyObject?) {
        return (nil, nil)
    }

    func finished(curr _: Current, servant _: Disp, cookie _: AnyObject?) throws {}

    func deactivate(_: String) {}
}

func allTests(_ helper: TestHelper) throws -> ThrowerPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing object adapter registration exceptions... ")
    do {
        _ = try communicator.createObjectAdapter("TestAdapter0")
    } catch is Ice.InitializationException {}

    communicator.getProperties().setProperty(key: "TestAdapter0.Endpoints", value: "tcp -h *")
    let first = try communicator.createObjectAdapter("TestAdapter0")
    do {
        _ = try communicator.createObjectAdapter("TestAdapter0")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    do {
        _ = try communicator.createObjectAdapterWithEndpoints(name: "TestAdapter0", endpoints: "ssl -h foo -p 12011")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}
    first.deactivate()
    output.writeLine("ok")

    output.write("testing servant registration exceptions... ")
    communicator.getProperties().setProperty(key: "TestAdapter1.Endpoints", value: "tcp -h *")
    var adapter = try communicator.createObjectAdapter("TestAdapter1")
    let obj = EmptyI()
    _ = try adapter.add(servant: EmptyDisp(obj), id: Ice.stringToIdentity("x"))
    do {
        _ = try adapter.add(servant: EmptyDisp(obj), id: Ice.stringToIdentity("x"))
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    _ = try adapter.remove(Ice.stringToIdentity("x"))
    do {
        _ = try adapter.remove(Ice.stringToIdentity("x"))
        try test(false)
    } catch is Ice.NotRegisteredException {}
    adapter.deactivate()
    output.writeLine("ok")

    output.write("testing servant locator registration exceptions... ")
    communicator.getProperties().setProperty(key: "TestAdapter2.Endpoints", value: "tcp -h *")
    adapter = try communicator.createObjectAdapter("TestAdapter2")
    try adapter.addServantLocator(locator: ServantLocatorI(), category: "x")
    do {
        try adapter.addServantLocator(locator: ServantLocatorI(), category: "x")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

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
    let ref = "thrower:\(helper.getTestEndpoint(num: 0))"
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

    do {
        try thrower.throwModA(a: 1, a2: 2)
        try test(false)
    } catch let ex as ModA {
        try test(ex.aMem == 1)
        try test(ex.a2Mem == 2)
    } catch is Ice.OperationNotExistException {
        //
        // This operation is not supported in Java.
        //
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

    do {
        try thrower.throwModA(a: 1, a2: 2)
        try test(false)
    } catch let ex as ModA {
        try test(ex.aMem == 1)
        try test(ex.a2Mem == 2)
    } catch is Ice.OperationNotExistException {
        //
        // This operation is not supported in Java.
        //
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
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }

        do {
            try thrower.throwUndeclaredB(a: 1, b: 2)
            try test(false)
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }

        do {
            try thrower.throwUndeclaredC(a: 1, b: 2, c: 3)
            try test(false)
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }
        output.writeLine("ok")
    }

    let conn = try thrower.ice_getConnection()
    if conn != nil {
        output.write("testing memory limit marshal exception...")
        do {
            _ = try thrower.throwMemoryLimitException(ByteSeq())
            try test(false)
        } catch is Ice.MemoryLimitException {} catch {
            try test(false)
        }

        do {
            _ = try thrower.throwMemoryLimitException(ByteSeq(repeating: 0, count: 20 * 1024)) // 20KB
            try test(false)
        } catch is Ice.ConnectionLostException {} catch is Ice.UnknownLocalException {
            // Expected with JS bidir server
        } catch {
            try test(false)
        }

        do {
            var str = "thrower:\(helper.getTestEndpoint(num: 1))"
            let thrower2 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
            do {
                // 2MB(no limits)
                _ = try thrower2.throwMemoryLimitException(ByteSeq(repeating: 0, count: 2 * 1024 * 1024))
            } catch is Ice.MemoryLimitException {}
            str = "thrower:\(helper.getTestEndpoint(num: 2))"
            let thrower3 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
            do {
                _ = try thrower3.throwMemoryLimitException(ByteSeq(repeating: 0, count: 1024)) // 1KB limit
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
            let thrower2 = uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
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
    } catch is Ice.UnknownLocalException {} catch {
        try test(false)
    }

    do {
        try thrower.throwLocalExceptionIdempotent()
        try test(false)
    } catch is Ice.UnknownLocalException {} catch is Ice.OperationNotExistException {} catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching unknown non-Ice exception... ")
    do {
        try thrower.throwNonIceException()
        try test(false)
    } catch is Ice.UnknownException {} catch {
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
    } catch is A {} catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching exact types with AMI mapping... ")
    try Promise<Void> { seal in
        firstly {
            thrower.throwAasAAsync(1)
        }.map {
            try test(false)
        }.catch { e in
            do {
                if let exc = e as? A {
                    try test(exc.aMem == 1)
                } else {
                    try test(false)
                }
                seal.fulfill(())
            } catch {
                seal.reject(error)
            }
        }
    }.wait()

    try Promise<Void> { seal in
        firstly {
            thrower.throwModAAsync(a: 1, a2: 2)
        }.map {
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? ModA {
                    try test(ex.aMem == 1)
                    try test(ex.a2Mem == 2)
                } else if e is Ice.OperationNotExistException {
                    //
                    // This operation is not supported in Java.
                    //
                } else {
                    try test(false)
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
                try test(false)
            }.catch { e in
                do {
                    if let ex = e as? A {
                        try test(ex.aMem == 1)
                    } else if let ex = e as? D {
                        try test(ex.dMem == -1)
                    } else {
                        try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? B {
                    try test(ex.aMem == 1)
                    try test(ex.bMem == 2)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? C {
                    try test(ex.aMem == 1)
                    try test(ex.bMem == 2)
                    try test(ex.cMem == 3)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? B {
                    try test(ex.aMem == 1)
                    try test(ex.bMem == 2)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? C {
                    try test(ex.aMem == 1)
                    try test(ex.bMem == 2)
                    try test(ex.cMem == 3)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? C {
                    try test(ex.aMem == 1)
                    try test(ex.bMem == 2)
                    try test(ex.cMem == 3)
                } else {
                    try test(false)
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
                try test(false)
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
                try test(false)
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
                try test(false)
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
        let id = try Ice.stringToIdentity("does not exist")
        let thrower2 = uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
        firstly {
            thrower2.throwAasAAsync(1)
        }.done {
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.ObjectNotExistException {
                    try test(ex.id == id)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.FacetNotExistException {
                    try test(ex.facet == "no such facet")
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.OperationNotExistException {
                    try test(ex.operation == "noSuchOperation")
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownException)
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
                try test(false)
            }.catch { e in
                do {
                    try test(e is Ice.UnknownUserException)
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
                try test(false)
            }.catch { e in
                do {
                    try test(e is Ice.UnknownUserException)
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
                try test(false)
            }.catch { e in
                do {
                    try test(e is Ice.UnknownUserException)
                    seal.fulfill(())
                } catch {
                    seal.reject(error)
                }
            }
        }.wait()
        output.writeLine("ok")
    }

    output.write("catching object not exist exception with new AMI mapping... ")
    try Promise<Void> { seal in
        let id = try Ice.stringToIdentity("does not exist")
        let thrower2 = uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
        firstly {
            thrower2.throwAasAAsync(1)
        }.done {
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.ObjectNotExistException {
                    try test(ex.id == id)
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.FacetNotExistException {
                    try test(ex.facet == "no such facet")
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                if let ex = e as? Ice.OperationNotExistException {
                    try test(ex.operation == "noSuchOperation")
                } else {
                    try test(false)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownLocalException || e is Ice.OperationNotExistException)
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
            try test(false)
        }.catch { e in
            do {
                try test(e is Ice.UnknownException)
                seal.fulfill(())
            } catch {
                seal.reject(error)
            }
        }
    }.wait()
    output.writeLine("ok")
    return thrower
}
