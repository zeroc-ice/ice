# Copyright (c) ZeroC, Inc.

import array
import sys
import threading

import Test

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class EmptyI(Test.Empty):
    pass


class ServantLocatorI(Ice.ServantLocator):
    def locate(self, current):
        return None

    def finished(self, current, servant, cookie):
        pass

    def deactivate(self, category):
        pass


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
        except Exception:
            test(False)
        self.called()

    def exception_BasB(self, ex):
        try:
            raise ex
        except Test.B as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except Exception:
            test(False)
        self.called()

    def exception_CasC(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except Exception:
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
        except Exception:
            test(False)
        self.called()

    def exception_BasA(self, ex):
        try:
            raise ex
        except Test.B as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
        except Exception:
            test(False)
        self.called()

    def exception_CasA(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except Exception:
            test(False)
        self.called()

    def exception_CasB(self, ex):
        try:
            raise ex
        except Test.C as ex:
            test(ex.aMem == 1)
            test(ex.bMem == 2)
            test(ex.cMem == 3)
        except Exception:
            test(False)
        self.called()

    def exception_UndeclaredA(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except Exception:
            test(False)
        self.called()

    def exception_UndeclaredB(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except Exception:
            test(False)
        self.called()

    def exception_UndeclaredC(self, ex):
        try:
            raise ex
        except Ice.UnknownUserException:
            pass
        except Exception:
            test(False)
        self.called()

    def exception_AasAObjectNotExist(self, ex):
        try:
            raise ex
        except Ice.ObjectNotExistException as ex:
            id = Ice.stringToIdentity("does not exist")
            test(ex.id == id)
        except Exception:
            test(False)
        self.called()

    def exception_AasAFacetNotExist(self, ex):
        try:
            raise ex
        except Ice.FacetNotExistException as ex:
            test(ex.facet == "no such facet")
        except Exception:
            test(False)
        self.called()

    def exception_noSuchOperation(self, ex):
        try:
            raise ex
        except Ice.OperationNotExistException as ex:
            test(ex.operation == "noSuchOperation")
        except Exception:
            test(False)
        self.called()

    def exception_LocalException(self, ex):
        try:
            raise ex
        except Ice.UnknownLocalException:
            pass
        except Ice.OperationNotExistException:
            pass
        except Exception:
            test(False)
        self.called()

    def exception_NonIceException(self, ex):
        try:
            raise ex
        except Ice.UnknownException:
            pass
        except Exception:
            test(False)
        self.called()


def allTests(helper, communicator):
    sys.stdout.write("testing servant registration exceptions... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("TestAdapter1.Endpoints", "tcp -h *")
    adapter = communicator.createObjectAdapter("TestAdapter1")
    obj = EmptyI()
    adapter.add(obj, Ice.stringToIdentity("x"))
    try:
        adapter.add(obj, Ice.stringToIdentity("x"))
        test(False)
    except Ice.AlreadyRegisteredException:
        pass

    try:
        adapter.add(obj, Ice.Identity("", ""))
        test(False)
    except Ice.LocalException:
        pass

    try:
        adapter.add(None, Ice.stringToIdentity("x"))
        test(False)
    except Ice.LocalException:
        pass

    adapter.remove(Ice.stringToIdentity("x"))
    try:
        adapter.remove(Ice.stringToIdentity("x"))
        test(False)
    except Ice.NotRegisteredException:
        pass

    adapter.deactivate()
    print("ok")

    sys.stdout.write("testing servant locator registrations exceptions... ")
    sys.stdout.flush()
    communicator.getProperties().setProperty("TestAdapter2.Endpoints", "tcp -h *")
    adapter = communicator.createObjectAdapter("TestAdapter2")
    loc = ServantLocatorI()
    adapter.addServantLocator(loc, "x")
    try:
        adapter.addServantLocator(loc, "x")
        test(False)
    except Ice.AlreadyRegisteredException:
        pass

    adapter.deactivate()
    print("ok")

    thrower = Test.ThrowerPrx(communicator, f"thrower:{helper.getTestEndpoint()}")

    sys.stdout.write("catching exact types... ")
    sys.stdout.flush()

    try:
        thrower.throwAasA(1)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorD(1)
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorD(-1)
        test(False)
    except Test.D as ex:
        test(ex.dMem == -1)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwBasB(1, 2)
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
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
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasC(1, 2, 3)
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except Exception:
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
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasA(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasB(1, 2, 3)
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
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
        except Exception:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredB(1, 2)
            test(False)
        except Ice.UnknownUserException:
            pass
        except Exception:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredC(1, 2, 3)
            test(False)
        except Ice.UnknownUserException:
            pass
        except Exception:
            print(sys.exc_info())
            test(False)

        print("ok")

    if thrower.ice_getConnection():
        sys.stdout.write("testing memory limit marshal exception...")
        sys.stdout.flush()

        try:
            thrower.throwMemoryLimitException(array.array("B"))
            test(False)
        except Ice.MarshalException:
            pass
        except Exception:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwMemoryLimitException(bytearray(20 * 1024))  # 20KB
            test(False)
        except Ice.ConnectionLostException:
            pass
        except Ice.UnknownLocalException:
            pass
        except Exception:
            test(False)

        print("ok")

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
    except Exception:
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
    except Exception:
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
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)
    try:
        thrower.throwLocalExceptionIdempotent()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except Ice.OperationNotExistException:
        pass
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching dispatch exception... ")
    sys.stdout.flush()

    try:
        thrower.throwDispatchException(Ice.ReplyStatus.OperationNotExist.value)
        test(False)
    except Ice.OperationNotExistException as ex:
        test(
            str(ex)
            == "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }"
        )
        pass
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwDispatchException(Ice.ReplyStatus.Unauthorized.value)
        test(False)
    except Ice.DispatchException as ex:
        test(ex.replyStatus == Ice.ReplyStatus.Unauthorized.value)
        test(
            str(ex) == "The dispatch failed with reply status Unauthorized."
            or str(ex) == "The dispatch failed with reply status unauthorized."
        )  # for Swift
        pass
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwDispatchException(212)
        test(False)
    except Ice.DispatchException as ex:
        test(ex.replyStatus == 212)
        test(str(ex) == "The dispatch failed with reply status 212.")
        pass
    except Exception:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("testing asynchronous exceptions... ")
    sys.stdout.flush()

    try:
        thrower.throwAfterResponse()
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAfterException()
        test(False)
    except Test.A:
        pass
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorDAsync(1).result()
        test(False)
    except Test.A as ex:
        test(ex.aMem == 1)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwAorDasAorDAsync(-1).result()
        test(False)
    except Test.D as ex:
        test(ex.dMem == -1)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwBasBAsync(1, 2).result()
        test(False)
    except Test.B as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasCAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
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
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasAAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
        print(sys.exc_info())
        test(False)

    try:
        thrower.throwCasBAsync(1, 2, 3).result()
        test(False)
    except Test.C as ex:
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    except Exception:
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
        except Exception:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredBAsync(1, 2).result()
            test(False)
        except Ice.UnknownUserException:
            pass
        except Exception:
            print(sys.exc_info())
            test(False)

        try:
            thrower.throwUndeclaredCAsync(1, 2, 3).result()
            test(False)
        except Ice.UnknownUserException:
            pass
        except Exception:
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
    except Exception:
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
    except Exception:
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
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)
    try:
        thrower.throwLocalExceptionIdempotentAsync().result()
        test(False)
    except Ice.UnknownLocalException:
        pass
    except Ice.OperationNotExistException:
        pass
    except Exception:
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
    except Exception:
        print(sys.exc_info())
        test(False)

    print("ok")

    sys.stdout.write("catching unknown non-Ice exception with futures... ")
    sys.stdout.flush()
    try:
        try:
            thrower.throwMarshalException(context={"response": ""})
        except Ice.UnknownLocalException as ex:
            test("cannot marshal result" in str(ex))
        try:
            thrower.throwMarshalException(context={"param": ""})
        except Ice.UnknownLocalException as ex:
            test("cannot marshal result" in str(ex))
        try:
            thrower.throwMarshalException()
        except Ice.UnknownLocalException as ex:
            test("cannot marshal result" in str(ex))
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing setting RFE exception fields from Python... ")
    sys.stdout.flush()
    try:
        try:
            thrower.throwRequestFailedException(
                "Ice.ObjectNotExistException",
                Ice.Identity("name", "category"),
                "facet",
                "operation",
            )
            test(False)
        except Ice.ObjectNotExistException as ex:
            test(ex.id == Ice.Identity("name", "category"))
            test(ex.facet == "facet")
            test(ex.operation == "operation")

        try:
            thrower.throwRequestFailedException(
                "Ice.OperationNotExistException",
                Ice.Identity("name", "category"),
                "facet",
                "operation",
            )
            test(False)
        except Ice.OperationNotExistException as ex:
            test(ex.id == Ice.Identity("name", "category"))
            test(ex.facet == "facet")
            test(ex.operation == "operation")

        try:
            thrower.throwRequestFailedException(
                "Ice.FacetNotExistException",
                Ice.Identity("name", "category"),
                "facet",
                "operation",
            )
            test(False)
        except Ice.FacetNotExistException as ex:
            test(ex.id == Ice.Identity("name", "category"))
            test(ex.facet == "facet")
            test(ex.operation == "operation")
    except Ice.OperationNotExistException:
        # Only implemented in Python
        pass
    print("ok")
    return thrower
