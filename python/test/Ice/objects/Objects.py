# Copyright (c) ZeroC, Inc.

from generated.test.Ice.objects import Test


class BI(Test.B):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True


class CI(Test.C):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True


class DI(Test.D):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True


def customSliceLoader(typeId):
    match typeId:
        case "::Test::B":
            return BI()
        case "::Test::C":
            return CI()
        case "::Test::D":
            return DI()
        case _:
            return None
