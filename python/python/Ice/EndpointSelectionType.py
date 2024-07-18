# Copyright (c) ZeroC, Inc. All rights reserved.

from .EnumBase import EnumBase

class EndpointSelectionType(EnumBase):
    """
        Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
    Enumerators:
    Random --  Random causes the endpoints to be arranged in a random order.
    Ordered --  Ordered forces the Ice run time to use the endpoints in the order they appeared in the proxy.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, _n):
        if _n in self._enumerators:
            return self._enumerators[_n]
        return None

    valueOf = classmethod(valueOf)

EndpointSelectionType.Random = EndpointSelectionType("Random", 0)
EndpointSelectionType.Ordered = EndpointSelectionType("Ordered", 1)
EndpointSelectionType._enumerators = {
    0: EndpointSelectionType.Random,
    1: EndpointSelectionType.Ordered,
}
