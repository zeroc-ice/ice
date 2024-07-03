# encoding: utf-8
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require_relative '../Ice/Identity.rb'

module Ice
    class InitializationException < Ice::LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::InitializationException'
        end

        attr_accessor :reason
    end

    class PluginInitializationException < Ice::LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::PluginInitializationException'
        end

        attr_accessor :reason
    end

    class AlreadyRegisteredException < Ice::LocalException
        def initialize(kindOfObject = '', id = '')
            @kindOfObject = kindOfObject
            @id = id
        end

        def to_s
            '::Ice::AlreadyRegisteredException'
        end

        attr_accessor :kindOfObject, :id
    end

    class NotRegisteredException < Ice::LocalException
        def initialize(kindOfObject = '', id = '')
            @kindOfObject = kindOfObject
            @id = id
        end

        def to_s
            '::Ice::NotRegisteredException'
        end

        attr_accessor :kindOfObject, :id
    end

    class TwowayOnlyException < Ice::LocalException
        def initialize(operation = '')
            @operation = operation
        end

        def to_s
            '::Ice::TwowayOnlyException'
        end

        attr_accessor :operation
    end

    class UnknownException < Ice::LocalException
        def initialize(unknown = '')
            @unknown = unknown
        end

        def to_s
            '::Ice::UnknownException'
        end

        attr_accessor :unknown
    end

    class UnknownLocalException < ::Ice::UnknownException
        def initialize(unknown = '')
            super(unknown)
        end

        def to_s
            '::Ice::UnknownLocalException'
        end
    end

    class UnknownUserException < ::Ice::UnknownException
        def initialize(unknown = '')
            super(unknown)
        end

        def to_s
            '::Ice::UnknownUserException'
        end
    end

    class CommunicatorDestroyedException < Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::CommunicatorDestroyedException'
        end
    end

    class ObjectAdapterDeactivatedException < Ice::LocalException
        def initialize(name = '')
            @name = name
        end

        def to_s
            '::Ice::ObjectAdapterDeactivatedException'
        end

        attr_accessor :name
    end

    class ObjectAdapterIdInUseException < Ice::LocalException
        def initialize(id = '')
            @id = id
        end

        def to_s
            '::Ice::ObjectAdapterIdInUseException'
        end

        attr_accessor :id
    end

    class NoEndpointException < Ice::LocalException
        def initialize(proxy = '')
            @proxy = proxy
        end

        def to_s
            '::Ice::NoEndpointException'
        end

        attr_accessor :proxy
    end

    class ParseException < Ice::LocalException
        def initialize(str = '')
            @str = str
        end

        def to_s
            '::Ice::ParseException'
        end

        attr_accessor :str
    end

    class IllegalIdentityException < Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::IllegalIdentityException'
        end
    end

    class IllegalServantException < Ice::LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::IllegalServantException'
        end

        attr_accessor :reason
    end

    class RequestFailedException < Ice::LocalException
        def initialize(id, facet, operation)
            @id = id
            @facet = facet
            @operation = operation
        end

        def to_s
            '::Ice::RequestFailedException'
        end

        attr_accessor :id, :facet, :operation
    end

    class ObjectNotExistException < ::Ice::RequestFailedException
        def to_s
            '::Ice::ObjectNotExistException'
        end
    end

    class FacetNotExistException < ::Ice::RequestFailedException
        def to_s
            '::Ice::FacetNotExistException'
        end
    end

    class OperationNotExistException < ::Ice::RequestFailedException
        def to_s
            '::Ice::OperationNotExistException'
        end
    end

    class SyscallException < Ice::LocalException
        def initialize(error = 0)
            @error = error
        end

        def to_s
            '::Ice::SyscallException'
        end

        attr_accessor :error
    end

    class SocketException < ::Ice::SyscallException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::SocketException'
        end
    end

    class CFNetworkException < ::Ice::SocketException
        def initialize(error = 0, domain = '')
            super(error)
            @domain = domain
        end

        def to_s
            '::Ice::CFNetworkException'
        end

        attr_accessor :domain
    end

    class FileException < ::Ice::SyscallException
        def initialize(error = 0, path = '')
            super(error)
            @path = path
        end

        def to_s
            '::Ice::FileException'
        end

        attr_accessor :path
    end

    class ConnectFailedException < ::Ice::SocketException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectFailedException'
        end
    end

    class ConnectionRefusedException < ::Ice::ConnectFailedException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectionRefusedException'
        end
    end

    class ConnectionLostException < ::Ice::SocketException
        def initialize(error = 0)
            super(error)
        end

        def to_s
            '::Ice::ConnectionLostException'
        end
    end

    class DNSException < Ice::LocalException
        def initialize(error = 0, host = '')
            @error = error
            @host = host
        end

        def to_s
            '::Ice::DNSException'
        end

        attr_accessor :error, :host
    end

    class ConnectionIdleException < ::Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::ConnectionIdleException'
        end
    end

    class TimeoutException < Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::TimeoutException'
        end
    end

    class ConnectTimeoutException < ::Ice::TimeoutException
        def initialize
        end

        def to_s
            '::Ice::ConnectTimeoutException'
        end
    end

    class CloseTimeoutException < ::Ice::TimeoutException
        def initialize
        end

        def to_s
            '::Ice::CloseTimeoutException'
        end
    end

    class InvocationTimeoutException < ::Ice::TimeoutException
        def initialize
        end

        def to_s
            '::Ice::InvocationTimeoutException'
        end
    end

    class InvocationCanceledException < Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::InvocationCanceledException'
        end
    end

    class ProtocolException < Ice::LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::ProtocolException'
        end

        attr_accessor :reason
    end

    class CloseConnectionException < ::Ice::ProtocolException
        def initialize(reason = '')
            super(reason)
        end

        def to_s
            '::Ice::CloseConnectionException'
        end
    end

    class ConnectionManuallyClosedException < Ice::LocalException
        def initialize(graceful = false)
            @graceful = graceful
        end

        def to_s
            '::Ice::ConnectionManuallyClosedException'
        end

        attr_accessor :graceful
    end

    class DatagramLimitException < ::Ice::ProtocolException
        def initialize(reason = '')
            super(reason)
        end

        def to_s
            '::Ice::DatagramLimitException'
        end
    end

    class MarshalException < ::Ice::ProtocolException
        def initialize(reason = '')
            super(reason)
        end

        def to_s
            '::Ice::MarshalException'
        end
    end

    class FeatureNotSupportedException < Ice::LocalException
        def initialize(unsupportedFeature = '')
            @unsupportedFeature = unsupportedFeature
        end

        def to_s
            '::Ice::FeatureNotSupportedException'
        end

        attr_accessor :unsupportedFeature
    end

    class SecurityException < Ice::LocalException
        def initialize(reason = '')
            @reason = reason
        end

        def to_s
            '::Ice::SecurityException'
        end

        attr_accessor :reason
    end

    class FixedProxyException < Ice::LocalException
        def initialize
        end

        def to_s
            '::Ice::FixedProxyException'
        end
    end
end
