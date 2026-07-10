# Copyright (c) ZeroC, Inc.

from .SliceInfo import SliceInfo


class SlicedData:
    """
    Holds class slices that cannot be unmarshaled because their types are not known locally.

    Attributes
    ----------
    slices : tuple[SliceInfo, ...]
        The slices of the unknown class, in order of most-derived to least-derived.
    """

    slices: tuple[SliceInfo, ...]
