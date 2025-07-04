# Copyright (c) ZeroC, Inc.

class SliceInfo:
    """
    Encapsulates the details of a slice with an unknown type.

    Attributes
    ----------
    typeId : str
        The Slice type ID for this slice.
    compactId : int
        The Slice compact type ID for this slice.
    bytes : bytes
        The encoded bytes for this slice, including the leading size integer.
    hasOptionalMembers : bool
        Whether or not the slice contains optional members.
    isLastSlice : bool
        Whether or not this is the last slice.
    """

    pass
