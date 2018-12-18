# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, Test

class MyObjectI(Test.MyObject):
    def ice_ping(self, current=None):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

    def getName(self, current=None):
        name = current.id.name

        if name == "ObjectNotExist":
            raise Ice.ObjectNotExistException()
        elif name == "FacetNotExist":
            raise Ice.FacetNotExistException()

        return name
