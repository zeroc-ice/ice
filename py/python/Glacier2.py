# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

"""
Glacier2 module
"""

import sys, exceptions, string, imp, os, threading, warnings, datetime, traceback, copy

#
# Import the Python extension.
#
import IcePy
import Ice

import Glacier2_RouterF_ice
import Glacier2_Router_ice
import Glacier2_Session_ice
import Glacier2_PermissionsVerifierF_ice
import Glacier2_PermissionsVerifier_ice
import Glacier2_SSLInfo_ice

class SessionNotExistException(Exception):
    def __init__(self):
        pass

class RestartSessionException(Exception):
    def __init__(self):
        pass

class AMI_Router_refreshSessionI:

    def __init__(self, app, pinger):
        self._app = app
        self._pinger = pinger
    
    def ice_response(self):
        pass

    def ice_exception(self, ex):
        # Here the session has gone. The thread
        # terminates, and we notify the
        # application that the session has been
        # destroyed.
        self._pinger.done()
        self._app.sessionDestroyed()


class SessionPingThread(threading.Thread):
    def __init__(self, app, router, timeout):
        threading.Thread.__init__(self)
        self._router = router
        self._app = app
        self._timeout = timeout
        self._terminated = False
        self._cond = threading.Condition()

    def run(self):
        self._cond.acquire()
        try:
            while not self._terminated:
                self._cond.wait(self._timeout)
                if not self._terminated:
                    self._router.refreshSession_async(AMI_Router_refreshSessionI(self._app, self))
        finally:
            self._cond.release()

    def done(self):
        self._cond.acquire()
        try:
            self._terminated = True
            self._cond.notify()
        finally:
            self._cond.release()

class Application(Ice.Application):

    def __init__(self, signalPolicy=0): # HandleSignals=0
        '''The constructor accepts an optional argument indicating
whether to handle signals. The value should be either
Application.HandleSignals (the default) or
Application.NoSignalHandling.
'''
        self._router = None
        self._session = None
        self._adapter = None
        self._createdSession = False

        if type(self) == Application:
            raise RuntimeError("Glacier2.Application is an abstract class")
        Ice.Application.__init__(self, signalPolicy)
 
    def run(self, args):
        raise RuntimeError('run should not be called on Galcier2.Application instead runWithSession should be used')

    def runWithSession(self, args):
        raise RuntimeError('runWithSession() not implemented')

    def createSession(self, args):
        raise RuntimeError('createSession() not implemented')

    def restart(self):
        raise RestartException()

    def sessionDestroyed(self):
        pass

    def router(self):
        return self._router

    def session(self):
        return self._session

    def categoryForClient(self):
        if self._router == None:
            raise SessionNotExistException()
        return self._router.getCategoryForClient()

    def createCallbackIdentity(self, name):
        return Ice.Identity(name, self.categoryForClient())

    def addWithUUID(self, servant):
        return objectAdapter().add(servant, createCallbackIdentity(Ice.generateUUID()))

    def objectAdapter(self):
        if self._adapter == None:
            # TODO: Depending on the resolution of
            # http://bugzilla/bugzilla/show_bug.cgi?id=4264 the OA
            # name could be an empty string.
            self._adapter = self.communicator().createObjectAdapterWithRouter(Ice.generateUUID(), self.router())
            self._adapter.activate();
        return self._adapter

    def doMainInternal(self, args, initData, status):
        # Reset internal state variables from Ice.Application. The
        # remainder are reset at the end of this method.
        Ice.Application._callbackInProgress = False
        Ice.Application._destroyed = False
        Ice.Application._interrupted = False

        restart = False
        status = 0

        ping = None
        try:
            Ice.Application._communicator = Ice.initialize(args, initData);
            self._router = RouterPrx.uncheckedCast(Ice.Application.communicator().getDefaultRouter())
            
            if self._router == None:
                Ice.getProcessLogger().error("no glacier2 router configured");
                status = 1;
            else:
                #
                # The default is to destroy when a signal is received.
                #
                if Ice.Application._signalPolicy == Ice.Application.HandleSignals:
                    Ice.Application.destroyOnInterrupt()

                # If createSession throws, we're done.
                try:
                    self._session = self.createSession()
                    self._createdSession = True
                except Ice.LocalException as (ex):
                    Ice.getProcessLogger().error(traceback.format_exc())
                    status = 1;

                if self._createdSession:
                    ping = SessionPingThread(self, self._router, self._router.getSessionTimeout() / 2)
                    ping.start();
                    status = self.runWithSession(args)

        # We want to restart on those exceptions which indicate a
        # break down in communications, but not those exceptions that
        # indicate a programming logic error (ie: marshal, protocol
        # failure, etc).
        except(RestartSessionException, Ice.ConnectionLostException, Ice.ConnectionLostException, \
                Ice.UnknownLocalException, Ice.RequestFailedException, Ice.TimeoutException) as (ex):
            Ice.getProcessLogger().error(traceback.format_exc())
        except:
            Ice.getProcessLogger().error(traceback.format_exc())
            status = 1


        if self._createdSession and self._router != None:
            try:
                self._router.destroySession();
            except (Ice.ConnectionLostException, SessionNotExistException):
                pass
            except:
                Ice.getProcessLogger().error(traceback.format_exc())

        #
        # Don't want any new interrupt and at this point (post-run),
        # it would not make sense to release a held signal to run
        # shutdown or destroy.
        #
        if Application._signalPolicy == Application.HandleSignals:
            Application.ignoreInterrupt()

        Application._condVar.acquire()
        while Application._callbackInProgress:
            Application._condVar.wait()
        if Application._destroyed:
            Application._communicator = None
        else:
            Application._destroyed = True
            #
            # And _communicator != 0, meaning will be destroyed
            # next, _destroyed = true also ensures that any
            # remaining callback won't do anything
            #
        Application._application = None
        Application._condVar.release()

        if Application._communicator:
            try:
                Application._communicator.destroy()
            except:
                getProcessLogger().error(traceback.format_exc())
                status = 1

            Application._communicator = None

        #
        # Set _ctrlCHandler to 0 only once communicator.destroy() has
        # completed.
        # 
        Application._ctrlCHandler.destroy()
        Application._ctrlCHandler = None

        if ping != None:
            ping.done()

        # Reset internal state. We cannot reset the Application state
        # here, since _destroyed must remain true until we re-run
        # this method.
        self._adapter = None
        self._router = None
        self._session = None
        self._createdSession = False
        
        return restart

    def doMain(self, args, initData):
        initData.properties.setProperty("Ice.ACM.Client", "0");
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        restart = True;
        ret = 0;
        while restart:
            # A copy of the initialization data and the string seq
            # needs to be passed to doMainInternal, as these can be
            # changed by the application.
            id = copy.copy(initData)
            if id.properties != None:
                id.properties = id.properties.clone()
            argsCopy = args[:]
            restart = self.doMainInternal(argsCopy, initData, ret)
        return ret;
