# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, IcePy

__name__ = 'Ice'
_M_Ice = Ice.openModule('Ice')

#
# Application class.
#
class Application(object):
    def __init__(self):
        pass

    def __del__(self):
        pass

    def main(self, args, configFile=None):
        if Ice.Application._communicator:
            print args[0] + ": only one instance of the Application class can be used"
            return False

        Ice.Application._interrupted = False
        Ice.Application._appName = args[0]

        status = 0

        try:
            if configFile:
                properties = Ice.createProperties(args)
                properties.load(configFile)
                Ice.Application._communicator = Ice.initializeWithProperties(args, properties)
            else:
                Ice.Application._communicator = Ice.initialize(args)

            #
            # The default is to destroy when a signal is received.
            #
            Ice.Application.destroyOnInterrupt()

            status = self.run(args)
        except Exception, ex:
            print Ice.Application._appName + ": " + str(ex)
            status = 1
        except exceptions.Exception, ex:
            print Ice.Application._appName + ": " + str(ex)
            status = 1

        if Ice.Application._communicator:
            #
            # We don't want to handle signals anymore.
            #
            Ice.Application.ignoreInterrupt()

            try:
                Ice.Application._communicator.destroy()
            except Exception, ex:
                print Ice.Application._appName + ": " + str(ex)
                status = 1

            Ice.Application._communicator = None

        return status

    def run(self, args):
        raise RuntimeError('run() not implemented')

    def appName():
        return Ice.Application._appName
    appName = staticmethod(appName)

    def communicator():
        return Ice.Application._communicator
    communicator = staticmethod(communicator)

    def destroyOnInterrupt():
        pass
    destroyOnInterrupt = staticmethod(destroyOnInterrupt)

    def shutdownOnInterrupt():
        pass
    shutdownOnInterrupt = staticmethod(shutdownOnInterrupt)

    def ignoreInterrupt():
        pass
    ignoreInterrupt = staticmethod(ignoreInterrupt)

    def holdInterrupt():
        pass
    holdInterrupt = staticmethod(holdInterrupt)

    def releaseInterrupt():
        pass
    releaseInterrupt = staticmethod(releaseInterrupt)

    def interrupted():
        pass
    interrupted = staticmethod(interrupted)

    _appName = None
    _communicator = None
    _interrupted = False
    _released = False

_M_Ice.Application = Application
del Application
