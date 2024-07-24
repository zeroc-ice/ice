#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
import Test


class MyObjectI(Test.MyObject):
    def ice_ping(self, current):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

    def getName(self, current):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

        return name
