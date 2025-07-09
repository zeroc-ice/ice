#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
import threading

import Glacier2
import Test
from TestHelper import TestHelper

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class CallbackReceiverI(Test.CallbackReceiver):
    def __init__(self):
        self._callback = False
        self._cond = threading.Condition()

    def callback(self, current):
        with self._cond:
            self._callback = True
            self._cond.notify()

    def callbackEx(self, current):
        self.callback(current)
        ex = Test.CallbackException()
        ex.someValue = 3.14
        ex.someString = "3.14"
        raise ex

    def callbackOK(self):
        with self._cond:
            while not self._callback:
                self._cond.wait(30000)
                test(self._callback)
            self._callback = False


class Client(TestHelper):
    def run(self, args):
        shutdown = "--shutdown" in args
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "1")
        properties.setProperty("Ice.Warn.Connections", "0")
        with self.initialize(properties=properties) as communicator:
            sys.stdout.write("testing stringToProxy for router... ")
            sys.stdout.flush()
            routerBase = communicator.stringToProxy(f"Glacier2/router:{self.getTestEndpoint(num=50)}")
            test(routerBase is not None)
            print("ok")

            sys.stdout.write("testing checked cast for router... ")
            sys.stdout.flush()
            router = Glacier2.RouterPrx.checkedCast(routerBase)
            test(router is not None)
            print("ok")

            sys.stdout.write("installing router with communicator... ")
            sys.stdout.flush()
            communicator.setDefaultRouter(router)
            print("ok")

            sys.stdout.write("getting the session timeout... ")
            sys.stdout.flush()
            timeout = router.getSessionTimeout()
            test(timeout == 30)
            print("ok")

            sys.stdout.write("testing stringToProxy for server object... ")
            sys.stdout.flush()
            base = communicator.stringToProxy(f"c1/callback:{self.getTestEndpoint()}")
            print("ok")

            sys.stdout.write("trying to ping server before session creation... ")
            sys.stdout.flush()
            try:
                base.ice_ping()
                test(False)
            except Ice.ConnectionLostException:
                pass
            print("ok")

            sys.stdout.write("trying to create session with wrong password... ")
            sys.stdout.flush()
            try:
                router.createSession("userid", "xxx")
                test(False)
            except Glacier2.PermissionDeniedException:
                pass
            print("ok")

            sys.stdout.write("trying to destroy non-existing session... ")
            sys.stdout.flush()
            try:
                router.destroySession()
                test(False)
            except Glacier2.SessionNotExistException:
                pass
            print("ok")

            sys.stdout.write("creating session with correct password... ")
            sys.stdout.flush()
            router.createSession("userid", "abc123")
            print("ok")

            sys.stdout.write("trying to create a second session... ")
            try:
                router.createSession("userid", "abc123")
                test(False)
            except Glacier2.CannotCreateSessionException:
                pass
            print("ok")

            sys.stdout.write("pinging server after session creation... ")
            sys.stdout.flush()
            base.ice_ping()
            print("ok")

            sys.stdout.write("pinging object with client endpoint... ")
            sys.stdout.flush()
            baseC = communicator.stringToProxy(f"collocated:{self.getTestEndpoint(num=50)}")
            try:
                baseC.ice_ping()
            except Exception:
                pass
            print("ok")

            sys.stdout.write("testing checked cast for server object... ")
            sys.stdout.flush()
            twoway = Test.CallbackPrx.checkedCast(base)
            test(twoway is not None)
            print("ok")

            sys.stdout.write("creating and activating callback receiver adapter... ")
            sys.stdout.flush()
            communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0")
            adapter = communicator.createObjectAdapterWithRouter("CallbackReceiverAdapter", router)
            adapter.activate()
            print("ok")

            sys.stdout.write("getting category from router... ")
            sys.stdout.flush()
            category = router.getCategoryForClient()
            print("ok")

            sys.stdout.write("creating and adding callback receiver object... ")
            sys.stdout.flush()
            callbackReceiverImpl = CallbackReceiverI()
            callbackReceiver = callbackReceiverImpl

            callbackReceiverIdent = Ice.Identity()
            callbackReceiverIdent.name = "callbackReceiver"
            callbackReceiverIdent.category = category
            twowayR = Test.CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent))

            fakeCallbackReceiverIdent = Ice.Identity()
            fakeCallbackReceiverIdent.name = "callbackReceiver"
            fakeCallbackReceiverIdent.category = "dummy"
            fakeTwowayR = Test.CallbackReceiverPrx.uncheckedCast(
                adapter.add(callbackReceiver, fakeCallbackReceiverIdent)
            )
            print("ok")

            sys.stdout.write("testing oneway callback... ")
            sys.stdout.flush()
            oneway = Test.CallbackPrx.uncheckedCast(twoway.ice_oneway())
            onewayR = Test.CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway())
            context = {"_fwd": "o"}
            oneway.initiateCallback(onewayR, context)
            callbackReceiverImpl.callbackOK()
            print("ok")

            sys.stdout.write("testing twoway callback... ")
            sys.stdout.flush()
            context = {"_fwd": "t"}
            twoway.initiateCallback(twowayR, context)
            callbackReceiverImpl.callbackOK()
            print("ok")

            sys.stdout.write("ditto, but with user exception... ")
            sys.stdout.flush()
            context = {"_fwd": "t"}
            try:
                twoway.initiateCallbackEx(twowayR, context)
                test(False)
            except Test.CallbackException as ex:
                test(ex.someValue == 3.14)
                test(ex.someString == "3.14")
            callbackReceiverImpl.callbackOK()
            print("ok")

            sys.stdout.write("trying twoway callback with fake category... ")
            sys.stdout.flush()
            context = {"_fwd": "t"}
            try:
                twoway.initiateCallback(fakeTwowayR, context)
                test(False)
            except Ice.ObjectNotExistException:
                pass
            print("ok")

            sys.stdout.write("testing whether other allowed category is accepted... ")
            context = {"_fwd": "t"}
            otherCategoryTwoway = Test.CallbackPrx.uncheckedCast(
                twoway.ice_identity(Ice.stringToIdentity("c2/callback"))
            )
            otherCategoryTwoway.initiateCallback(twowayR, context)
            callbackReceiverImpl.callbackOK()
            print("ok")

            sys.stdout.write("testing whether disallowed category gets rejected... ")
            sys.stdout.flush()
            context = {"_fwd": "t"}
            otherCategoryTwoway = Test.CallbackPrx.uncheckedCast(
                twoway.ice_identity(Ice.stringToIdentity("c3/callback"))
            )
            try:
                otherCategoryTwoway.initiateCallback(twowayR, context)
                test(False)
            except Ice.ObjectNotExistException:
                pass
            print("ok")

            sys.stdout.write("testing whether user-id as category is accepted... ")
            sys.stdout.flush()
            context = {"_fwd": "t"}
            otherCategoryTwoway = Test.CallbackPrx.uncheckedCast(
                twoway.ice_identity(Ice.stringToIdentity("_userid/callback"))
            )
            otherCategoryTwoway.initiateCallback(twowayR, context)
            callbackReceiverImpl.callbackOK()
            print("ok")

            if shutdown:
                sys.stdout.write("testing server shutdown... ")
                sys.stdout.flush()
                twoway.shutdown()
                print("ok")
                # No ping, otherwise the router prints a warning message if it's
                # started with --Ice.Warn.Connections.

            sys.stdout.write("destroying session... ")
            sys.stdout.flush()
            router.destroySession()
            print("ok")

            sys.stdout.write("trying to ping server after session destruction... ")
            try:
                base.ice_ping()
                test(False)
            except Ice.ConnectionLostException:
                pass
            print("ok")

            if shutdown:
                sys.stdout.write("uninstalling router with communicator... ")
                sys.stdout.flush()
                communicator.setDefaultRouter(None)
                print("ok")

                sys.stdout.write("testing stringToProxy for process object... ")
                sys.stdout.flush()
                processBase = communicator.stringToProxy(f"Glacier2/admin -f Process:{self.getTestEndpoint(num=51)}")
                print("ok")

                sys.stdout.write("testing checked cast for admin object... ")
                sys.stdout.flush()
                processPrx = Ice.ProcessPrx.checkedCast(processBase)
                test(processPrx is not None)
                print("ok")

                sys.stdout.write("testing Glacier2 shutdown... ")
                sys.stdout.flush()
                processPrx.shutdown()
                try:
                    processPrx.ice_invocationTimeout(500).ice_ping()
                    test(False)
                except Ice.LocalException:
                    pass
                print("ok")
