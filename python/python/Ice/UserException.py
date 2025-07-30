# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException


class UserException(IceException):
    """
    The base class for all user-defined exceptions.
    """


__all__ = ["UserException"]
