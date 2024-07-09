# Copyright (c) ZeroC, Inc. All rights reserved.

import Ice.Version_ice

class EndpointInfo(object):
    """
        Base class providing access to the endpoint details.
    Members:
    underlying --  The information of the underlying endpoint or null if there's no underlying endpoint.
    timeout --  The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
    compress --  Specifies whether or not compression should be used if available when using this endpoint.
    """

    def __init__(self, underlying=None, timeout=0, compress=False):
        if type(self) == EndpointInfo:
            raise RuntimeError("Ice.EndpointInfo is an abstract class")
        self.underlying = underlying
        self.timeout = timeout
        self.compress = compress

    def type(self):
        """
            Returns the type of the endpoint.
        Returns: The endpoint type.
        """
        raise NotImplementedError("method 'type' not implemented")

    def datagram(self):
        """
            Returns true if this endpoint is a datagram endpoint.
        Returns: True for a datagram endpoint.
        """
        raise NotImplementedError("method 'datagram' not implemented")

    def secure(self):
        """
        Returns: True for a secure endpoint.
        """
        raise NotImplementedError("method 'secure' not implemented")


class IPEndpointInfo(EndpointInfo):
    """
        Provides access to the address details of a IP endpoint.
    Members:
    host --  The host or address configured with the endpoint.
    port --  The port number.
    sourceAddress --  The source IP address.
    """

    def __init__(
        self,
        underlying=None,
        timeout=0,
        compress=False,
        host="",
        port=0,
        sourceAddress="",
    ):
        if type(self) == IPEndpointInfo:
            raise RuntimeError("Ice.IPEndpointInfo is an abstract class")
        EndpointInfo.__init__(self, underlying, timeout, compress)
        self.host = host
        self.port = port
        self.sourceAddress = sourceAddress


class TCPEndpointInfo(IPEndpointInfo):
    """
    Provides access to a TCP endpoint information.
    """

    def __init__(
        self,
        underlying=None,
        timeout=0,
        compress=False,
        host="",
        port=0,
        sourceAddress="",
    ):
        if type(self) == TCPEndpointInfo:
            raise RuntimeError("Ice.TCPEndpointInfo is an abstract class")
        IPEndpointInfo.__init__(
            self, underlying, timeout, compress, host, port, sourceAddress
        )


class UDPEndpointInfo(IPEndpointInfo):
    """
        Provides access to an UDP endpoint information.
    Members:
    mcastInterface --  The multicast interface.
    mcastTtl --  The multicast time-to-live (or hops).
    """

    def __init__(
        self,
        underlying=None,
        timeout=0,
        compress=False,
        host="",
        port=0,
        sourceAddress="",
        mcastInterface="",
        mcastTtl=0,
    ):
        if type(self) == UDPEndpointInfo:
            raise RuntimeError("Ice.UDPEndpointInfo is an abstract class")
        IPEndpointInfo.__init__(
            self, underlying, timeout, compress, host, port, sourceAddress
        )
        self.mcastInterface = mcastInterface
        self.mcastTtl = mcastTtl


class WSEndpointInfo(EndpointInfo):
        """
         Provides access to a WebSocket endpoint information.
        Members:
        resource --  The URI configured with the endpoint.
        """

        def __init__(self, underlying=None, timeout=0, compress=False, resource=""):
            if type(self) == WSEndpointInfo:
                raise RuntimeError("Ice.WSEndpointInfo is an abstract class")
            EndpointInfo.__init__(self, underlying, timeout, compress)
            self.resource = resource


class OpaqueEndpointInfo(EndpointInfo):
    """
        Provides access to the details of an opaque endpoint.
    Members:
    rawEncoding --  The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    rawBytes --   The raw encoding of the opaque endpoint.
    """

    def __init__(
        self,
        underlying=None,
        timeout=0,
        compress=False,
        rawEncoding=None,
        rawBytes=None,
    ):
        if type(self) == OpaqueEndpointInfo:
            raise RuntimeError("Ice.OpaqueEndpointInfo is an abstract class")
        EndpointInfo.__init__(self, underlying, timeout, compress)
        if rawEncoding is None:
            self.rawEncoding = Ice.EncodingVersion()
        else:
            self.rawEncoding = rawEncoding
        self.rawBytes = rawBytes
