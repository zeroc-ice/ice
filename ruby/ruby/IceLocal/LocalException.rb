# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class InitializationException < LocalException
    end

    class PluginInitializationException < LocalException
    end

    class AlreadyRegisteredException < LocalException
        def initialize(kindOfObject, id, msg)
            super(msg)
            @kindOfObject = kindOfObject
            @id = id
        end

        attr_reader :kindOfObject, :id
    end

    class NotRegisteredException < LocalException
        def initialize(kindOfObject, id, msg)
            super(msg)
            @kindOfObject = kindOfObject
            @id = id
        end

        attr_reader :kindOfObject, :id
    end

    class TwowayOnlyException < LocalException
    end

    class UnknownException < LocalException
    end

    class UnknownLocalException < UnknownException
    end

    class UnknownUserException < UnknownException
    end

    class CommunicatorDestroyedException < LocalException
    end

    class NoEndpointException < LocalException
    end

    class ParseException < LocalException
    end

    class RequestFailedException < LocalException
        def initialize(id, facet, operation, msg)
            super(msg)
            @id = id
            @facet = facet
            @operation = operation
        end

        attr_reader :id, :facet, :operation
    end

    class ObjectNotExistException < RequestFailedException
    end

    class FacetNotExistException < RequestFailedException
    end

    class OperationNotExistException < RequestFailedException
    end

    class SyscallException < LocalException
    end

    class SocketException < SyscallException
    end

    class ConnectFailedException < SocketException
    end

    class ConnectionRefusedException < ConnectFailedException
    end

    class ConnectionLostException < SocketException
    end

    class DNSException < SyscallException
    end

    class ConnectionIdleException < LocalException
    end

    class TimeoutException < LocalException
    end

    class ConnectTimeoutException < TimeoutException
    end

    class CloseTimeoutException < TimeoutException
    end

    class InvocationTimeoutException < TimeoutException
    end

    class InvocationCanceledException < LocalException
    end

    class ProtocolException < LocalException
    end

    class CloseConnectionException < ProtocolException
    end

    class ConnectionManuallyClosedException < LocalException
    end

    class DatagramLimitException < ProtocolException
    end

    class MarshalException < ProtocolException
    end

    class FeatureNotSupportedException < LocalException
    end

    class SecurityException < LocalException
    end

    class FixedProxyException < LocalException
    end
end
