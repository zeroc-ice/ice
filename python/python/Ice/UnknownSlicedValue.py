# Copyright (c) ZeroC, Inc.

import IcePy

from .Value import Value


class UnknownSlicedValue(Value):
    """
    Represents an instance of an unknown class.

    Instances of this class are created by the Ice runtime during unmarshaling; they should not be constructed
    directly. A directly constructed instance has no ``unknownTypeId``.

    Attributes
    ----------
    unknownTypeId : str
        The Slice type ID of the unknown value, or the string form of the compact type ID (for example, ``"1"``)
        when the most-derived slice was marshaled with a compact type ID.
    """

    _ice_type = None  # Will be set after class definition
    unknownTypeId: str

    def ice_id(self) -> str:
        """
        Returns the Slice type ID associated with this instance.

        Returns
        -------
        str
            The Slice type ID of the unknown value, or the string form of the compact type ID (for example,
            ``"1"``) when the most-derived slice was marshaled with a compact type ID.
        """
        return self.unknownTypeId


IcePy._t_UnknownSlicedValue = IcePy.defineValue(
    "::Ice::UnknownSlicedValue", UnknownSlicedValue, -1, (), False, None, ()
)

UnknownSlicedValue._ice_type = IcePy._t_UnknownSlicedValue

__all__ = ["UnknownSlicedValue"]
