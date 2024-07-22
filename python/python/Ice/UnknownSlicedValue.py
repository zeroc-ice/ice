# Copyright (c) ZeroC, Inc. All rights reserved.

from .Value import Value
import IcePy

__name__ = "Ice"

class UnknownSlicedValue(Value):
    """
    Unknown sliced value holds an instance of an unknown Slice class type.

    Attributes
    ----------
    unknownTypeId : str
        The type ID of the unknown Slice class type.
    """

    def ice_id(self):
        return self.unknownTypeId


class SlicedData(object):
    """
    Holds the slices of an unknown class instance.

    Attributes
    ----------
    slices : tuple of SliceInfo
        The slices of the unknown class instance.
    """
    pass


class SliceInfo(object):
    """
    Encapsulate the details of a slice for an unknown class or exception type.

    Attributes
    ----------
    typeId : str
        The Slice type ID for this slice.
    compactId : int
        The Slice compact type ID for this slice.
    bytes : bytes
        The encoded bytes for this slice, including the leading size integer.
    hasOptionalMembers : bool
        Whether or not the slice contains optional members.
    isLastSlice : bool
        Whether or not this is the last slice.
    """
    pass

IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)
UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue
