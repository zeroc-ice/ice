# Copyright (c) ZeroC, Inc. All rights reserved.

from .Value import Value


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
