# Copyright (c) ZeroC, Inc.

from builtins import Exception as BuiltinsException


class Exception(BuiltinsException):
    """
    Abstract base class for all Ice exceptions.
    It has only two derived classes: :class:`LocalException` and :class:`UserException`.
    """

    _ice_id: str

    def ice_id(self) -> str:
        """
        Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
        and to a fully scoped name for other exceptions. For example: ``"::Ice::CommunicatorDestroyedException"``.

        Returns
        -------
        str
            The type ID of this exception.
        """
        return self._ice_id


__all__ = ["Exception"]
