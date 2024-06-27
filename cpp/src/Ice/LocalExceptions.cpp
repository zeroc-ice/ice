//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/LocalExceptions.h"
#include "Ice/Initialize.h"
#include "Ice/StringUtil.h"
#include "Network.h"
#include "RequestFailedMessage.h"

#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    inline string socketErrorToString(int error)
    {
        if (error == 0)
        {
            return "unknown error";
        }
        return IceInternal::errorToString(error);
    }
}

namespace
{
    inline std::string createRequestFailedMessage(const char* typeName)
    {
        ostringstream os;
        os << "dispatch failed with " << typeName;
        return os.str();
    }
}

// Can't move id/facet/operation because the evaluation order is unspecified.
// https://en.cppreference.com/w/cpp/language/eval_order
Ice::ObjectNotExistException::ObjectNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          createRequestFailedMessage("ObjectNotExistException", id, facet, operation),
          id,
          facet,
          operation)
{
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* file, int line)
    : RequestFailedException(file, line, createRequestFailedMessage("ObjectNotExistException"))
{
}

const char*
Ice::ObjectNotExistException::ice_id() const noexcept
{
    return "::Ice::ObjectNotExistException";
}

Ice::FacetNotExistException::FacetNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          createRequestFailedMessage("FacetNotExistException", id, facet, operation),
          id,
          facet,
          operation)
{
}

Ice::FacetNotExistException::FacetNotExistException(const char* file, int line)
    : RequestFailedException(file, line, createRequestFailedMessage("FacetNotExistException"))
{
}

const char*
Ice::FacetNotExistException::ice_id() const noexcept
{
    return "::Ice::FacetNotExistException";
}

Ice::OperationNotExistException::OperationNotExistException(
    const char* file,
    int line,
    Identity id,
    string facet,
    string operation)
    : RequestFailedException(
          file,
          line,
          createRequestFailedMessage("OperationNotExistException", id, facet, operation),
          id,
          facet,
          operation)
{
}

Ice::OperationNotExistException::OperationNotExistException(const char* file, int line)
    : RequestFailedException(file, line, createRequestFailedMessage("OperationNotExistException"))
{
}

const char*
Ice::OperationNotExistException::ice_id() const noexcept
{
    return "::Ice::OperationNotExistException";
}

const char*
Ice::UnknownException::ice_id() const noexcept
{
    return "::Ice::UnknownException";
}

const char*
Ice::UnknownLocalException::ice_id() const noexcept
{
    return "::Ice::UnknownLocalException";
}

const char*
Ice::UnknownUserException::ice_id() const noexcept
{
    return "::Ice::UnknownUserException";
}

const char*
Ice::ParseException::ice_id() const noexcept
{
    return "::Ice::ParseException";
}

const char*
Ice::ProtocolException::ice_id() const noexcept
{
    return "::Ice::ProtocolException";
}

const char*
Ice::CloseConnectionException::ice_id() const noexcept
{
    return "::Ice::CloseConnectionException";
}

const char*
Ice::DatagramLimitException::ice_id() const noexcept
{
    return "::Ice::DatagramLimitException";
}

const char*
Ice::MarshalException::ice_id() const noexcept
{
    return "::Ice::MarshalException";
}

const char*
Ice::FeatureNotSupportedException::ice_id() const noexcept
{
    return "::Ice::FeatureNotSupportedException";
}

//
// Below: not refactored yet
//
const char*
Ice::InitializationException::ice_id() const noexcept
{
    return "::Ice::InitializationException";
}

const char*
Ice::PluginInitializationException::ice_id() const noexcept
{
    return "::Ice::PluginInitializationException";
}

const char*
Ice::AlreadyRegisteredException::ice_id() const noexcept
{
    return "::Ice::AlreadyRegisteredException";
}

const char*
Ice::NotRegisteredException::ice_id() const noexcept
{
    return "::Ice::NotRegisteredException";
}

const char*
Ice::TwowayOnlyException::ice_id() const noexcept
{
    return "::Ice::TwowayOnlyException";
}

const char*
Ice::CloneNotImplementedException::ice_id() const noexcept
{
    return "::Ice::CloneNotImplementedException";
}

const char*
Ice::VersionMismatchException::ice_id() const noexcept
{
    return "::Ice::VersionMismatchException";
}

const char*
Ice::CommunicatorDestroyedException::ice_id() const noexcept
{
    return "::Ice::CommunicatorDestroyedException";
}

const char*
Ice::ObjectAdapterDeactivatedException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterDeactivatedException";
}

const char*
Ice::ObjectAdapterIdInUseException::ice_id() const noexcept
{
    return "::Ice::ObjectAdapterIdInUseException";
}

const char*
Ice::NoEndpointException::ice_id() const noexcept
{
    return "::Ice::NoEndpointException";
}

const char*
Ice::IllegalIdentityException::ice_id() const noexcept
{
    return "::Ice::IllegalIdentityException";
}

const char*
Ice::IllegalServantException::ice_id() const noexcept
{
    return "::Ice::IllegalServantException";
}

