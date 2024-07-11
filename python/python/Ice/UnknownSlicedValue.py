# Copyright (c) ZeroC, Inc. All rights reserved.

from .Value import Value
import IcePy


class UnknownSlicedValue(Value):
    #
    # Members:
    #
    # unknownTypeId - string

    def ice_id(self):
        return self.unknownTypeId


class SlicedData(object):
    #
    # Members:
    #
    # slices - tuple of SliceInfo
    #
    pass


class SliceInfo(object):
    #
    # Members:
    #
    # typeId - string
    # compactId - int
    # bytes - string/bytes
    # instances - tuple of Ice.Value
    # hasOptionalMembers - boolean
    # isLastSlice - boolean
    pass

IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)
UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue
