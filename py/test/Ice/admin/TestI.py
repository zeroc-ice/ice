# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestFacetI(Test.TestFacet):
    def op(self, current = None):
        return

class RemoteCommunicatorI(Test.RemoteCommunicator, Ice.PropertiesAdminUpdateCallback):
    def __init__(self, communicator):
        self.communicator = communicator
        self.called = False
        self.m = threading.Condition()

    def getAdmin(current = None):
        return self.communicator.getAdmin()

    def getChanges(current = None):
        self.m.acquire()
        try:
            #
            # The client calls PropertiesAdmin::setProperties() and then invokes
            # this operation. Since setProperties() is implemented using AMD, the
            # client might receive its reply and then call getChanges() before our
            # updated() method is called. We block here to ensure that updated()
            # gets called before we return the most recent set of changes.
            #
            while !self.called:
                self.m.wait()
            
            self.called = false
            
            return self.changes
        finally:
            self.m.release()

    def shutdown(current = None):
        self.communicator.shutdown()

    def waitForShutdown(current = None):
        #
        # Note that we are executing in a thread of the *main* communicator,
        # not the one that is being shut down.
        #
        self.communicator.waitForShutdown()

    def destroy(current = None):
        self.communicator.destroy()

    def updated(changes):
        self.m.acquire()
        try:
            self.changes = changes
            self.called = true
            self.m.notify()
        finally:
            self.m.release()

class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):

    def createCommunicator(props, current = None):
        #
        # Prepare the property set using the given properties.
        #
        init = Ice.InitializationData()
        init.properties = Ice.Util.createProperties()
        for k, v in props:
            init.properties.setProperty(k, v)

        #
        # Initialize a new communicator.
        #
        communicator = Ice.Util.initialize(init)

        #
        # Install a custom admin facet.
        #
        communicator.addAdminFacet(TestFacetI(), "TestFacet")

        #
        # The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        # Set the callback on the admin facet.
        #
        RemoteCommunicatorI servant = RemoteCommunicatorI(communicator)
        propFacet = communicator.findAdminFacet("Properties")

        admin = (Ice.NativePropertiesAdmin)propFacet
        test(admin != None)
        admin.addUpdateCallback(servant)

        proxy = current.adapter.addWithUUID(servant)
        return RemoteCommunicatorPrxHelper.uncheckedCast(proxy)

    def shutdown(current = None)
        current.adapter.getCommunicator().shutdown()

