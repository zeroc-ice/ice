# Copyright (c) ZeroC, Inc. All rights reserved.

import IcePy

class Value(object):
    def ice_id(self):
        """Obtains the type id corresponding to the most-derived Slice
        interface supported by the target object.
        Returns:
            The type id."""
        return "::Ice::Object"

    @staticmethod
    def ice_staticId():
        """Obtains the type id of this Slice class or interface.
        Returns:
            The type id."""
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
        """Returns the sliced data if the value has a preserved-slice base class and has been sliced during
        un-marshaling of the value, null is returned otherwise.
        Returns:
            The sliced data or null."""
        return getattr(self, "_ice_slicedData", None)

IcePy._t_Value = IcePy.defineValue("::Ice::Object", Value, -1, (), False, None, ())
