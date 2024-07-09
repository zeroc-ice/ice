# Copyright (c) ZeroC, Inc. All rights reserved.

class ConnectionInfo(object):
    """
        Base class providing access to the connection details.
    Members:
    underlying --  The information of the underlying transport or null if there's no underlying transport.
    incoming --  Whether or not the connection is an incoming or outgoing connection.
    adapterName --   The name of the adapter associated with the connection.
    connectionId --  The connection id.
    """

    def __init__(
        self, underlying=None, incoming=False, adapterName="", connectionId=""
    ):
        self.underlying = underlying
        self.incoming = incoming
        self.adapterName = adapterName
        self.connectionId = connectionId

class IPConnectionInfo(ConnectionInfo):
    """
        Provides access to the connection details of an IP connection
    Members:
    localAddress --  The local address.
    localPort --  The local port.
    remoteAddress --  The remote address.
    remotePort --  The remote port.
    """

    def __init__(
        self,
        underlying=None,
        incoming=False,
        adapterName="",
        connectionId="",
        localAddress="",
        localPort=-1,
        remoteAddress="",
        remotePort=-1,
    ):
        ConnectionInfo.__init__(
            self, underlying, incoming, adapterName, connectionId
        )
        self.localAddress = localAddress
        self.localPort = localPort
        self.remoteAddress = remoteAddress
        self.remotePort = remotePort


class TCPConnectionInfo(IPConnectionInfo):
    """
        Provides access to the connection details of a TCP connection
    Members:
    rcvSize --  The connection buffer receive size.
    sndSize --  The connection buffer send size.
    """

    def __init__(
        self,
        underlying=None,
        incoming=False,
        adapterName="",
        connectionId="",
        localAddress="",
        localPort=-1,
        remoteAddress="",
        remotePort=-1,
        rcvSize=0,
        sndSize=0,
    ):
        IPConnectionInfo.__init__(
            self,
            underlying,
            incoming,
            adapterName,
            connectionId,
            localAddress,
            localPort,
            remoteAddress,
            remotePort,
        )
        self.rcvSize = rcvSize
        self.sndSize = sndSize


class UDPConnectionInfo(IPConnectionInfo):
    """
        Provides access to the connection details of a UDP connection
    Members:
    mcastAddress --  The multicast address.
    mcastPort --  The multicast port.
    rcvSize --  The connection buffer receive size.
    sndSize --  The connection buffer send size.
    """

    def __init__(
        self,
        underlying=None,
        incoming=False,
        adapterName="",
        connectionId="",
        localAddress="",
        localPort=-1,
        remoteAddress="",
        remotePort=-1,
        mcastAddress="",
        mcastPort=-1,
        rcvSize=0,
        sndSize=0,
    ):
        IPConnectionInfo.__init__(
            self,
            underlying,
            incoming,
            adapterName,
            connectionId,
            localAddress,
            localPort,
            remoteAddress,
            remotePort,
        )
        self.mcastAddress = mcastAddress
        self.mcastPort = mcastPort
        self.rcvSize = rcvSize
        self.sndSize = sndSize

class WSConnectionInfo(ConnectionInfo):
    """
        Provides access to the connection details of a WebSocket connection
    Members:
    headers --  The headers from the HTTP upgrade request.
    """

    def __init__(
        self,
        underlying=None,
        incoming=False,
        adapterName="",
        connectionId="",
        headers=None,
    ):
        ConnectionInfo.__init__(
            self, underlying, incoming, adapterName, connectionId
        )
        self.headers = headers