Ice::SyscallException::SyscallException(const char* file, int line) noexcept
    : LocalException(file, line),
#ifdef _WIN32
      error(GetLastError())
#else
      error(errno)
#endif
{
}

const char*
Ice::SyscallException::ice_id() const noexcept
{
    return "::Ice::SyscallException";
}

const char*
Ice::SocketException::ice_id() const noexcept
{
    return "::Ice::SocketException";
}

const char*
Ice::CFNetworkException::ice_id() const noexcept
{
    return "::Ice::CFNetworkException";
}

const char*
Ice::FileException::ice_id() const noexcept
{
    return "::Ice::FileException";
}

const char*
Ice::ConnectFailedException::ice_id() const noexcept
{
    return "::Ice::ConnectFailedException";
}

const char*
Ice::ConnectionRefusedException::ice_id() const noexcept
{
    return "::Ice::ConnectionRefusedException";
}

const char*
Ice::ConnectionLostException::ice_id() const noexcept
{
    return "::Ice::ConnectionLostException";
}

const char*
Ice::DNSException::ice_id() const noexcept
{
    return "::Ice::DNSException";
}

const char*
Ice::ConnectionClosedException::ice_id() const noexcept
{
    return "::Ice::ConnectionClosedException";
}

const char*
Ice::ConnectionIdleException::ice_id() const noexcept
{
    return "::Ice::ConnectionIdleException";
}

const char*
Ice::TimeoutException::ice_id() const noexcept
{
    return "::Ice::TimeoutException";
}

const char*
Ice::ConnectTimeoutException::ice_id() const noexcept
{
    return "::Ice::ConnectTimeoutException";
}

const char*
Ice::CloseTimeoutException::ice_id() const noexcept
{
    return "::Ice::CloseTimeoutException";
}

const char*
Ice::InvocationTimeoutException::ice_id() const noexcept
{
    return "::Ice::InvocationTimeoutException";
}

const char*
Ice::InvocationCanceledException::ice_id() const noexcept
{
    return "::Ice::InvocationCanceledException";
}

const char*
Ice::ConnectionManuallyClosedException::ice_id() const noexcept
{
    return "::Ice::ConnectionManuallyClosedException";
}

const char*
Ice::SecurityException::ice_id() const noexcept
{
    return "::Ice::SecurityException";
}

const char*
Ice::FixedProxyException::ice_id() const noexcept
{
    return "::Ice::FixedProxyException";
}

// ice_print for all exceptions

void
Ice::InitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninitialization exception";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::VersionMismatchException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter `" << name << "' deactivated";
}

void
Ice::ObjectAdapterIdInUseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter with id `" << id << "' is already in use";
}

void
Ice::NoEndpointException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno suitable endpoint available for proxy `" << proxy << "'";
}

void
Ice::IllegalIdentityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nan identity with an empty name is not allowed";
}

void
Ice::IllegalServantException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal servant: `" << reason << "'";
}

void
Ice::SyscallException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if (error != 0)
    {
        out << ":\nsyscall exception: " << IceInternal::errorToString(error);
    }
}

void
Ice::SocketException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsocket exception: " << socketErrorToString(error);
}

void
Ice::FileException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfile exception: ";
    if (error == 0)
    {
        out << "couldn't open file";
    }
    else
    {
        out << IceInternal::errorToString(error);
    }
    if (!path.empty())
    {
        out << "\npath: " << path;
    }
}

void
Ice::ConnectFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnect failed: " << socketErrorToString(error);
}

void
Ice::ConnectionRefusedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection refused: " << socketErrorToString(error);
}

void
Ice::ConnectionLostException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection lost: ";
    if (error == 0)
    {
        out << "recv() returned zero";
    }
    else
    {
        out << socketErrorToString(error);
    }
}

void
Ice::DNSException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nDNS error: ";
    out << errorToStringDNS(error);
    out << "\nhost: " << host;
}

void
Ice::ConnectionClosedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}

void
Ice::ConnectionIdleException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection aborted by the idle check";
}

void
Ice::TimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::CloseTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while closing a connection";
}

void
Ice::InvocationTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation has timed out";
}

void
Ice::InvocationCanceledException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation canceled";
}

void
Ice::ConnectionManuallyClosedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection manually closed (" << (graceful ? "gracefully" : "forcefully") << ")";
}

void
Ice::PluginInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nplug-in initialization failed";
    if (!reason.empty())
    {
        out << ": " << reason;
    }
}

void
Ice::AlreadyRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << kindOfObject << " with id `" << id << "' is already registered";
}

void
Ice::NotRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno " << kindOfObject << " with id `" << id << "' is registered";
}

void
Ice::TwowayOnlyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation `" << operation << "' can only be invoked as a twoway request";
}

void
Ice::CloneNotImplementedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nice_clone() must be implemented in classes derived from abstract base classes";
}

void
Ice::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity exception";
    if (!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::FixedProxyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfixed proxy exception";
}

void
Ice::CFNetworkException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nnetwork exception: domain: " << domain << " error: " << error;
}
