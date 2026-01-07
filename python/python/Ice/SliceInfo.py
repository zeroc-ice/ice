# Copyright (c) ZeroC, Inc.


class SliceInfo:
    """
    Encapsulates the details of a class slice with an unknown type.

    Attributes
    ----------
    typeId : str
        The Slice type ID for this slice. It's empty when ``compactId`` is set (not ``-1``).
    compactId : int
        The Slice compact type ID for this slice, or ``-1`` if the slice has no compact ID.
    bytes : bytes
        The encoded bytes for this slice, including the leading size integer.
    hasOptionalMembers : bool
        Whether or not the slice contains optional members.
    isLastSlice : bool
        Whether or not this is the last slice.
    """

    pass
