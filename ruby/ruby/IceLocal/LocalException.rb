# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class InitializationException < LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::InitializationException'
        end

        attr_accessor :reason
    end

    class PluginInitializationException < LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::PluginInitializationException'
        end

        attr_accessor :reason
    end

    class AlreadyRegisteredException < LocalException
        def initialize(kindOfObject = '', id = '')
            @kindOfObject = kindOfObject
            @id = id
        end

        def to_s
            '::Ice::AlreadyRegisteredException'
        end

        attr_accessor :kindOfObject, :id
    end

    class NotRegisteredException < LocalException
        def initialize(kindOfObject = '', id = '')
            @kindOfObject = kindOfObject
            @id = id
        end

        def to_s
            '::Ice::NotRegisteredException'
        end

        attr_accessor :kindOfObject, :id
    end

    class TwowayOnlyException < LocalException
        def initialize(operation = '')
            @operation = operation
        end

        def to_s
            '::Ice::TwowayOnlyException'
        end

        attr_accessor :operation
    end

    class UnknownException < LocalException
        def initialize(unknown = '')
            @unknown = unknown
        end

        def to_s
            '::Ice::UnknownException'
        end

        attr_accessor :unknown
    end

    class UnknownLocalException < UnknownException
        def initialize(unknown = '')
            super(unknown)
        end

        def to_s
            '::Ice::UnknownLocalException'
        end
    end

    class UnknownUserException < UnknownException
        def initialize(unknown = '')
            super(unknown)
        end

        def to_s
            '::Ice::UnknownUserException'
        end
    end

    class CommunicatorDestroyedException < LocalException
        def initialize
        end

        def to_s
            '::Ice::CommunicatorDestroyedException'
        end
    end

    class ObjectAdapterDeactivatedException < LocalException
        def initialize(name = '')
            @name = name
        end

        def to_s
            '::Ice::ObjectAdapterDeactivatedException'
        end

        attr_accessor :name
    end

    class ObjectAdapterIdInUseException < LocalException
        def initialize(id = '')
            @id = id
        end

        def to_s
            '::Ice::ObjectAdapterIdInUseException'
        end

        attr_accessor :id
    end

    class NoEndpointException < LocalException
        def initialize(proxy = '')
            @proxy = proxy
        end

        def to_s
            '::Ice::NoEndpointException'
        end

        attr_accessor :proxy
    end

    class ParseException < LocalException
        def initialize(str = '')
            @str = str
        end

        def to_s
            '::Ice::ParseException'
        end

        attr_accessor :str
    end

    class IllegalIdentityException < LocalException
        def initialize
        end

        def to_s
            '::Ice::IllegalIdentityException'
        end
    end

    class IllegalServantException < LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::IllegalServantException'
        end

        attr_accessor :reason
    end

    class RequestFailedException < LocalException
        def initialize(id, facet, operation)
            @id = id
            @facet = facet
            @operation = operation
        end

        def to_s
            '::Ice::RequestFailedException'
        end

        attr_reader :id, :facet, :operation
    end

    class ObjectNotExistException < RequestFailedException
        def to_s
            '::Ice::ObjectNotExistException'
        end
    end

    class FacetNotExistException < RequestFailedException
        def to_s
            '::Ice::FacetNotExistException'
        end
    end

    class OperationNotExistException < RequestFailedException
        def to_s
            '::Ice::OperationNotExistException'
        end
    end

    class SyscallException < LocalException
        def initialize(error = 0)
            @error = error
        end

        def to_s
            '::Ice::SyscallException'
        end

        attr_accessor :error
    end

    class SocketException < SyscallException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::SocketException'
        end
    end

    class CFNetworkException < SocketException
        def initialize(error = 0, domain = '')
            super(error)
            @domain = domain
        end

        def to_s
            '::Ice::CFNetworkException'
        end

        attr_accessor :domain
    end

    class FileException < SyscallException
        def initialize(error = 0, path = '')
            super(error)
            @path = path
        end

        def to_s
            '::Ice::FileException'
        end

        attr_accessor :path
    end

    class ConnectFailedException < SocketException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectFailedException'
        end
    end

    class ConnectionRefusedException < ConnectFailedException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectionRefusedException'
        end
    end

    class ConnectionLostException < SocketException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectionLostException'
        end
    end

    class DNSException < LocalException
        def initialize(error = 0, host = '')
            @error = error
            @host = host
        end

        def to_s
            '::Ice::DNSException'
        end

        attr_accessor :error, :host
    end

    class ConnectionIdleException < LocalException
        def initialize
        end

        def to_s
            '::Ice::ConnectionIdleException'
        end
    end

    class TimeoutException < LocalException
        def initialize
        end

        def to_s
            '::Ice::TimeoutException'
        end
    end

    class ConnectTimeoutException < TimeoutException
        def initialize
        end

        def to_s
            '::Ice::ConnectTimeoutException'
        end
    end

    class CloseTimeoutException < TimeoutException
        def initialize
        end

        def to_s
            '::Ice::CloseTimeoutException'
        end
    end

    class InvocationTimeoutException < TimeoutException
        def initialize
        end

        def to_s
            '::Ice::InvocationTimeoutException'
        end
    end

    class InvocationCanceledException < LocalException
        def initialize
        end

        def to_s
            '::Ice::InvocationCanceledException'
        end
    end

    class ProtocolException < LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::ProtocolException'
        end

        attr_accessor :reason
    end

    class CloseConnectionException < ProtocolException
        def initialize(reason = '')
            super(reason)
        end

        def to_s
            '::Ice::CloseConnectionException'
        end
    end

    class ConnectionManuallyClosedException < LocalException
        def initialize(graceful = false)
            @graceful = graceful
        end

        def to_s
            '::Ice::ConnectionManuallyClosedException'
        end

        attr_accessor :graceful
    end

    class DatagramLimitException < ProtocolException
        def initialize(reason = '')
            super(reason)
        end

        def to_s
            '::Ice::DatagramLimitException'
        end
    end

    class MarshalException < LocalException # TODO: ProtocolException
    end

    class FeatureNotSupportedException < LocalException
        def initialize(unsupportedFeature = '')
            @unsupportedFeature = unsupportedFeature
        end

        def to_s
            '::Ice::FeatureNotSupportedException'
        end

        attr_accessor :unsupportedFeature
    end

    class SecurityException < LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::SecurityException'
        end

        attr_accessor :reason
    end

    class FixedProxyException < LocalException
        def initialize
        end

        def to_s
            '::Ice::FixedProxyException'
        end
    end
end
