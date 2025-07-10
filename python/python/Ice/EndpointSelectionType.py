# Copyright (c) ZeroC, Inc.

from enum import Enum


class EndpointSelectionType(Enum):
    """
    Determines the order in which the Ice runtime uses the endpoints in a proxy when establishing a connection.

    Enumerators
    -----------
    Random : EndpointSelectionType
        Random causes the endpoints to be arranged in a random order.
    Ordered : EndpointSelectionType
        Ordered forces the Ice runtime to use the endpoints in the order they appeared in the proxy.
    """

    Random = 0
    Ordered = 1
