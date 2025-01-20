#  Copyright (c) ZeroC, Inc.

class LocalException(Exception):
    """The base class for Ice run-time exceptions."""

    __module__ = "Ice"

__all__ = ["LocalException"]
