# Copyright (c) ZeroC, Inc.

class SlicedData:
    """
    Holds class slices that cannot be unmarshaled because their types are not known locally.

    Attributes
    ----------
    slices : tuple of SliceInfo
        The details of each slice, in order of most-derived to least-derived.
    """

    pass
