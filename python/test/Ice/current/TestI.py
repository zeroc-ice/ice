# Copyright (c) ZeroC, Inc.

import Test

class MyClassI(Test.TestIntf):

    def getAdapterName(self, current):
        return current.adapter.getName()

    def getConnection(self, current):
        return str(current.con) if current.con else ""

    def getIdentity(self, current):
        return current.id

    def getFacet(self, current):
        return current.facet

    def getOperation(self, current):
        return current.operation

    def getMode(self, current):
        return str(current.mode)

    def getContext(self, current):
        return current.ctx

    def getRequestId(self, current):
        return current.requestId

    def getEncoding(self, current):
        return str(current.encoding)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()
