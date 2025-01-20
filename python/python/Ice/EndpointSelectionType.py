#  Copyright (c) ZeroC, Inc.

from .EnumBase import EnumBase

class EndpointSelectionType(EnumBase):
    """
    Determines the order in which the Ice runtime uses the endpoints in a proxy when establishing a connection.

    Enumerators
    -----------
    Random : EndpointSelectionType
        Random causes the endpoints to be arranged in a random order.
    Ordered : EndpointSelectionType
        Ordered forces the Ice runtime to use the endpoints in the order they appeared in the proxy.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, value):
        """
        Get the enumerator corresponding to the given value.

        Parameters
        ----------
        value : int
            The enumerator's value.

        Returns
        -------
        CompressBatch or None
            The enumerator corresponding to the given value, or None if no such enumerator exists.
        """
        return self._enumerators[value] if value in self._enumerators else None

    valueOf = classmethod(valueOf)

EndpointSelectionType.Random = EndpointSelectionType("Random", 0)
EndpointSelectionType.Ordered = EndpointSelectionType("Ordered", 1)
EndpointSelectionType._enumerators = {
    0: EndpointSelectionType.Random,
    1: EndpointSelectionType.Ordered,
}
