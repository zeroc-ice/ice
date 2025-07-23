# Copyright (c) ZeroC, Inc.

import IcePy

from .Value import Value


class UnknownSlicedValue(Value):
    """
    Unknown sliced value holds an instance of an unknown Slice class type.

    Attributes
    ----------
    unknownTypeId : str
        The type ID of the unknown Slice class type.
    """

    _ice_type = None  # Will be set after class definition
    unknownTypeId: str

    def ice_id(self):
        return self.unknownTypeId


IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)

UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue

__all__ = ["UnknownSlicedValue"]
