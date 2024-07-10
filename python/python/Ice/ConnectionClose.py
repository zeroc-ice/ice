# Copyright (c) ZeroC, Inc. All rights reserved.

from .EnumBase import EnumBase

class ConnectionClose(EnumBase):
    """
        Determines the behavior when manually closing a connection.
    Enumerators:
    Forcefully --  Close the connection immediately without sending a close connection protocol message to the peer and waiting
        for the peer to acknowledge it.
    Gracefully --  Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete. On the
        server side, the connection will not be closed until all incoming invocations have completed.
    GracefullyWithWait --  Wait for all pending invocations to complete before closing the connection.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, _n):
        if _n in self._enumerators:
            return self._enumerators[_n]
        return None

    valueOf = classmethod(valueOf)

ConnectionClose.Forcefully = ConnectionClose("Forcefully", 0)
ConnectionClose.Gracefully = ConnectionClose("Gracefully", 1)
ConnectionClose.GracefullyWithWait = ConnectionClose("GracefullyWithWait", 2)
ConnectionClose._enumerators = {
    0: ConnectionClose.Forcefully,
    1: ConnectionClose.Gracefully,
    2: ConnectionClose.GracefullyWithWait,
}
