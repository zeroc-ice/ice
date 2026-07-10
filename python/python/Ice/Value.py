# Copyright (c) ZeroC, Inc.

import IcePy

from .SlicedData import SlicedData


class Value:
    """
    The base class for instances of Slice-defined classes.

    Notes
    -----
    If a subclass defines ``ice_preMarshal(self)``, the Ice runtime calls it just before marshaling the instance;
    if it defines ``ice_postUnmarshal(self)``, the runtime calls it after the instance has been fully unmarshaled.
    """

    def ice_id(self) -> str:
        """
        Returns the Slice type ID of the most-derived class supported by this object.

        Returns
        -------
        str
            The Slice type ID.
        """
        # Call ice_staticId() on self to get the value from the most-derived class.
        return self.ice_staticId()

    @staticmethod
    def ice_staticId() -> str:
        """
        Returns the Slice type ID of this type.

        Returns
        -------
        str
            The return value is always ``::Ice::Object``.
        """
        return "::Ice::Object"

    #
    # Do not define these here. They will be invoked if defined by a subclass.
    #
    # def ice_preMarshal(self):
    #    pass
    #
    # def ice_postUnmarshal(self):
    #    pass

    def ice_getSlicedData(self) -> SlicedData | None:
        """
        Gets the sliced data associated with this instance.

        Returns
        -------
        SlicedData | None
            The sliced data if this value was sliced during unmarshaling, ``None`` otherwise. Unknown slices are
            preserved only when the sender uses the sliced format.
        """
        return getattr(self, "_ice_slicedData", None)


IcePy._t_Value = IcePy.defineValue("::Ice::Object", Value, -1, (), False, None, ())

__all__ = ["Value"]
