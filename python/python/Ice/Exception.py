# Copyright (c) ZeroC, Inc. All rights reserved.

class Exception(Exception):  # Derives from built-in base 'Exception' class.
    """The base class for all Ice exceptions."""

    def __init__(self):
        super().__init__()

    def __str__(self):
        return self.__class__.__name__

    def ice_id(self):
        """Returns the type id of this exception."""
        return self._ice_id

    __module__ = "Ice"
