# Copyright (c) ZeroC, Inc.

from typing import Awaitable, override

from generated.test.Ice.servantLocator import Test
from TestHelper import test

import Ice


class TestI(Test.TestIntf):
    @override
    def requestFailedException(self, current: Ice.Current):
        return None

    @override
    def unknownUserException(self, current: Ice.Current):
        return None

    @override
    def unknownLocalException(self, current: Ice.Current):
        return None

    @override
    def unknownException(self, current: Ice.Current):
        return None

    @override
    def localException(self, current: Ice.Current):
        return None

    @override
    def userException(self, current: Ice.Current):
        return None

    @override
    def pythonException(self, current: Ice.Current):
        return None

    @override
    def unknownExceptionWithServantException(self, current: Ice.Current) -> Awaitable[None]:
        f = Ice.Future()
        f.set_exception(Ice.ObjectNotExistException())
        return f

    @override
    def impossibleException(self, shouldThrow: bool, current: Ice.Current) -> Awaitable[str]:
        f = Ice.Future()
        if shouldThrow:
            f.set_exception(Test.TestImpossibleException())
        else:
            #
            # Return a value so we can be sure that the stream position
            # is reset correctly if finished() throws.
            #
            f.set_result("Hello")
        return f

    @override
    def intfUserException(self, shouldThrow: bool, current: Ice.Current) -> Awaitable[str]:
        f = Ice.Future()
        if shouldThrow:
            f.set_exception(Test.TestIntfUserException())
        else:
            #
            # Return a value so we can be sure that the stream position
            # is reset correctly if finished() throws.
            #
            f.set_result("Hello")
        return f

    @override
    def asyncResponse(self, current: Ice.Current) -> Awaitable[None]:
        #
        # We can't do this with futures.
        #
        # return Ice.Future.completed(None)
        raise Ice.ObjectNotExistException()

    @override
    def asyncException(self, current: Ice.Current) -> Awaitable[None]:
        #
        # We can't do this with futures.
        #
        # f = Ice.Future()
        # f.set_exception(Test.TestIntfUserException())
        # return f
        raise Ice.ObjectNotExistException()

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.deactivate()


class Cookie:
    def message(self):
        return "blahblah"


class ServantLocatorI(Ice.ServantLocator):
    def __init__(self, category: str):
        self._deactivated = False
        self._category = category
        self._requestId = -1

    def __del__(self):
        test(self._deactivated)

    @override
    def locate(self, current: Ice.Current) -> tuple[Ice.Object | None, object | None]:
        test(not self._deactivated)

        test(current.id.category == self._category or self._category == "")

        if current.id.name == "unknown":
            return (None, None)

        if current.id.name == "invalidReturnValue":
            return (45, 12)  # type: ignore

        if current.id.name == "invalidReturnType":
            return "invalid"  # type: ignore

        test(current.id.name == "locate" or current.id.name == "finished")
        if current.id.name == "locate":
            self.exception(current)

        #
        # Ensure locate() is only called once per request.
        #
        test(self._requestId == -1)
        self._requestId = current.requestId

        return (TestI(), Cookie())

    @override
    def finished(self, current: Ice.Current, servant: Ice.Object, cookie: object | None):
        test(not self._deactivated)

        #
        # Ensure finished() is only called once per request.
        #
        test(self._requestId == current.requestId)
        self._requestId = -1

        test(current.id.category == self._category or self._category == "")
        test(current.id.name == "locate" or current.id.name == "finished")

        if current.id.name == "finished":
            self.exception(current)

        assert isinstance(cookie, Cookie)
        test(cookie.message() == "blahblah")

    @override
    def deactivate(self, category: str):
        test(not self._deactivated)

        self._deactivated = True

    def exception(self, current: Ice.Current):
        if current.operation == "ice_ids":
            raise Test.TestIntfUserException()
        elif current.operation == "requestFailedException":
            raise Ice.ObjectNotExistException()
        elif current.operation == "unknownUserException":
            raise Ice.UnknownUserException("reason")
        elif current.operation == "unknownLocalException":
            raise Ice.UnknownLocalException("reason")
        elif current.operation == "unknownException":
            raise Ice.UnknownException("reason")
        elif current.operation == "userException":
            raise Test.TestIntfUserException()
        elif current.operation == "localException":
            raise Ice.SocketException(0)
        elif current.operation == "pythonException":
            raise RuntimeError("message")
        elif current.operation == "unknownExceptionWithServantException":
            raise Ice.UnknownException("reason")
        elif current.operation == "impossibleException":
            raise Test.TestIntfUserException()  # Yes, it really is meant to be TestIntfUserException.
        elif current.operation == "intfUserException":
            raise Test.TestImpossibleException()  # Yes, it really is meant to be TestImpossibleException.
        elif current.operation == "asyncResponse":
            raise Test.TestImpossibleException()
        elif current.operation == "asyncException":
            raise Test.TestImpossibleException()
