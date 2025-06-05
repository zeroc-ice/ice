# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException

class UserException(IceException):
    """The base class for all user-defined exceptions."""

    def __init__(self):
        super().__init__()

    def __str__(self):
        return repr(self)

__all__ = ["UserException"]
