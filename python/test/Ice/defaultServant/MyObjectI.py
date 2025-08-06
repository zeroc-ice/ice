# Copyright (c) ZeroC, Inc.

from generated.test.Ice.defaultServant import Test

import Ice


class MyObjectI(Test.MyObject):
    def ice_ping(self, current: Ice.Current):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

    def getName(self, current: Ice.Current):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

        return name
