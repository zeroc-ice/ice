# Copyright (c) ZeroC, Inc.

from builtins import Exception as BuiltinsException

class Exception(BuiltinsException):
    """
    The base class for all Ice exceptions.
    """

    def ice_id(self):
        """
        Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
        and to a similar fully scoped name for other exceptions. For example "::Ice::CommunicatorDestroyedException".

        Returns
        -------
        str
            The type ID.
        """
        return self._ice_id
