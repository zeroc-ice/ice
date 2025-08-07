# Copyright (c) ZeroC, Inc.

from typing import override

from generated.test.Ice.current import Test

import Ice


class MyClassI(Test.TestIntf):
    @override
    def getAdapterName(self, current: Ice.Current):
        return current.adapter.getName()

    def getConnection(self, current: Ice.Current):
        return str(current.con) if current.con else ""

    @override
    def getIdentity(self, current: Ice.Current):
        return current.id

    @override
    def getFacet(self, current: Ice.Current):
        return current.facet

    @override
    def getOperation(self, current: Ice.Current):
        return current.operation

    @override
    def getMode(self, current: Ice.Current):
        return str(current.mode)

    @override
    def getContext(self, current: Ice.Current):
        return current.ctx

    @override
    def getRequestId(self, current: Ice.Current):
        return current.requestId

    @override
    def getEncoding(self, current: Ice.Current):
        return str(current.encoding)

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()
