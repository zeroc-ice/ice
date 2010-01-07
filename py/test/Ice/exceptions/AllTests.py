# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading, sys

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

class ObjectFactoryI(Ice.ObjectFactory):
    def create(id):
        return None

    def destroy():
        pass

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        self._cond.acquire()
        try:
            while not self._called:
                self._cond.wait()
            self._called = False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

class AMI_Thrower_throwAasAI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.A, ex:
            test(ex.aMem == 1)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwAasAObjectNotExistI(CallbackBase):
    def __init__(self, communicator):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.ObjectNotExistException, ex:
            id = self._communicator.stringToIdentity("does not exist")
            test(ex.id == id)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwAasAFacetNotExistI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.FacetNotExistException, ex:
            test(ex.facet == "no such facet")
        except:
            test(False)
        self.called()

class AMI_Thrower_throwAorDasAorDI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.A, ex:
            test(ex.aMem == 1)
        except Test.D, ex:
            test(ex.dMem == -1)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwBasAI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.B, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwCasAI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.C, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwBasBI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.B, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwCasBI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.C, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwCasCI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.C, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

class AMI_Thrower_throwModAI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Test.Mod.A, ex:
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

class AMI_Thrower_throwUndeclaredAI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

class AMI_Thrower_throwUndeclaredBI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

class AMI_Thrower_throwUndeclaredCI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except:
            test(False)
        self.called()

class AMI_Thrower_throwLocalExceptionI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.UnknownLocalException:
            pass
        except:
            test(False)
        self.called()

class AMI_Thrower_throwNonIceExceptionI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.UnknownException:
            pass
        except:
            test(False)
        self.called()

class AMI_WrongOperation_noSuchOperationI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        try:
            raise ex
        except Ice.OperationNotExistException, ex:
            test(ex.operation == "noSuchOperation")
        except:
            test(False)
        self.called()

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
        except Test.A, ex:
            test(ex.aMem == 1)
        except Test.D, ex:
            test(ex.dMem == -1)
        except:
            test(False)
        self.called()

    def exception_BasB(self, ex):
        try:
            raise ex
        except Test.B, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

    def exception_CasC(self, ex):
        try:
            raise ex
        except Test.C, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

    def exception_ModA(self, ex):
        try:
            raise ex
        except Test.Mod.A, ex:
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
        except Test.B, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except:
            test(False)
        self.called()

    def exception_CasA(self, ex):
        try:
            raise ex
        except Test.C, ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except:
            test(False)
        self.called()

    def exception_CasB(self, ex):
        try:
            raise ex
        except Test.C, ex:
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
        except Ice.ObjectNotExistException, ex:
            id = self._communicator.stringToIdentity("does not exist")
            test(ex.id == id)
        except:
            test(False)
        self.called()

    def exception_AasAFacetNotExist(self, ex):
        try:
            raise ex
        except Ice.FacetNotExistException, ex:
            test(ex.facet == "no such facet")
        except:
            test(False)
        self.called()

    def exception_noSuchOperation(self, ex):
        try:
            raise ex
        except Ice.OperationNotExistException, ex:
            test(ex.operation == "noSuchOperation")
        except:
            test(False)
        self.called()

    def exception_LocalException(self, ex):
        try:
            raise ex
        except Ice.UnknownLocalException, ex:
            pass
        except:
            test(False)
        self.called()

    def exception_NonIceException(self, ex):
        try:
            raise ex
        except Ice.UnknownException, ex:
            pass
        except:
            test(False)
        self.called()

