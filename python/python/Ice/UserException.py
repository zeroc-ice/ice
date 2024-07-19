# Copyright (c) ZeroC, Inc. All rights reserved.

from .Exception import Exception

class UserException(Exception):
    """The base class for all user-defined exceptions."""

    __module__ = "Ice"
