# Copyright (c) ZeroC, Inc.

from generated.test.Ice.current import Test

import Ice


class MyClassI(Test.TestIntf):
    def getAdapterName(self, current: Ice.Current):
        return current.adapter.getName()

    def getConnection(self, current: Ice.Current):
        return str(current.con) if current.con else ""

    def getIdentity(self, current: Ice.Current):
        return current.id

    def getFacet(self, current: Ice.Current):
        return current.facet

    def getOperation(self, current: Ice.Current):
        return current.operation

    def getMode(self, current: Ice.Current):
        return str(current.mode)

    def getContext(self, current: Ice.Current):
        return current.ctx

    def getRequestId(self, current: Ice.Current):
        return current.requestId

    def getEncoding(self, current: Ice.Current):
        return str(current.encoding)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()
