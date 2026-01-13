# Copyright (c) ZeroC, Inc.

import IcePy

from .Value import Value


class UnknownSlicedValue(Value):
    """
    Represents an instance of an unknown class.

    Attributes
    ----------
    unknownTypeId : str
        The Slice type ID of the unknown value.
    """

    _ice_type = None  # Will be set after class definition
    unknownTypeId: str

    def ice_id(self) -> str:
        """
        Returns the Slice type ID associated with this instance.

        Returns
        -------
        str
            The Slice type ID of the unknown value.
        """
        return self.unknownTypeId


IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)

UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue

__all__ = ["UnknownSlicedValue"]
