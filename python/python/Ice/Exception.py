# Copyright (c) ZeroC, Inc.

from builtins import Exception as BuiltinsException

class Exception(BuiltinsException):
    """
    The base class for all Ice exceptions.
    """

    def ice_id(self):
        """
        Return the type ID of this exception.

        For exceptions defined in Slice, this corresponds to the Slice type ID.
        For other exceptions, it is a fully scoped name in the same format.
        For example: "::Ice::CommunicatorDestroyedException".

        Returns
        -------
        str
            The type ID of the exception.
        """
        return self._ice_id
