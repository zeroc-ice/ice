# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class EndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false)
            @underlying = underlying
            @timeout = timeout
            @compress = compress
        end

        attr_accessor :underlying, :timeout, :compress
    end

    class IPEndpointInfo < Ice::EndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='')
            super(underlying, timeout, compress)
            @host = host
            @port = port
            @sourceAddress = sourceAddress
        end

        attr_accessor :host, :port, :sourceAddress
    end

    class TCPEndpointInfo < Ice::IPEndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='')
            super(underlying, timeout, compress, host, port, sourceAddress)
        end
    end

    class UDPEndpointInfo < Ice::IPEndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='', mcastInterface='', mcastTtl=0)
            super(underlying, timeout, compress, host, port, sourceAddress)
            @mcastInterface = mcastInterface
            @mcastTtl = mcastTtl
        end

        attr_accessor :mcastInterface, :mcastTtl
    end

    class WSEndpointInfo < Ice::EndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false, resource='')
            super(underlying, timeout, compress)
            @resource = resource
        end

        attr_accessor :resource
    end

    class OpaqueEndpointInfo < Ice::EndpointInfo
        def initialize(underlying=nil, timeout=0, compress=false, rawEncoding=Ice::EncodingVersion.new, rawBytes=nil)
            super(underlying, timeout, compress)
            @rawEncoding = rawEncoding
            @rawBytes = rawBytes
        end

        attr_accessor :rawEncoding, :rawBytes
    end
end
