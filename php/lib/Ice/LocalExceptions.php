<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// This file contains all the exception classes derived from LocalException.
// All these exceptions are created from the C++ code.

namespace Ice;

//
// The 6 (7 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice reply
// message. Other local exceptions can't be marshaled.
//

class RequestFailedException extends LocalException
{
    public Identity $id;
    public string $facet;
    public string $operation;
}

final class ObjectNotExistException extends RequestFailedException {}

final class FacetNotExistException extends RequestFailedException {}

final class OperationNotExistException extends RequestFailedException {}

class UnknownException extends LocalException {}

final class UnknownLocalException extends UnknownException {}

final class UnknownUserException extends UnknownException {}

//
// Protocol exceptions
//

class ProtocolException extends LocalException {}

final class CloseConnectionException extends ProtocolException {}

final class DatagramLimitException extends ProtocolException {}

final class MarshalException extends ProtocolException {}

//
// Timeout exceptions
//

class TimeoutException extends LocalException {}

final class ConnectTimeoutException extends TimeoutException {}

final class CloseTimeoutException extends TimeoutException {}

final class InvocationTimeoutException extends TimeoutException {}

//
// Syscall exceptions
//

class SyscallException extends LocalException {}

final class DNSException extends SyscallException {}

//
// Socket exceptions
//

class SocketException extends SyscallException {}

class ConnectFailedException extends SocketException {}

final class ConnectionLostException extends SocketException {}

final class ConnectionRefusedException extends ConnectFailedException {}

//
// Other leaf local exceptions in alphabetical order.
//

final class AlreadyRegisteredException extends LocalException
{
    public string $kindOfObject;
    public string $id;
}

final class CommunicatorDestroyedException extends LocalException {}

// We don't map closedByApplication because it's very difficult to produce a ConnectionAbortedException or
// ConnectionClosedException without AMI.
final class ConnectionAbortedException extends LocalException {}

final class ConnectionClosedException extends LocalException {}

final class FixedProxyException extends LocalException {}

final class InitializationException extends LocalException {}

final class InvocationCanceledException extends LocalException {}

final class NoEndpointException extends LocalException {}

final class NotRegisteredException extends LocalException
{
    public string $kindOfObject;
    public string $id;
}

final class ParseException extends LocalException {}

final class PluginInitializationException extends LocalException {}

final class SecurityException extends LocalException {}

final class TwowayOnlyException extends LocalException {}

final class PropertyException extends LocalException {}
