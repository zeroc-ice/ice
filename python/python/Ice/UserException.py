# Copyright (c) ZeroC, Inc.

from .Exception import Exception

class UserException(Exception):
    """The base class for all user-defined exceptions."""

    __module__ = "Ice"

    def __init__(self):
        super().__init__()

    def __str__(self):
        return repr(self)

__all__ = ["UserException"]
