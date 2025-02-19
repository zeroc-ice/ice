# Copyright (c) ZeroC, Inc.

# This file provides all the exceptions derived from Ice::LocalException. Application code should not throw any of
# these exceptions.

module Ice
    #
    # The 7 (8 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice
    # reply message. Other local exceptions can't be marshaled.
    #

    class DispatchException < LocalException
        def initialize(replyStatus, msg)
            super(msg)
            @replyStatus = replyStatus
        end

        attr_reader :replyStatus
    end

    class RequestFailedException < DispatchException
        def initialize(replyStatus, id, facet, operation, msg)
            super(replyStatus, msg)
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

    class UnknownException < DispatchException
    end

    class UnknownLocalException < UnknownException
    end

    class UnknownUserException < UnknownException
    end

    #
    # Protocol exceptions
    #

    class ProtocolException < LocalException
    end

    class CloseConnectionException < ProtocolException
    end

    class DatagramLimitException < ProtocolException
    end

    class MarshalException < ProtocolException
    end

    #
    # Timeout exceptions
    #

    class TimeoutException < LocalException
    end

    class ConnectTimeoutException < TimeoutException
    end

    class CloseTimeoutException < TimeoutException
    end

    class InvocationTimeoutException < TimeoutException
    end

    #
    # Syscall exceptions
    #

    class SyscallException < LocalException
    end

    class DNSException < SyscallException
    end

    #
    # Socket exceptions
    #

    class SocketException < SyscallException
    end

    class ConnectFailedException < SocketException
    end

    class ConnectionLostException < SocketException
    end

    class ConnectionRefusedException < ConnectFailedException
    end

    #
    # Other leaf local exceptions in alphabetical order.
    #

    class AlreadyRegisteredException < LocalException
        def initialize(kindOfObject, id, msg)
            super(msg)
            @kindOfObject = kindOfObject
            @id = id
        end

        attr_reader :kindOfObject, :id
    end

    class CommunicatorDestroyedException < LocalException
    end

    class ConnectionAbortedException < LocalException
    # We don't map closedByApplication because it's very difficult to produce a ConnectionAbortedException or
    # ConnectionClosedException without AMI.
    end

    class ConnectionClosedException < LocalException
    end

    class FeatureNotSupportedException < LocalException
    end

    class FixedProxyException < LocalException
    end

    class InitializationException < LocalException
    end

    class InvocationCanceledException < LocalException
    end

    class NoEndpointException < LocalException
    end

    class NotRegisteredException < LocalException
        def initialize(kindOfObject, id, msg)
            super(msg)
            @kindOfObject = kindOfObject
            @id = id
        end

        attr_reader :kindOfObject, :id
    end

    class ParseException < LocalException
    end

    class PluginInitializationException < LocalException
    end

    class SecurityException < LocalException
    end

    class TwowayOnlyException < LocalException
    end

    class PropertyException < LocalException
    end
end
