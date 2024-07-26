# Copyright (c) ZeroC, Inc. All rights reserved.

from .EnumBase import EnumBase

__name__ = "Ice"

class ConnectionClose(EnumBase):
    """
    Determines the behavior when manually closing a connection.

    Enumerators
    -----------
    Forcefully : ConnectionClose
        Close the connection immediately without sending a close connection protocol message to the peer and
        waiting for the peer to acknowledge it.
    Gracefully : ConnectionClose
        Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete.
        On the server side, the connection will not be closed until all incoming invocations have completed.
    GracefullyWithWait : ConnectionClose
        Wait for all pending invocations to complete before closing the connection.
    """

    def __init__(self, _n, _v):
        super().__init__(_n, _v)

    def valueOf(self, value):
        """
        Get the enumerator corresponding to the given value.

        Parameters
        ----------
        value : int
            The enumerator's value.

        Returns
        -------
        ConnectionClose or None
            The enumerator corresponding to the given value, or None if no such enumerator exists.
        """
        return self._enumerators[value] if value in self._enumerators else None

    valueOf = classmethod(valueOf)


ConnectionClose.Forcefully = ConnectionClose("Forcefully", 0)
ConnectionClose.Gracefully = ConnectionClose("Gracefully", 1)
ConnectionClose.GracefullyWithWait = ConnectionClose("GracefullyWithWait", 2)
ConnectionClose._enumerators = {
    0: ConnectionClose.Forcefully,
    1: ConnectionClose.Gracefully,
    2: ConnectionClose.GracefullyWithWait,
}
