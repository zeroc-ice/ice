# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class CompressBatch
        include Comparable

        def initialize(name, value)
            @name = name
            @value = value
        end

        def CompressBatch.from_int(val)
            @@_enumerators[val]
        end

        def to_s
            @name
        end

        def to_i
            @value
        end

        def <=>(other)
            other.is_a?(CompressBatch) or raise ArgumentError, "value must be a CompressBatch"
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def CompressBatch.each(&block)
            @@_enumerators.each_value(&block)
        end

        Yes = CompressBatch.new("Yes", 0)
        No = CompressBatch.new("No", 1)
        BasedOnProxy = CompressBatch.new("BasedOnProxy", 2)

        @@_enumerators = {0=>Yes, 1=>No, 2=>BasedOnProxy}

        def CompressBatch._enumerators
            @@_enumerators
        end

        private_class_method :new
    end

    class ConnectionInfo
        def initialize(underlying=nil, incoming=false, adapterName='', connectionId='')
            @underlying = underlying
            @incoming = incoming
            @adapterName = adapterName
            @connectionId = connectionId
        end

        attr_accessor :underlying, :incoming, :adapterName, :connectionId
    end

    class ConnectionClose
        include Comparable

        def initialize(name, value)
            @name = name
            @value = value
        end

        def ConnectionClose.from_int(val)
            @@_enumerators[val]
        end

        def to_s
            @name
        end

        def to_i
            @value
        end

        def <=>(other)
            other.is_a?(ConnectionClose) or raise ArgumentError, "value must be a ConnectionClose"
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def ConnectionClose.each(&block)
            @@_enumerators.each_value(&block)
        end

        Forcefully = ConnectionClose.new("Forcefully", 0)
        Gracefully = ConnectionClose.new("Gracefully", 1)
        GracefullyWithWait = ConnectionClose.new("GracefullyWithWait", 2)

        @@_enumerators = {0=>Forcefully, 1=>Gracefully, 2=>GracefullyWithWait}

        def ConnectionClose._enumerators
            @@_enumerators
        end

        private_class_method :new
    end

    class IPConnectionInfo < Ice::ConnectionInfo
        def initialize(underlying=nil, incoming=false, adapterName='', connectionId='', localAddress="", localPort=-1, remoteAddress="", remotePort=-1)
            super(underlying, incoming, adapterName, connectionId)
            @localAddress = localAddress
            @localPort = localPort
            @remoteAddress = remoteAddress
            @remotePort = remotePort
        end

        attr_accessor :localAddress, :localPort, :remoteAddress, :remotePort
    end

    class TCPConnectionInfo < Ice::IPConnectionInfo
        def initialize(underlying=nil, incoming=false, adapterName='', connectionId='', localAddress="", localPort=-1, remoteAddress="", remotePort=-1, rcvSize=0, sndSize=0)
            super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
            @rcvSize = rcvSize
            @sndSize = sndSize
        end

        attr_accessor :rcvSize, :sndSize
    end

    class UDPConnectionInfo < Ice::IPConnectionInfo
        def initialize(underlying=nil, incoming=false, adapterName='', connectionId='', localAddress="", localPort=-1, remoteAddress="", remotePort=-1, mcastAddress='', mcastPort=-1, rcvSize=0, sndSize=0)
            super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
            @mcastAddress = mcastAddress
            @mcastPort = mcastPort
            @rcvSize = rcvSize
            @sndSize = sndSize
        end

        attr_accessor :mcastAddress, :mcastPort, :rcvSize, :sndSize
    end

    class WSConnectionInfo < Ice::ConnectionInfo
        def initialize(underlying=nil, incoming=false, adapterName='', connectionId='', headers=nil)
            super(underlying, incoming, adapterName, connectionId)
            @headers = headers
        end

        attr_accessor :headers
    end
end
