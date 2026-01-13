# Copyright (c) ZeroC, Inc.

from enum import Enum


class FormatType(Enum):
    """
    Specifies the format for marshaling classes and exceptions with the Slice 1.1 encoding.
    """

    CompactFormat = 0
    """
    Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
    """

    SlicedFormat = 1
    """
    Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled data.
    """


__all__ = ["FormatType"]
