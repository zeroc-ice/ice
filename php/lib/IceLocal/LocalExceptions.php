<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace
{
    require_once 'Ice/Identity.php';
}

namespace Ice
{
    class RequestFailedException extends LocalException
    {
        public Identity $id;
        public string $facet;
        public string $operation;
    }

    final class ObjectNotExistException extends RequestFailedException {}

    final class FacetNotExistException extends RequestFailedException {}

    final class OperationNotExistException extends RequestFailedException {}

    final class InitializationException extends LocalException {}

    final class PluginInitializationException extends LocalException {}

    final class AlreadyRegisteredException extends LocalException
    {
        public string $kindOfObject;
        public string $id;
    }

    final class NotRegisteredException extends LocalException
    {
        public string $kindOfObject;
        public string $id;
    }

    final class TwowayOnlyException extends LocalException {}

    class UnknownException extends LocalException {}

    final class UnknownLocalException extends UnknownException {}

    final class UnknownUserException extends UnknownException {}

    final class CommunicatorDestroyedException extends LocalException {}

    final class ObjectAdapterDeactivatedException extends LocalException {}

    final class ObjectAdapterIdInUseException extends LocalException {}

    final class NoEndpointException extends LocalException {}

    final class ParseException extends LocalException {}

    class SyscallException extends LocalException {}

    class SocketException extends SyscallException {}

    class ConnectFailedException extends SocketException {}

    final class ConnectionRefusedException extends ConnectFailedException {}

    final class ConnectionLostException extends SocketException {}

    final class DNSException extends LocalException {}

    final class ConnectionIdleException extends LocalException {}

    class TimeoutException extends LocalException {}

    final class ConnectTimeoutException extends TimeoutException {}

    final class CloseTimeoutException extends TimeoutException {}

    final class InvocationTimeoutException extends TimeoutException {}

    final class InvocationCanceledException extends LocalException {}

    class ProtocolException extends LocalException {}

    final class CloseConnectionException extends ProtocolException {}

    final class ConnectionManuallyClosedException extends LocalException {}

    final class DatagramLimitException extends ProtocolException {}

    final class MarshalException extends ProtocolException {}

    final class SecurityException extends LocalException {}

    final class FixedProxyException extends LocalException {}
}

?>
