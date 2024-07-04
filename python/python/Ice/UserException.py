# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from .Exception import Exception

class UserException(Exception):
    """The base class for all user-defined exceptions."""

    def ice_getSlicedData(self):
        """Returns the sliced data if the value has a preserved-slice base class and has been sliced during
        un-marshaling of the value, null is returned otherwise.
        Returns:
            The sliced data or null."""
        return getattr(self, "_ice_slicedData", None)

    __module__ = "Ice"
    __class__ = "UserException"
