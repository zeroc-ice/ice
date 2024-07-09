# Copyright (c) ZeroC, Inc. All rights reserved.

class Connection(object):
    """
    The user-level interface to a connection.
    """

    def __init__(self):
        if type(self) == Connection:
            raise RuntimeError("Ice.Connection is an abstract class")

    def close(self, mode):
        """
            Manually close the connection using the specified closure mode.
        Arguments:
        mode -- Determines how the connection will be closed.
        """
        raise NotImplementedError("method 'close' not implemented")

    def createProxy(self, id):
        """
            Create a special proxy that always uses this connection. This can be used for callbacks from a server to a
            client if the server cannot directly establish a connection to the client, for example because of firewalls. In
            this case, the server would create a proxy using an already established connection from the client.
        Arguments:
        id -- The identity for which a proxy is to be created.
        Returns: A proxy that matches the given identity and uses this connection.
        """
        raise NotImplementedError("method 'createProxy' not implemented")

    def setAdapter(self, adapter):
        """
            Explicitly set an object adapter that dispatches requests that are received over this connection. A client can
            invoke an operation on a server using a proxy, and then set an object adapter for the outgoing connection that
            is used by the proxy in order to receive callbacks. This is useful if the server cannot establish a connection
            back to the client, for example because of firewalls.
        Arguments:
        adapter -- The object adapter that should be used by this connection to dispatch requests. The object adapter must be activated. When the object adapter is deactivated, it is automatically removed from the connection. Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException
        """
        raise NotImplementedError("method 'setAdapter' not implemented")

    def getAdapter(self):
        """
            Get the object adapter that dispatches requests for this connection.
        Returns: The object adapter that dispatches requests for the connection, or null if no adapter is set.
        """
        raise NotImplementedError("method 'getAdapter' not implemented")

    def getEndpoint(self):
        """
            Get the endpoint from which the connection was created.
        Returns: The endpoint from which the connection was created.
        """
        raise NotImplementedError("method 'getEndpoint' not implemented")

    def flushBatchRequests(self, compress):
        """
            Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
            associated with the connection.
        Arguments:
        compress -- Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
        """
        raise NotImplementedError("method 'flushBatchRequests' not implemented")

    def setCloseCallback(self, callback):
        """
            Set a close callback on the connection. The callback is called by the connection when it's closed. The callback
            is called from the Ice thread pool associated with the connection. If the callback needs more information about
            the closure, it can call Connection#throwException.
        Arguments:
        callback -- The close callback object.
        """
        raise NotImplementedError("method 'setCloseCallback' not implemented")

    def type(self):
        """
            Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
        Returns: The type of the connection.
        """
        raise NotImplementedError("method 'type' not implemented")

    def timeout(self):
        """
            Get the timeout for the connection.
        Returns: The connection's timeout.
        """
        raise NotImplementedError("method 'timeout' not implemented")

    def toString(self):
        """
            Return a description of the connection as human readable text, suitable for logging or error messages.
        Returns: The description of the connection as human readable text.
        """
        raise NotImplementedError("method 'toString' not implemented")

    def getInfo(self):
        """
            Returns the connection information.
        Returns: The connection information.
        """
        raise NotImplementedError("method 'getInfo' not implemented")

    def setBufferSize(self, rcvSize, sndSize):
        """
            Set the connection buffer receive/send size.
        Arguments:
        rcvSize -- The connection receive buffer size.
        sndSize -- The connection send buffer size.
        """
        raise NotImplementedError("method 'setBufferSize' not implemented")

    def throwException(self):
        """
        Throw an exception indicating the reason for connection closure. For example,
        CloseConnectionException is raised if the connection was closed gracefully, whereas
        ConnectionManuallyClosedException is raised if the connection was manually closed by
        the application. This operation does nothing if the connection is not yet closed.
        """
        raise NotImplementedError("method 'throwException' not implemented")
