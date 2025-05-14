# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException

class LocalException(IceException):
    """The base class for Ice run-time exceptions."""
    pass

__all__ = ["LocalException"]
