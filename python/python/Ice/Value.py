#  Copyright (c) ZeroC, Inc.

import IcePy

class Value(object):
    def ice_id(self):
        """
        Obtain the type ID corresponding to the most-derived Slice interface supported by the target object.

        Returns
        -------
        str
            The type ID.
        """
        return "::Ice::Object"

    @staticmethod
    def ice_staticId():
        """
        Obtain the type ID of this Slice class or interface.

        Returns
        -------
        str
            The type ID.
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

    def ice_getSlicedData(self):
        """
        Return the sliced data if the value has a preserved-slice base class and has been sliced during
        un-marshaling of the value. Return None otherwise.

        Returns
        -------
        Ice.SlicedData or None
            The sliced data or None.
        """
        return getattr(self, "_ice_slicedData", None)

IcePy._t_Value = IcePy.defineValue("::Ice::Object", Value, -1, (), False, None, ())
