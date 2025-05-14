# Copyright (c) ZeroC, Inc.

from .Value import Value
import IcePy

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


class SlicedData:
    """
    Holds class slices that cannot be unmarshaled because their types are not known locally.

    Attributes
    ----------
    slices : tuple of SliceInfo
        The details of each slice, in order of most-derived to least-derived.
    """

    pass


class SliceInfo:
    """
    Encapsulates the details of a slice with an unknown type.

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
