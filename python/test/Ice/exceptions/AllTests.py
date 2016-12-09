# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading, sys, array

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class EmptyI(Test.Empty):
    pass

class ServantLocatorI(Ice.ServantLocator):
    def locate(self, current):
        return None

    def finished(self, current, servant, cookie):
        pass

    def deactivate(self, category):
        pass

def ValueFactory(type):
    return None

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

class Callback(CallbackBase):
    def __init__(self, communicator=None):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def response(self):
        test(False)

    def exception_AasA(self, ex):
        test(isinstance(ex, Test.A))
        test(ex.aMem == 1)
        self.called()

    def exception_AorDasAorD(self, ex):
        try:
            raise ex
        except Test.A as ex:
            test(ex.aMem == 1)
        except Test.D as ex:
            test(ex.dMem == -1)
        except:
            test(False)
        self.called()

    def exception_BasB(self, ex):
        try:
            raise ex
        except Test.B as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

    def exception_CasC(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

    def exception_ModA(self, ex):
        try:
            raise ex
        except Test.Mod.A as ex:
            test(ex.aMem == 1)
            test(ex.a2Mem == 2)
        except Ice.OperationNotExistException:
            #
            # This operation is not supported in Java.
            #
            pass
        except:
            test(False)
        self.called()

    def exception_BasA(self, ex):
        try:
            raise ex
        except Test.B as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

    def exception_CasA(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

    def exception_CasB(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

    def exception_UndeclaredA(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

    def exception_UndeclaredB(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

    def exception_UndeclaredC(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

    def exception_AasAObjectNotExist(self, ex):
        try:
            raise ex
        except Ice.ObjectNotExistException as ex:
            id = Ice.stringToIdentity("does not exist")
            test(ex.id == id)
        except:
            test(False)
        self.called()

    def exception_AasAFacetNotExist(self, ex):
        try:
            raise ex
        except Ice.FacetNotExistException as ex:
            test(ex.facet == "no such facet")
        except:
            test(False)
        self.called()

    def exception_noSuchOperation(self, ex):
        try:
            raise ex
        except Ice.OperationNotExistException as ex:
            test(ex.operation == "noSuchOperation")
        except:
            test(False)
        self.called()

    def exception_LocalException(self, ex):
        try:
            raise ex
        except Ice.UnknownLocalException as ex:
            pass
        except Ice.OperationNotExistException as ex:
            pass
        except:
            test(False)
        self.called()

    def exception_NonIceException(self, ex):
        try:
            raise ex
        except Ice.UnknownException as ex:
            pass
        except:
            test(False)
        self.called()

def allTests(communicator):
    sys.stdout.write("testing servant registration exceptions... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default")
    adapter = communicator.createObjectAdapter("TestAdapter1")
    obj = EmptyI()
    adapter.add(obj, Ice.stringToIdentity("x"))
    try:
        adapter.add(obj, Ice.stringToIdentity("x"))
        test(false)
    except Ice.AlreadyRegisteredException:
        pass

    try:
        adapter.add(obj, Ice.stringToIdentity(""))
        test(false)
    except Ice.IllegalIdentityException as ex:
        test(ex.id.name == "")

    try:
        adapter.add(None, Ice.stringToIdentity("x"))
        test(false)
    except Ice.IllegalServantException:
        pass


    adapter.remove(Ice.stringToIdentity("x"))
    try:
        adapter.remove(Ice.stringToIdentity("x"))
        test(false)
    except Ice.NotRegisteredException:
        pass

    adapter.deactivate()
    print("ok")

    sys.stdout.write("testing servant locator registrations exceptions... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("TestAdapter2.Endpoints", "default")
    adapter = communicator.createObjectAdapter("TestAdapter2")
    loc = ServantLocatorI()
    adapter.addServantLocator(loc, "x")
    try:
        adapter.addServantLocator(loc, "x")
        test(false)
    except Ice.AlreadyRegisteredException:
        pass

    adapter.deactivate()
    print("ok")

    sys.stdout.write("testing object factory registration exception... ")
    sys.stdout.flush()

    communicator.getValueFactoryManager().add(ValueFactory, "x")
    try:
        communicator.getValueFactoryManager().add(ValueFactory, "x")
        test(false)
    except Ice.AlreadyRegisteredException:
        pass
    print("ok")

    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "thrower:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    thrower = Test.ThrowerPrx.checkedCast(base)
    test(thrower)
    test(thrower == base)
    print("ok")

    sys.stdout.write("catching exact types... ")
    sys.stdout.flush()

    try:
        thrower.throwAasA(1)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorD(1)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorD(-1)
        test(False)
    except Test.D as ex:
        test(ex.dMem == -1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwBasB(1, 2)
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwModA(1, 2)
        test(False)
    except Test.Mod.A as ex:
        test(ex.aMem == 1)
        test(ex.a2Mem == 2)
    except Ice.OperationNotExistException:
        #
        # This operation is not supported in Java.
        #
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching base types... ")
    sys.stdout.flush()

    try:
        thrower.throwBasB(1, 2)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwModA(1, 2)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except Ice.OperationNotExistException:
        #
        # This operation is not supported in Java.
        #
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching derived types... ")
    sys.stdout.flush()

    try:
        thrower.throwBasA(1, 2)
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasA(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasB(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    if thrower.supportsUndeclaredExceptions():
        sys.stdout.write("catching unknown user exception... ")
        sys.stdout.flush()

        try:
            thrower.throwUndeclaredA(1)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredB(1, 2)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredC(1, 2, 3)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        print("ok")


    if thrower.ice_getConnection():
        sys.stdout.write("testing memory limit marshal exception...");
        sys.stdout.flush();

        try:
            thrower.throwMemoryLimitException(array.array('B'));
            test(False)
        except Ice.MemoryLimitException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwMemoryLimitException(bytearray(20 * 1024)) # 20KB
            test(False)
        except Ice.ConnectionLostException:
            pass
        except:
            test(False)

        print("ok");

    sys.stdout.write("catching object not exist exception... ")
    sys.stdout.flush()

    id = Ice.stringToIdentity("does not exist")
    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
        thrower2.throwAasA(1)
#        thrower2.ice_ping()
        test(False)
    except Ice.ObjectNotExistException as ex:
        test(ex.id == id)
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching facet not exist exception... ")
    sys.stdout.flush()

    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
        try:
            thrower2.ice_ping()
            test(False)
        except Ice.FacetNotExistException as ex:
            test(ex.facet == "no such facet")
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching operation not exist exception... ")
    sys.stdout.flush()

    try:
        thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower)
        thrower2.noSuchOperation()
        test(False)
    except Ice.OperationNotExistException as ex:
        test(ex.operation == "noSuchOperation")
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching unknown local exception... ")
    sys.stdout.flush()

    try:
        thrower.throwLocalException()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except:
        print(sys.exc_info())
        test(False)
    try:
        thrower.throwLocalExceptionIdempotent()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except Ice.OperationNotExistException:
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching unknown non-Ice exception... ")
    sys.stdout.flush()

    try:
        thrower.throwNonIceException()
        test(False)
    except Ice.UnknownException:
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("testing asynchronous exceptions... ")
    sys.stdout.flush()

    try:
        thrower.throwAfterResponse()
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAfterException()
        test(False)
    except Test.A:
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching exact types with futures... ")
    sys.stdout.flush()

    try:
        thrower.throwAasAAsync(1).result()
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorDAsync(1).result()
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorDAsync(-1).result()
        test(False)
    except Test.D as ex:
        test(ex.dMem == -1)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwBasBAsync(1, 2).result()
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasCAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwModAAsync(1, 2).result()
        test(False)
    except Test.Mod.A as ex:
        test(ex.aMem == 1)
        test(ex.a2Mem == 2)
    except Ice.OperationNotExistException:
        #
        # This operation is not supported in Java.
        #
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching derived types with futures... ")
    sys.stdout.flush()

    try:
        thrower.throwBasAAsync(1, 2).result()
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasAAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasBAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    if thrower.supportsUndeclaredExceptions():
        sys.stdout.write("catching unknown user exception with futures... ")
        sys.stdout.flush()

        try:
            thrower.throwUndeclaredAAsync(1).result()
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredBAsync(1, 2).result()
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredCAsync(1, 2, 3).result()
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print(sys.exc_info())
            test(False)

        print("ok")

    sys.stdout.write("catching object not exist exception with futures... ")
    sys.stdout.flush()

    id = Ice.stringToIdentity("does not exist")
    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
        thrower2.throwAasAAsync(1).result()
#        thrower2.ice_ping()
        test(False)
    except Ice.ObjectNotExistException as ex:
        test(ex.id == id)
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching facet not exist exception with futures... ")
    sys.stdout.flush()

    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
        try:
            thrower2.ice_pingAsync().result()
            test(False)
        except Ice.FacetNotExistException as ex:
            test(ex.facet == "no such facet")
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching operation not exist exception with futures... ")
    sys.stdout.flush()

    try:
        thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower)
        thrower2.noSuchOperationAsync().result()
        test(False)
    except Ice.OperationNotExistException as ex:
        test(ex.operation == "noSuchOperation")
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching unknown local exception with futures... ")
    sys.stdout.flush()

    try:
        thrower.throwLocalExceptionAsync().result()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except:
        print(sys.exc_info())
        test(False)
    try:
        thrower.throwLocalExceptionIdempotentAsync().result()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except Ice.OperationNotExistException:
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching unknown non-Ice exception with futures... ")
    sys.stdout.flush()

    try:
        thrower.throwNonIceExceptionAsync().result()
        test(False)
    except Ice.UnknownException:
        pass
    except:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching exact types with AMI mapping... ")
    sys.stdout.flush()

    cb = Callback()
    thrower.begin_throwAasA(1, cb.response, cb.exception_AasA)
    cb.check()

    cb = Callback()
    thrower.begin_throwAorDasAorD(1, cb.response, cb.exception_AorDasAorD)
    cb.check()

    cb = Callback()
    thrower.begin_throwAorDasAorD(-1, cb.response, cb.exception_AorDasAorD)
    cb.check()

    cb = Callback()
    thrower.begin_throwBasB(1, 2, cb.response, cb.exception_BasB)
    cb.check()

    cb = Callback()
    thrower.begin_throwCasC(1, 2, 3, cb.response, cb.exception_CasC)
    cb.check()

    cb = Callback()
    thrower.begin_throwModA(1, 2, cb.response, cb.exception_ModA)
    cb.check()

    print("ok")

    sys.stdout.write("catching derived types with AMI mapping... ")
    sys.stdout.flush()

    cb = Callback()
    thrower.begin_throwBasA(1, 2, cb.response, cb.exception_BasA)
    cb.check()

    cb = Callback()
    thrower.begin_throwCasA(1, 2, 3, cb.response, cb.exception_CasA)
    cb.check()

    cb = Callback()
    thrower.begin_throwCasB(1, 2, 3, cb.response, cb.exception_CasB)
    cb.check()

    print("ok")

    if thrower.supportsUndeclaredExceptions():
        sys.stdout.write("catching unknown user exception with AMI mapping... ")
        sys.stdout.flush()

        cb = Callback()
        thrower.begin_throwUndeclaredA(1, cb.response, cb.exception_UndeclaredA)
        cb.check()

        cb = Callback()
        thrower.begin_throwUndeclaredB(1, 2, cb.response, cb.exception_UndeclaredB)
        cb.check()

        cb = Callback()
        thrower.begin_throwUndeclaredC(1, 2, 3, cb.response, cb.exception_UndeclaredC)
        cb.check()

        print("ok")

    sys.stdout.write("catching object not exist exception with AMI mapping... ")
    sys.stdout.flush()

    id = Ice.stringToIdentity("does not exist")
    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
    cb = Callback(communicator)
    thrower2.begin_throwAasA(1, cb.response, cb.exception_AasAObjectNotExist)
    cb.check()

    print("ok")

    sys.stdout.write("catching facet not exist exception with AMI mapping... ")
    sys.stdout.flush()

    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
    cb = Callback()
    thrower2.begin_throwAasA(1, cb.response, cb.exception_AasAFacetNotExist)
    cb.check()

    print("ok")

    sys.stdout.write("catching operation not exist exception with AMI mapping... ")
    sys.stdout.flush()

    cb = Callback()
    thrower4 = Test.WrongOperationPrx.uncheckedCast(thrower)
    thrower4.begin_noSuchOperation(cb.response, cb.exception_noSuchOperation)
    cb.check()

    print("ok")

    sys.stdout.write("catching unknown local exception with AMI mapping... ")
    sys.stdout.flush()

    cb = Callback()
    thrower.begin_throwLocalException(cb.response, cb.exception_LocalException)
    cb.check()

    cb = Callback()
    thrower.begin_throwLocalExceptionIdempotent(cb.response, cb.exception_LocalException)
    cb.check()

    print("ok")

    sys.stdout.write("catching unknown non-Ice exception with AMI mapping... ")
    sys.stdout.flush()

    cb = Callback()
    thrower.begin_throwNonIceException(cb.response, cb.exception_NonIceException)
    cb.check()

    print("ok")

    return thrower
