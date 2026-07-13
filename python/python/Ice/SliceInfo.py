# Copyright (c) ZeroC, Inc.

# Avoid evaluating annotations at class definition time.
from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .Value import Value


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
        The encoded bytes for this slice, excluding the slice header.
    instances : tuple[Value, ...]
        The class instances referenced by this slice.
    hasOptionalMembers : bool
        Whether or not the slice contains optional members.
    isLastSlice : bool
        Whether or not this is the last slice.
    """

    typeId: str
    compactId: int
    bytes: bytes
    instances: tuple[Value, ...]
    hasOptionalMembers: bool
    isLastSlice: bool