def allTests(communicator):
    print "testing servant registration exceptions...",
    communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default")
    adapter = communicator.createObjectAdapter("TestAdapter1")
    obj = EmptyI()
    adapter.add(obj, communicator.stringToIdentity("x"))
    try:
        adapter.add(obj, communicator.stringToIdentity("x"))
        test(false)
    except Ice.AlreadyRegisteredException:
        pass

    adapter.remove(communicator.stringToIdentity("x"))
    try:
        adapter.remove(communicator.stringToIdentity("x"))
        test(false)
    except Ice.NotRegisteredException:
        pass

    adapter.deactivate()
    print "ok"

    print "testing servant locator registrations exceptions...",
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
    print "ok"

    print "testing object factory registration exception...",
    of = ObjectFactoryI()
    communicator.addObjectFactory(of, "x")
    try:
        communicator.addObjectFactory(of, "x")
        test(false)
    except Ice.AlreadyRegisteredException:
        pass
    print "ok"

    print "testing stringToProxy...",
    ref = "thrower:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)
    print "ok"

    print "testing checked cast...",
    thrower = Test.ThrowerPrx.checkedCast(base)
    test(thrower)
    test(thrower == base)
    print "ok"

    print "catching exact types...",

    try:
        thrower.throwAasA(1)
        test(False)
    except Test.A, ex:
        test(ex.aMem == 1)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwAorDasAorD(1)
        test(False)
    except Test.A, ex:
        test(ex.aMem == 1)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwAorDasAorD(-1)
        test(False)
    except Test.D, ex:
        test(ex.dMem == -1)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwBasB(1, 2)
        test(False)
    except Test.B, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.C, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwModA(1, 2)
        test(False)
    except Test.Mod.A, ex:
        test(ex.aMem == 1)
        test(ex.a2Mem == 2)
    except Ice.OperationNotExistException:
        #
        # This operation is not supported in Java.
        #
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching base types...",

    try:
        thrower.throwBasB(1, 2)
        test(False)
    except Test.A, ex:
        test(ex.aMem == 1)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.B, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwModA(1, 2)
        test(False)
    except Test.A, ex:
        test(ex.aMem == 1)
    except Ice.OperationNotExistException:
        #
        # This operation is not supported in Java.
        #
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching derived types...",

    try:
        thrower.throwBasA(1, 2)
        test(False)
    except Test.B, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwCasA(1, 2, 3)
        test(False)
    except Test.C, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print sys.exc_info()
        test(False)

    try:
        thrower.throwCasB(1, 2, 3)
        test(False)
    except Test.C, ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    if thrower.supportsUndeclaredExceptions():
        print "catching unknown user exception...",

        try:
            thrower.throwUndeclaredA(1)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print sys.exc_info()
            test(False)

        try:
            thrower.throwUndeclaredB(1, 2)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print sys.exc_info()
            test(False)

        try:
            thrower.throwUndeclaredC(1, 2, 3)
            test(False)
        except Ice.UnknownUserException:
            pass
        except:
            print sys.exc_info()
            test(False)

        print "ok"

    print "catching object not exist exception...",

    id = communicator.stringToIdentity("does not exist")
    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
        thrower2.throwAasA(1)
