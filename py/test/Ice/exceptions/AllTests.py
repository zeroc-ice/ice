# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, threading, sys

Ice.loadSlice('Test.ice')
import Test

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
                self._cond.wait(5.0)
            if self._called:
                self._called = false
                return True;
            else:
                return False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        _called = True
        self._cond.notify()
        self._cond.release()

def allTests(communicator):
    print "testing servant registration exceptions... ",
    adapter = communicator.createObjectAdapter("TestAdapter1")
    obj = EmptyI()
    adapter.add(obj, Ice.stringToIdentity("x"))
    gotException = False
    try:
        adapter.add(obj, Ice.stringToIdentity("x"))
    except Ice.AlreadyRegisteredException:
        gotException = True
    test(gotException)

    gotException = False
    adapter.remove(Ice.stringToIdentity("x"))
    try:
        adapter.remove(Ice.stringToIdentity("x"))
    except Ice.NotRegisteredException:
        gotException = True
    test(gotException)

    adapter.deactivate()
    print "ok"

    print "testing servant locator registrations exceptions... ",
    adapter = communicator.createObjectAdapter("TestAdapter2")
    loc = ServantLocatorI()
    adapter.addServantLocator(loc, "x")
    gotException = False
    try:
        adapter.addServantLocator(loc, "x")
    except Ice.AlreadyRegisteredException:
        gotException = True
    test(gotException)

    adapter.deactivate()
    print "ok"

    print "testing object factory registration exceptions... ",
    of = ObjectFactoryI()
    communicator.addObjectFactory(of, "x")
    gotException = False
    try:
        communicator.addObjectFactory(of, "x")
    except Ice.AlreadyRegisteredException:
        gotException = True
    test(gotException)

    gotException = False
    communicator.removeObjectFactory("x")
    try:
        communicator.removeObjectFactory("x")
    except Ice.NotRegisteredException:
        gotException = True
    test(gotException)
    print "ok"

    print "testing stringToProxy... ",
    ref = "thrower:default -p 12345 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    print "ok"

    print "testing checked cast... ",
    thrower = Test.ThrowerPrx.checkedCast(base)
    test(thrower)
    test(thrower == base)
    print "ok"

    print "catching exact types... ",

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

    print "catching base types... ",

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

    print "catching derived types... ",

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
        print "catching unknown user exception... ",

        try:
            thrower.throwUndeclaredA(1)
            test(False)
        except Ice.UnknownUserException:
            #
            # We get an unknown user exception without collocation
            # optimization.
            #
            pass
        except:
            print sys.exc_info()
            test(False)

        try:
            thrower.throwUndeclaredB(1, 2)
            test(False)
        except Ice.UnknownUserException:
            #
            # We get an unknown user exception without collocation
            # optimization.
            #
            pass
        except:
            print sys.exc_info()
            test(False)

        try:
            thrower.throwUndeclaredC(1, 2, 3)
            test(False)
        except Ice.UnknownUserException:
            #
            # We get an unknown user exception without
            # collocation optimization.
            #
            pass
        except:
            print sys.exc_info()
            test(False)

        print "ok"

    print "catching object not exist exception... ",

    id = Ice.stringToIdentity("does not exist")
    try:
        thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_newIdentity(id))
        thrower2.throwAasA(1)
#        thrower2.ice_ping()
        test(False)
    except Ice.ObjectNotExistException, ex:
        test(ex.id == id)
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching facet not exist exception... ",

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

    print "catching operation not exist exception... ",

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

    print "catching unknown local exception... ",

    try:
        thrower.throwLocalException()
        test(False)
    except Ice.UnknownLocalException:
        #
        # We get an unknown local exception without collocation
        # optimization.
        #
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    print "catching unknown non-Ice exception... ",

    try:
        thrower.throwNonIceException()
        test(False)
    except Ice.UnknownException:
        #
        # We get an unknown exception without collocation
        # optimization.
        #
        pass
    except:
        print sys.exc_info()
        test(False)

    print "ok"

    return thrower
