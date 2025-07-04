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

IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)
UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue
