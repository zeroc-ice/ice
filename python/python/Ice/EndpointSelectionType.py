# Copyright (c) ZeroC, Inc.

from enum import Enum


class EndpointSelectionType(Enum):
    """
    Determines how the Ice runtime sorts proxy endpoints when establishing a connection.
    """

    Random = 0
    """
    The Ice runtime shuffles the endpoints in a random order.
    """

    Ordered = 1
    """
    The Ice runtime uses the endpoints in the order they appear in the proxy.
    """


__all__ = ["EndpointSelectionType"]
