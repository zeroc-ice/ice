# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

"""
Glacier2 module
"""

import threading, traceback, copy

#
# Import the Python extension.
#
import Ice
Ice.updateModule("Glacier2")

import Glacier2_Router_ice
import Glacier2_Session_ice
import Glacier2_PermissionsVerifier_ice
import Glacier2_SSLInfo_ice
import Glacier2_Metrics_ice

class SessionNotExistException(Exception):
    def __init__(self):
        pass

class RestartSessionException(Exception):
    def __init__(self):
        pass

class ConnectionCallbackI(Ice.ConnectionCallback):
    def __init__(self, app):
        self._app = app

    def heartbeat(self, conn):
        pass

    def closed(self, conn):
        self._app.sessionDestroyed()


class Application(Ice.Application):

    def __init__(self, signalPolicy=0): # HandleSignals=0
        '''The constructor accepts an optional argument indicating
whether to handle signals. The value should be either
Application.HandleSignals (the default) or
Application.NoSignalHandling.
'''

        if type(self) == Application:
            raise RuntimeError("Glacier2.Application is an abstract class")

        Ice.Application.__init__(self, signalPolicy)

        Application._adapter = None
        Application._router = None
        Application._session = None
        Application._createdSession = False
        Application._category = None

    def run(self, args):
        raise RuntimeError('run should not be called on Glacier2.Application - call runWithSession instead')

    def runWithSession(self, args):
        raise RuntimeError('runWithSession() not implemented')

    def createSession(self, args):
        raise RuntimeError('createSession() not implemented')

    def restart(self):
        raise RestartSessionException()

    def sessionDestroyed(self):
        pass

    def router(self):
        return Application._router
    router = classmethod(router)

    def session(self):
        return Application._session
    session = classmethod(session)

    def categoryForClient(self):
        if Application._router == None:
            raise SessionNotExistException()
        return Application._category

    def createCallbackIdentity(self, name):
        return Ice.Identity(name, self.categoryForClient())

    def addWithUUID(self, servant):
        return self.objectAdapter().add(servant, self.createCallbackIdentity(Ice.generateUUID()))

    def objectAdapter(self):
        if Application._router == None:
            raise SessionNotExistException()
        if Application._adapter == None:
            Application._adapter = self.communicator().createObjectAdapterWithRouter("", Application._router)
            Application._adapter.activate()
        return Application._adapter

    def doMainInternal(self, args, initData):
        # Reset internal state variables from Ice.Application. The
        # remainder are reset at the end of this method.
        Ice.Application._callbackInProgress = False
        Ice.Application._destroyed = False
        Ice.Application._interrupted = False

        restart = False
        status = 0

        try:
            Ice.Application._communicator = Ice.initialize(args, initData)

            Application._router = RouterPrx.uncheckedCast(Ice.Application.communicator().getDefaultRouter())
            if Application._router == None:
                Ice.getProcessLogger().error("no glacier2 router configured")
                status = 1
            else:
                #
                # The default is to destroy when a signal is received.
                #
                if Ice.Application._signalPolicy == Ice.Application.HandleSignals:
                    Ice.Application.destroyOnInterrupt()

                # If createSession throws, we're done.
                try:
                    Application._session = self.createSession()
                    Application._createdSession = True
                except Ice.LocalException:
                    Ice.getProcessLogger().error(traceback.format_exc())
                    status = 1

                if Application._createdSession:
                    acmTimeout = 0
                    try:
                        acmTimeout = Application._router.getACMTimeout()
                    except(Ice.OperationNotExistException):
                        pass
                    if acmTimeout <= 0:
                        acmTimeout = Application._router.getSessionTimeout()
                    if acmTimeout > 0:
                        connection = Application._router.ice_getCachedConnection()
                        assert(connection)
                        connection.setACM(acmTimeout, Ice.Unset, Ice.ACMHeartbeat.HeartbeatAlways)
                        connection.setCallback(ConnectionCallbackI(self))
                    Application._category = Application._router.getCategoryForClient()
                    status = self.runWithSession(args)

        # We want to restart on those exceptions which indicate a
        # break down in communications, but not those exceptions that
        # indicate a programming logic error (ie: marshal, protocol
        # failure, etc).
        except(RestartSessionException):
            restart = True
        except(Ice.ConnectionRefusedException, Ice.ConnectionLostException, Ice.UnknownLocalException, \
               Ice.RequestFailedException, Ice.TimeoutException):
            Ice.getProcessLogger().error(traceback.format_exc())
            restart = True
        except:
            Ice.getProcessLogger().error(traceback.format_exc())
            status = 1

        #
        # Don't want any new interrupt and at this point (post-run),
        # it would not make sense to release a held signal to run
        # shutdown or destroy.
        #
        if Ice.Application._signalPolicy == Ice.Application.HandleSignals:
            Ice.Application.ignoreInterrupt()

        Ice.Application._condVar.acquire()
        while Ice.Application._callbackInProgress:
            Ice.Application._condVar.wait()
        if Ice.Application._destroyed:
            Ice.Application._communicator = None
        else:
            Ice.Application._destroyed = True
            #
            # And _communicator != None, meaning will be destroyed
            # next, _destroyed = True also ensures that any
            # remaining callback won't do anything
            #
        Ice.Application._condVar.release()

        if Application._createdSession and Application._router:
            try:
                Application._router.destroySession()
            except (Ice.ConnectionLostException, SessionNotExistException):
                pass
            except:
                Ice.getProcessLogger().error("unexpected exception when destroying the session " + \
                                             traceback.format_exc())
            Application._router = None

        if Ice.Application._communicator:
            try:
                Ice.Application._communicator.destroy()
            except:
                getProcessLogger().error(traceback.format_exc())
                status = 1

            Ice.Application._communicator = None

        # Reset internal state. We cannot reset the Application state
        # here, since _destroyed must remain true until we re-run
        # this method.
        Application._adapter = None
        Application._router = None
        Application._session = None
        Application._createdSession = False
        Application._category = None

        return (restart, status)

    def doMain(self, args, initData):
        # Set the default properties for all Glacier2 applications.
        initData.properties.setProperty("Ice.RetryIntervals", "-1")

        restart = True
        ret = 0
        while restart:
            # A copy of the initialization data and the string seq
            # needs to be passed to doMainInternal, as these can be
            # changed by the application.
            id = copy.copy(initData)
            if id.properties:
                id.properties = id.properties.clone()
            argsCopy = args[:]
            (restart, ret) = self.doMainInternal(argsCopy, initData)
        return ret
