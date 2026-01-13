# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException


class LocalException(IceException):
    """Base class for all Ice exceptions not defined in Slice."""

    pass


__all__ = ["LocalException"]
