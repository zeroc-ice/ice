# Copyright (c) ZeroC, Inc. All rights reserved.

class LocalException(Exception):
    """The base class for Ice run-time exceptions."""

    __module__ = "Ice"

__all__ = ["LocalException"]
