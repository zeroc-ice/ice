# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice

    if not defined?(::Ice::EndpointInfo_Mixin)

        module ::Ice::EndpointInfo_Mixin
        end
        class EndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false)
                @underlying = underlying
                @timeout = timeout
                @compress = compress
            end

            attr_accessor :underlying, :timeout, :compress
        end

        if not defined?(::Ice::T_EndpointInfo)
            T_EndpointInfo = ::Ice::__declareLocalClass('::Ice::EndpointInfo')
        end

        T_EndpointInfo.defineClass(EndpointInfo, -1, false, nil, [
            ['underlying', ::Ice::T_EndpointInfo, false, 0],
            ['timeout', ::Ice::T_int, false, 0],
            ['compress', ::Ice::T_bool, false, 0]
        ])
    end

    if not defined?(::Ice::T_Endpoint)
        T_Endpoint = ::Ice::__declareLocalClass('::Ice::Endpoint')
    end

    if not defined?(::Ice::IPEndpointInfo_Mixin)

        module ::Ice::IPEndpointInfo_Mixin
        end
        class IPEndpointInfo < ::Ice::EndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='')
                super(underlying, timeout, compress)
                @host = host
                @port = port
                @sourceAddress = sourceAddress
            end

            attr_accessor :host, :port, :sourceAddress
        end

        if not defined?(::Ice::T_IPEndpointInfo)
            T_IPEndpointInfo = ::Ice::__declareLocalClass('::Ice::IPEndpointInfo')
        end

        T_IPEndpointInfo.defineClass(IPEndpointInfo, -1, false, ::Ice::T_EndpointInfo, [
            ['host', ::Ice::T_string, false, 0],
            ['port', ::Ice::T_int, false, 0],
            ['sourceAddress', ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::TCPEndpointInfo_Mixin)

        module ::Ice::TCPEndpointInfo_Mixin
        end
        class TCPEndpointInfo < ::Ice::IPEndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='')
                super(underlying, timeout, compress, host, port, sourceAddress)
            end
        end

        if not defined?(::Ice::T_TCPEndpointInfo)
            T_TCPEndpointInfo = ::Ice::__declareLocalClass('::Ice::TCPEndpointInfo')
        end

        T_TCPEndpointInfo.defineClass(TCPEndpointInfo, -1, false, ::Ice::T_IPEndpointInfo, [])
    end

    if not defined?(::Ice::UDPEndpointInfo_Mixin)

        module ::Ice::UDPEndpointInfo_Mixin
        end
        class UDPEndpointInfo < ::Ice::IPEndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false, host='', port=0, sourceAddress='', mcastInterface='', mcastTtl=0)
                super(underlying, timeout, compress, host, port, sourceAddress)
                @mcastInterface = mcastInterface
                @mcastTtl = mcastTtl
            end

            attr_accessor :mcastInterface, :mcastTtl
        end

        if not defined?(::Ice::T_UDPEndpointInfo)
            T_UDPEndpointInfo = ::Ice::__declareLocalClass('::Ice::UDPEndpointInfo')
        end

        T_UDPEndpointInfo.defineClass(UDPEndpointInfo, -1, false, ::Ice::T_IPEndpointInfo, [
            ['mcastInterface', ::Ice::T_string, false, 0],
            ['mcastTtl', ::Ice::T_int, false, 0]
        ])
    end

    if not defined?(::Ice::WSEndpointInfo_Mixin)

        module ::Ice::WSEndpointInfo_Mixin
        end
        class WSEndpointInfo < ::Ice::EndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false, resource='')
                super(underlying, timeout, compress)
                @resource = resource
            end

            attr_accessor :resource
        end

        if not defined?(::Ice::T_WSEndpointInfo)
            T_WSEndpointInfo = ::Ice::__declareLocalClass('::Ice::WSEndpointInfo')
        end

        T_WSEndpointInfo.defineClass(WSEndpointInfo, -1, false, ::Ice::T_EndpointInfo, [['resource', ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::OpaqueEndpointInfo_Mixin)

        module ::Ice::OpaqueEndpointInfo_Mixin
        end
        class OpaqueEndpointInfo < ::Ice::EndpointInfo

            def initialize(underlying=nil, timeout=0, compress=false, rawEncoding=::Ice::EncodingVersion.new, rawBytes=nil)
                super(underlying, timeout, compress)
                @rawEncoding = rawEncoding
                @rawBytes = rawBytes
            end

            attr_accessor :rawEncoding, :rawBytes
        end

        if not defined?(::Ice::T_OpaqueEndpointInfo)
            T_OpaqueEndpointInfo = ::Ice::__declareLocalClass('::Ice::OpaqueEndpointInfo')
        end

        T_OpaqueEndpointInfo.defineClass(OpaqueEndpointInfo, -1, false, ::Ice::T_EndpointInfo, [
            ['rawEncoding', ::Ice::T_EncodingVersion, false, 0],
            ['rawBytes', ::Ice::T_ByteSeq, false, 0]
        ])
    end
end
