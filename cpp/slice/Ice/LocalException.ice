// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_ICE
#define ICE_LOCAL_EXCEPTION_ICE

module Ice
{

/**
 *
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <literal>Ice::Exception</literal>.
 *
 **/
local exception UnknownException
{
};

/**
 *
 * This exception is raised if the Ice library version doesn't match
 * the Ice header files version.
 *
 **/
local exception VersionMismatchException
{
};

/**
 *
 * This exception is raised if the [Communicator] has been destroyed.
 *
 * @see Communicator::destroy
 *
 **/
local exception CommunicatorDestroyedException
{
};

/**
 *
 * This exception is raised if an attempt is made to use a deactivated
 * [ObjectAdapter].
 *
 * @see ObjectAdapter::deactivate
 * @see Communicator::shutdown
 *
 **/
local exception ObjectAdapterDeactivatedException
{
};

/**
 *
 * This exception is raised if not suitable endpoint is available in
 * an object reference.
 *
 **/
local exception NoEndpointException
{
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * endpoint.
 *
 **/
local exception EndpointParseException
{
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * object reference.
 *
 **/
local exception ReferenceParseException
{
};

/**
 *
 * This exception is raised if an operation call using a Proxy
 * resulted in a location forward to an object reference that doesn't
 * match the Proxy's identity.
 *
 **/
local exception ReferenceIdentityException
{
};

/**
 *
 * This exception is raised if an object does not exist on the server.
 *
 * @see ObjectAdapter::add
 * @see ObjectAdapter::addServantLocator
 *
 **/
local exception ObjectNotExistException
{
};

/**
 *
 * This exception is raised if an operation for a given object does
 * not exist on the server. Typically this is caused by either the
 * client or the server using an outdated Slice specification.
 *
 **/
local exception OperationNotExistException
{
};

/**
 *
 * This exception is raised if no suitable factory for a Servant was
 * found.
 *
 * @see ServantFactory
 * @see Communicator::addServantFactory
 * @see Communicator::removeServantFactory
 * @see Communicator::findServantFactory
 *
 **/
local exception NoServantFactoryException
{
};

/**
 *
 * This exception is raised if a system error occurred in the server
 * or client process. There are many possible causes for such a system
 * exception. For details on the cause, [SystemException::errno]
 * should be inspected.
 *
 **/
local exception SystemException
{
    /**
     *
     * The error number describing the system exception. For C++ and
     * Unix, this is equivalent to <literal>errno</literal> or
     * <literal>h_errno</literal>. For C++ and Windows, this is the
     * value returned by <literal>GetLastError()</literal> or
     * <literal>WSAGetLastError()</literal>.
     *
     **/
    int error; // Don't use errno, as errno is usually a macro.
};

/**
 *
 * This exception is a specialization of [SystemException] for socket
 * errors.
 *
 **/
local exception SocketException extends SystemException
{
};

/**
 *
 * This exception is a specialization of [SocketException] for timeout
 * conditions.
 *
 **/
local exception TimeoutException extends SocketException
{
};

/**
 *
 * This exception is a specialization of [TimeoutException] for
 * connection timeout conditions.
 *
 **/
local exception ConnectTimeoutException extends TimeoutException
{
};

/**
 *
 * This exception is a specialization of [SocketException] for
 * connection failures.
 *
 **/
local exception ConnectFailedException extends SocketException
{
};

/**
 *
 * This exception is a specialization of [SocketException], indicating
 * a lost connection.
 *
 **/
local exception ConnectionLostException extends SocketException
{
};

/**
 *
 * This exception is a specialization of [SystemException], indicating
 * DNS problems.
 *
 **/
local exception DNSException extends SystemException
{
};

/**
 *
 * A generic exception base for all kinds of protocol error
 * conditions.
 *
 **/
local exception ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if no suitable servant factory was found during request
 * parameter unmarshaling.
 *
 * @see ServantFactory
 * @see Communicator::addServantFactory
 * @see Communicator::removeServantFactory
 * @see Communicator::findServantFactory
 *
 **/
local exception ServantUnmarshalException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating a malformed string encoding.
 *
 **/
local exception StringEncodingException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if the system-specific memory limit is exceeded during
 * marshaling or unmarshaling parameters.
 *
 **/
local exception MemoryLimitException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating a malformed data encapsulation.
 *
 **/
local exception EncapsulationException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported protocol version has been
 * encountered.
 *
 **/
local exception UnsupportedProtocolException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported data encoding version has been
 * encountered.
 *
 **/
local exception UnsupportedEncodingException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an invalid protocol message has been received.
 *
 **/
local exception InvalidMessageException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown protocol message has been received.
 *
 **/
local exception UnknownMessageException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a response for an unknown request id has been
 * received.
 *
 **/
local exception UnknownRequestIdException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown reply status been received.
 *
 **/
local exception UnknownReplyStatusException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an exception has been closed by the peer.
 *
 **/
local exception CloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a batch request has been aborted.
 *
 **/
local exception AbortBatchRequestException extends ProtocolException
{
};

};

#endif
