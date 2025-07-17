# Copyright (c) ZeroC, Inc.

from enum import Enum


class FormatType(Enum):
    """
    This enumeration describes the possible formats for classes and exceptions.
    """

    CompactFormat = 0
    SlicedFormat = 1


__all__ = ["FormatType"]
