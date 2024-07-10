# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class EndpointInfo
        attr_accessor :underlying, :timeout, :compress
    end

    class IPEndpointInfo < EndpointInfo
        attr_accessor :host, :port, :sourceAddress
    end

    class TCPEndpointInfo < IPEndpointInfo
    end

    class UDPEndpointInfo < IPEndpointInfo
        attr_accessor :mcastInterface, :mcastTtl
    end

    class WSEndpointInfo < EndpointInfo
        attr_accessor :resource
    end

    class OpaqueEndpointInfo < EndpointInfo
        attr_accessor :rawEncoding, :rawBytes
    end
end