#        thrower2.ice_ping()
        test(False)
    except Ice.ObjectNotExistException, ex:
        test(ex.id == id)
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching facet not exist exception...",

    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
        try:
            thrower2.ice_ping()
            test(False)
        except Ice.FacetNotExistException, ex:
            test(ex.facet == "no such facet")
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching operation not exist exception...",

    try:
        thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower)
        thrower2.noSuchOperation()
        test(False)
    except Ice.OperationNotExistException, ex:
        test(ex.operation == "noSuchOperation")
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching unknown local exception...",

    try:
        thrower.throwLocalException()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching unknown non-Ice exception...",

    try:
        thrower.throwNonIceException()
        test(False)
    except Ice.UnknownException:
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching exact types with AMI...",

    cb = AMI_Thrower_throwAasAI()
    thrower.throwAasA_async(cb, 1)
    cb.check()
    # Let's check if we can reuse the same callback object for another call.
    thrower.throwAasA_async(cb, 1)
    cb.check()

    cb = AMI_Thrower_throwAorDasAorDI()
    thrower.throwAorDasAorD_async(cb, 1)
    cb.check()

    cb = AMI_Thrower_throwAorDasAorDI()
    thrower.throwAorDasAorD_async(cb, -1)
    cb.check()

    cb = AMI_Thrower_throwBasBI()
    thrower.throwBasB_async(cb, 1, 2)
    cb.check()

    cb = AMI_Thrower_throwCasCI()
    thrower.throwCasC_async(cb, 1, 2, 3)
    cb.check()
    # Let's check if we can reuse the same callback object for another call.
    thrower.throwCasC_async(cb, 1, 2, 3)
    cb.check()

    cb = AMI_Thrower_throwModAI()
    thrower.throwModA_async(cb, 1, 2)
    cb.check()

    print "ok"

    print "catching derived types...",

    cb = AMI_Thrower_throwBasAI()
    thrower.throwBasA_async(cb, 1, 2)
    cb.check()

    cb = AMI_Thrower_throwCasAI()
    thrower.throwCasA_async(cb, 1, 2, 3)
    cb.check()

    cb = AMI_Thrower_throwCasBI()
    thrower.throwCasB_async(cb, 1, 2, 3)
    cb.check()

    print "ok"

    if thrower.supportsUndeclaredExceptions():
        print "catching unknown user exception with AMI...",

        cb = AMI_Thrower_throwUndeclaredAI()
        thrower.throwUndeclaredA_async(cb, 1)
        cb.check()

        cb = AMI_Thrower_throwUndeclaredBI()
        thrower.throwUndeclaredB_async(cb, 1, 2)
        cb.check()

        cb = AMI_Thrower_throwUndeclaredCI()
        thrower.throwUndeclaredC_async(cb, 1, 2, 3)
        cb.check()

        print "ok"

    print "catching object not exist exception with AMI...",

    id = communicator.stringToIdentity("does not exist")
    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
    cb = AMI_Thrower_throwAasAObjectNotExistI(communicator)
    thrower2.throwAasA_async(cb, 1)
    cb.check()

    print "ok"

    print "catching facet not exist exception with AMI...",

    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
    cb = AMI_Thrower_throwAasAFacetNotExistI()
    thrower2.throwAasA_async(cb, 1)
    cb.check()

    print "ok"

    print "catching operation not exist exception with AMI...",

    cb = AMI_WrongOperation_noSuchOperationI()
    thrower4 = Test.WrongOperationPrx.uncheckedCast(thrower)
    thrower4.noSuchOperation_async(cb)
    cb.check()

    print "ok"

    print "catching unknown local exception with AMI...",

    cb = AMI_Thrower_throwLocalExceptionI()
    thrower.throwLocalException_async(cb)
    cb.check()

    print "ok"

    print "catching unknown non-Ice exception with AMI...",

    cb = AMI_Thrower_throwNonIceExceptionI()
    thrower.throwNonIceException_async(cb)
    cb.check()

    print "ok"

    print "catching exact types with new AMI mapping...",

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

    print "ok"

    print "catching derived types with new AMI mapping...",

    cb = Callback()
    thrower.begin_throwBasA(1, 2, cb.response, cb.exception_BasA)
    cb.check()

    cb = Callback()
    thrower.begin_throwCasA(1, 2, 3, cb.response, cb.exception_CasA)
    cb.check()

    cb = Callback()
    thrower.begin_throwCasB(1, 2, 3, cb.response, cb.exception_CasB)
    cb.check()

    print "ok"

    if thrower.supportsUndeclaredExceptions():
        print "catching unknown user exception with new AMI mapping...",

        cb = Callback()
        thrower.begin_throwUndeclaredA(1, cb.response, cb.exception_UndeclaredA)
        cb.check()

        cb = Callback()
        thrower.begin_throwUndeclaredB(1, 2, cb.response, cb.exception_UndeclaredB)
        cb.check()

        cb = Callback()
        thrower.begin_throwUndeclaredC(1, 2, 3, cb.response, cb.exception_UndeclaredC)
        cb.check()

        print "ok"

    print "catching object not exist exception with new AMI mapping...",

    id = communicator.stringToIdentity("does not exist")
    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id))
    cb = Callback(communicator)
    thrower2.begin_throwAasA(1, cb.response, cb.exception_AasAObjectNotExist)
    cb.check()

    print "ok"

    print "catching facet not exist exception with new AMI mapping...",

    thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet")
    cb = Callback()
    thrower2.begin_throwAasA(1, cb.response, cb.exception_AasAFacetNotExist)
    cb.check()

    print "ok"

    print "catching operation not exist exception with new AMI mapping...",

    cb = Callback()
    thrower4 = Test.WrongOperationPrx.uncheckedCast(thrower)
    thrower4.begin_noSuchOperation(cb.response, cb.exception_noSuchOperation)
    cb.check()

    print "ok"

    print "catching unknown local exception with new AMI mapping...",

    cb = Callback()
    thrower.begin_throwLocalException(cb.response, cb.exception_LocalException)
    cb.check()

    print "ok"

    print "catching unknown non-Ice exception with new AMI mapping...",

    cb = Callback()
    thrower.begin_throwNonIceException(cb.response, cb.exception_NonIceException)
    cb.check()

    print "ok"

    return thrower
