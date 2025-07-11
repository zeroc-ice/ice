# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException


class UserException(IceException):
    """The base class for all user-defined exceptions."""

    def __str__(self) -> str:
        return repr(self)


__all__ = ["UserException"]
