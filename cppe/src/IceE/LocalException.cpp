// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice-E version 1.0.0
// Generated from file `LocalException.ice'

#include <IceE/LocalException.h>
#include <IceE/BasicStream.h>
#ifndef ICEE_PURE_CLIENT
#  include <IceE/Object.h>
#endif

#ifndef ICEE_IGNORE_VERSION
#   if ICEE_INT_VERSION / 100 != 100
#       error IceE version mismatch!
#   endif
#   if ICEE_INT_VERSION % 100 < 0
#       error IceE patch level mismatch!
#   endif
#endif

Ice::AlreadyRegisteredException::AlreadyRegisteredException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::AlreadyRegisteredException::AlreadyRegisteredException(const char* __file, int __line, const ::std::string& __kindOfObject, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    kindOfObject(__kindOfObject),
    id(__id)
{
}

static const char* __Ice__AlreadyRegisteredException_name = "Ice::AlreadyRegisteredException";

const ::std::string
Ice::AlreadyRegisteredException::ice_name() const
{
    return __Ice__AlreadyRegisteredException_name;
}

::Ice::Exception*
Ice::AlreadyRegisteredException::ice_clone() const
{
    return new AlreadyRegisteredException(*this);
}

void
Ice::AlreadyRegisteredException::ice_throw() const
{
    throw *this;
}

Ice::NotRegisteredException::NotRegisteredException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::NotRegisteredException::NotRegisteredException(const char* __file, int __line, const ::std::string& __kindOfObject, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    kindOfObject(__kindOfObject),
    id(__id)
{
}

static const char* __Ice__NotRegisteredException_name = "Ice::NotRegisteredException";

const ::std::string
Ice::NotRegisteredException::ice_name() const
{
    return __Ice__NotRegisteredException_name;
}

::Ice::Exception*
Ice::NotRegisteredException::ice_clone() const
{
    return new NotRegisteredException(*this);
}

void
Ice::NotRegisteredException::ice_throw() const
{
    throw *this;
}

Ice::TwowayOnlyException::TwowayOnlyException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::TwowayOnlyException::TwowayOnlyException(const char* __file, int __line, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    operation(__operation)
{
}

static const char* __Ice__TwowayOnlyException_name = "Ice::TwowayOnlyException";

const ::std::string
Ice::TwowayOnlyException::ice_name() const
{
    return __Ice__TwowayOnlyException_name;
}

::Ice::Exception*
Ice::TwowayOnlyException::ice_clone() const
{
    return new TwowayOnlyException(*this);
}

void
Ice::TwowayOnlyException::ice_throw() const
{
    throw *this;
}

Ice::CloneNotImplementedException::CloneNotImplementedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__CloneNotImplementedException_name = "Ice::CloneNotImplementedException";

const ::std::string
Ice::CloneNotImplementedException::ice_name() const
{
    return __Ice__CloneNotImplementedException_name;
}

::Ice::Exception*
Ice::CloneNotImplementedException::ice_clone() const
{
    return new CloneNotImplementedException(*this);
}

void
Ice::CloneNotImplementedException::ice_throw() const
{
    throw *this;
}

Ice::UnknownException::UnknownException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::UnknownException::UnknownException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    unknown(__unknown)
{
}

static const char* __Ice__UnknownException_name = "Ice::UnknownException";

const ::std::string
Ice::UnknownException::ice_name() const
{
    return __Ice__UnknownException_name;
}

::Ice::Exception*
Ice::UnknownException::ice_clone() const
{
    return new UnknownException(*this);
}

void
Ice::UnknownException::ice_throw() const
{
    throw *this;
}

Ice::UnknownLocalException::UnknownLocalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line)
#else
    ::Ice::UnknownException(__file, __line)
#endif
{
}

Ice::UnknownLocalException::UnknownLocalException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line, __unknown)
#else
    ::Ice::UnknownException(__file, __line, __unknown)
#endif
{
}

static const char* __Ice__UnknownLocalException_name = "Ice::UnknownLocalException";

const ::std::string
Ice::UnknownLocalException::ice_name() const
{
    return __Ice__UnknownLocalException_name;
}

::Ice::Exception*
Ice::UnknownLocalException::ice_clone() const
{
    return new UnknownLocalException(*this);
}

void
Ice::UnknownLocalException::ice_throw() const
{
    throw *this;
}

Ice::UnknownUserException::UnknownUserException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line)
#else
    ::Ice::UnknownException(__file, __line)
#endif
{
}

Ice::UnknownUserException::UnknownUserException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line, __unknown)
#else
    ::Ice::UnknownException(__file, __line, __unknown)
#endif
{
}

static const char* __Ice__UnknownUserException_name = "Ice::UnknownUserException";

const ::std::string
Ice::UnknownUserException::ice_name() const
{
    return __Ice__UnknownUserException_name;
}

::Ice::Exception*
Ice::UnknownUserException::ice_clone() const
{
    return new UnknownUserException(*this);
}

void
Ice::UnknownUserException::ice_throw() const
{
    throw *this;
}

Ice::VersionMismatchException::VersionMismatchException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__VersionMismatchException_name = "Ice::VersionMismatchException";

const ::std::string
Ice::VersionMismatchException::ice_name() const
{
    return __Ice__VersionMismatchException_name;
}

::Ice::Exception*
Ice::VersionMismatchException::ice_clone() const
{
    return new VersionMismatchException(*this);
}

void
Ice::VersionMismatchException::ice_throw() const
{
    throw *this;
}

Ice::CommunicatorDestroyedException::CommunicatorDestroyedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__CommunicatorDestroyedException_name = "Ice::CommunicatorDestroyedException";

const ::std::string
Ice::CommunicatorDestroyedException::ice_name() const
{
    return __Ice__CommunicatorDestroyedException_name;
}

::Ice::Exception*
Ice::CommunicatorDestroyedException::ice_clone() const
{
    return new CommunicatorDestroyedException(*this);
}

void
Ice::CommunicatorDestroyedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* __file, int __line, const ::std::string& __name) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    name(__name)
{
}

static const char* __Ice__ObjectAdapterDeactivatedException_name = "Ice::ObjectAdapterDeactivatedException";

const ::std::string
Ice::ObjectAdapterDeactivatedException::ice_name() const
{
    return __Ice__ObjectAdapterDeactivatedException_name;
}

::Ice::Exception*
Ice::ObjectAdapterDeactivatedException::ice_clone() const
{
    return new ObjectAdapterDeactivatedException(*this);
}

void
Ice::ObjectAdapterDeactivatedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* __file, int __line, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    id(__id)
{
}

static const char* __Ice__ObjectAdapterIdInUseException_name = "Ice::ObjectAdapterIdInUseException";

const ::std::string
Ice::ObjectAdapterIdInUseException::ice_name() const
{
    return __Ice__ObjectAdapterIdInUseException_name;
}

::Ice::Exception*
Ice::ObjectAdapterIdInUseException::ice_clone() const
{
    return new ObjectAdapterIdInUseException(*this);
}

void
Ice::ObjectAdapterIdInUseException::ice_throw() const
{
    throw *this;
}

Ice::NoEndpointException::NoEndpointException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::NoEndpointException::NoEndpointException(const char* __file, int __line, const ::std::string& __proxy) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    proxy(__proxy)
{
}

static const char* __Ice__NoEndpointException_name = "Ice::NoEndpointException";

const ::std::string
Ice::NoEndpointException::ice_name() const
{
    return __Ice__NoEndpointException_name;
}

::Ice::Exception*
Ice::NoEndpointException::ice_clone() const
{
    return new NoEndpointException(*this);
}

void
Ice::NoEndpointException::ice_throw() const
{
    throw *this;
}

Ice::EndpointParseException::EndpointParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::EndpointParseException::EndpointParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__EndpointParseException_name = "Ice::EndpointParseException";

const ::std::string
Ice::EndpointParseException::ice_name() const
{
    return __Ice__EndpointParseException_name;
}

::Ice::Exception*
Ice::EndpointParseException::ice_clone() const
{
    return new EndpointParseException(*this);
}

void
Ice::EndpointParseException::ice_throw() const
{
    throw *this;
}

Ice::IdentityParseException::IdentityParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::IdentityParseException::IdentityParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__IdentityParseException_name = "Ice::IdentityParseException";

const ::std::string
Ice::IdentityParseException::ice_name() const
{
    return __Ice__IdentityParseException_name;
}

::Ice::Exception*
Ice::IdentityParseException::ice_clone() const
{
    return new IdentityParseException(*this);
}

void
Ice::IdentityParseException::ice_throw() const
{
    throw *this;
}

Ice::ProxyParseException::ProxyParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::ProxyParseException::ProxyParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__ProxyParseException_name = "Ice::ProxyParseException";

const ::std::string
Ice::ProxyParseException::ice_name() const
{
    return __Ice__ProxyParseException_name;
}

::Ice::Exception*
Ice::ProxyParseException::ice_clone() const
{
    return new ProxyParseException(*this);
}

void
Ice::ProxyParseException::ice_throw() const
{
    throw *this;
}

Ice::IllegalIdentityException::IllegalIdentityException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::IllegalIdentityException::IllegalIdentityException(const char* __file, int __line, const ::Ice::Identity& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    id(__id)
{
}

static const char* __Ice__IllegalIdentityException_name = "Ice::IllegalIdentityException";

const ::std::string
Ice::IllegalIdentityException::ice_name() const
{
    return __Ice__IllegalIdentityException_name;
}

::Ice::Exception*
Ice::IllegalIdentityException::ice_clone() const
{
    return new IllegalIdentityException(*this);
}

void
Ice::IllegalIdentityException::ice_throw() const
{
    throw *this;
}

Ice::RequestFailedException::RequestFailedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::RequestFailedException::RequestFailedException(const char* __file, int __line, const ::Ice::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    id(__id),
    facet(__facet),
    operation(__operation)
{
}

static const char* __Ice__RequestFailedException_name = "Ice::RequestFailedException";

const ::std::string
Ice::RequestFailedException::ice_name() const
{
    return __Ice__RequestFailedException_name;
}

::Ice::Exception*
Ice::RequestFailedException::ice_clone() const
{
    return new RequestFailedException(*this);
}

void
Ice::RequestFailedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::Ice::RequestFailedException(__file, __line)
#endif
{
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* __file, int __line, const ::Ice::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::Ice::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__ObjectNotExistException_name = "Ice::ObjectNotExistException";

const ::std::string
Ice::ObjectNotExistException::ice_name() const
{
    return __Ice__ObjectNotExistException_name;
}

::Ice::Exception*
Ice::ObjectNotExistException::ice_clone() const
{
    return new ObjectNotExistException(*this);
}

void
Ice::ObjectNotExistException::ice_throw() const
{
    throw *this;
}

Ice::FacetNotExistException::FacetNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::Ice::RequestFailedException(__file, __line)
#endif
{
}

Ice::FacetNotExistException::FacetNotExistException(const char* __file, int __line, const ::Ice::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::Ice::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__FacetNotExistException_name = "Ice::FacetNotExistException";

const ::std::string
Ice::FacetNotExistException::ice_name() const
{
    return __Ice__FacetNotExistException_name;
}

::Ice::Exception*
Ice::FacetNotExistException::ice_clone() const
{
    return new FacetNotExistException(*this);
}

void
Ice::FacetNotExistException::ice_throw() const
{
    throw *this;
}

Ice::OperationNotExistException::OperationNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::Ice::RequestFailedException(__file, __line)
#endif
{
}

Ice::OperationNotExistException::OperationNotExistException(const char* __file, int __line, const ::Ice::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::Ice::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__OperationNotExistException_name = "Ice::OperationNotExistException";

const ::std::string
Ice::OperationNotExistException::ice_name() const
{
    return __Ice__OperationNotExistException_name;
}

::Ice::Exception*
Ice::OperationNotExistException::ice_clone() const
{
    return new OperationNotExistException(*this);
}

void
Ice::OperationNotExistException::ice_throw() const
{
    throw *this;
}

Ice::SyscallException::SyscallException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::SyscallException::SyscallException(const char* __file, int __line, ::Ice::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    error(__error)
{
}

static const char* __Ice__SyscallException_name = "Ice::SyscallException";

const ::std::string
Ice::SyscallException::ice_name() const
{
    return __Ice__SyscallException_name;
}

::Ice::Exception*
Ice::SyscallException::ice_clone() const
{
    return new SyscallException(*this);
}

void
Ice::SyscallException::ice_throw() const
{
    throw *this;
}

Ice::SocketException::SocketException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line)
#else
    ::Ice::SyscallException(__file, __line)
#endif
{
}

Ice::SocketException::SocketException(const char* __file, int __line, ::Ice::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line, __error)
#else
    ::Ice::SyscallException(__file, __line, __error)
#endif
{
}

static const char* __Ice__SocketException_name = "Ice::SocketException";

const ::std::string
Ice::SocketException::ice_name() const
{
    return __Ice__SocketException_name;
}

::Ice::Exception*
Ice::SocketException::ice_clone() const
{
    return new SocketException(*this);
}

void
Ice::SocketException::ice_throw() const
{
    throw *this;
}

Ice::FileException::FileException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line)
#else
    ::Ice::SyscallException(__file, __line)
#endif
{
}

Ice::FileException::FileException(const char* __file, int __line, ::Ice::Int __error, const ::std::string& __path) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line, __error),
#else
    ::Ice::SyscallException(__file, __line, __error),
#endif
    path(__path)
{
}

static const char* __Ice__FileException_name = "Ice::FileException";

const ::std::string
Ice::FileException::ice_name() const
{
    return __Ice__FileException_name;
}

::Ice::Exception*
Ice::FileException::ice_clone() const
{
    return new FileException(*this);
}

void
Ice::FileException::ice_throw() const
{
    throw *this;
}

Ice::ConnectFailedException::ConnectFailedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line)
#else
    ::Ice::SocketException(__file, __line)
#endif
{
}

Ice::ConnectFailedException::ConnectFailedException(const char* __file, int __line, ::Ice::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line, __error)
#else
    ::Ice::SocketException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectFailedException_name = "Ice::ConnectFailedException";

const ::std::string
Ice::ConnectFailedException::ice_name() const
{
    return __Ice__ConnectFailedException_name;
}

::Ice::Exception*
Ice::ConnectFailedException::ice_clone() const
{
    return new ConnectFailedException(*this);
}

void
Ice::ConnectFailedException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionRefusedException::ConnectionRefusedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ConnectFailedException(__file, __line)
#else
    ::Ice::ConnectFailedException(__file, __line)
#endif
{
}

Ice::ConnectionRefusedException::ConnectionRefusedException(const char* __file, int __line, ::Ice::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ConnectFailedException(__file, __line, __error)
#else
    ::Ice::ConnectFailedException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectionRefusedException_name = "Ice::ConnectionRefusedException";

const ::std::string
Ice::ConnectionRefusedException::ice_name() const
{
    return __Ice__ConnectionRefusedException_name;
}

::Ice::Exception*
Ice::ConnectionRefusedException::ice_clone() const
{
    return new ConnectionRefusedException(*this);
}

void
Ice::ConnectionRefusedException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionLostException::ConnectionLostException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line)
#else
    ::Ice::SocketException(__file, __line)
#endif
{
}

Ice::ConnectionLostException::ConnectionLostException(const char* __file, int __line, ::Ice::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line, __error)
#else
    ::Ice::SocketException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectionLostException_name = "Ice::ConnectionLostException";

const ::std::string
Ice::ConnectionLostException::ice_name() const
{
    return __Ice__ConnectionLostException_name;
}

::Ice::Exception*
Ice::ConnectionLostException::ice_clone() const
{
    return new ConnectionLostException(*this);
}

void
Ice::ConnectionLostException::ice_throw() const
{
    throw *this;
}

Ice::DNSException::DNSException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::DNSException::DNSException(const char* __file, int __line, ::Ice::Int __error, const ::std::string& __host) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    error(__error),
    host(__host)
{
}

static const char* __Ice__DNSException_name = "Ice::DNSException";

const ::std::string
Ice::DNSException::ice_name() const
{
    return __Ice__DNSException_name;
}

::Ice::Exception*
Ice::DNSException::ice_clone() const
{
    return new DNSException(*this);
}

void
Ice::DNSException::ice_throw() const
{
    throw *this;
}

Ice::TimeoutException::TimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__TimeoutException_name = "Ice::TimeoutException";

const ::std::string
Ice::TimeoutException::ice_name() const
{
    return __Ice__TimeoutException_name;
}

::Ice::Exception*
Ice::TimeoutException::ice_clone() const
{
    return new TimeoutException(*this);
}

void
Ice::TimeoutException::ice_throw() const
{
    throw *this;
}

Ice::ConnectTimeoutException::ConnectTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::Ice::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectTimeoutException_name = "Ice::ConnectTimeoutException";

const ::std::string
Ice::ConnectTimeoutException::ice_name() const
{
    return __Ice__ConnectTimeoutException_name;
}

::Ice::Exception*
Ice::ConnectTimeoutException::ice_clone() const
{
    return new ConnectTimeoutException(*this);
}

void
Ice::ConnectTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::CloseTimeoutException::CloseTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::Ice::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__CloseTimeoutException_name = "Ice::CloseTimeoutException";

const ::std::string
Ice::CloseTimeoutException::ice_name() const
{
    return __Ice__CloseTimeoutException_name;
}

::Ice::Exception*
Ice::CloseTimeoutException::ice_clone() const
{
    return new CloseTimeoutException(*this);
}

void
Ice::CloseTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionTimeoutException::ConnectionTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::Ice::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectionTimeoutException_name = "Ice::ConnectionTimeoutException";

const ::std::string
Ice::ConnectionTimeoutException::ice_name() const
{
    return __Ice__ConnectionTimeoutException_name;
}

::Ice::Exception*
Ice::ConnectionTimeoutException::ice_clone() const
{
    return new ConnectionTimeoutException(*this);
}

void
Ice::ConnectionTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::ProtocolException::ProtocolException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__ProtocolException_name = "Ice::ProtocolException";

const ::std::string
Ice::ProtocolException::ice_name() const
{
    return __Ice__ProtocolException_name;
}

::Ice::Exception*
Ice::ProtocolException::ice_clone() const
{
    return new ProtocolException(*this);
}

void
Ice::ProtocolException::ice_throw() const
{
    throw *this;
}

Ice::BadMagicException::BadMagicException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

Ice::BadMagicException::BadMagicException(const char* __file, int __line, const ::Ice::ByteSeq& __badMagic) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::Ice::ProtocolException(__file, __line),
#endif
    badMagic(__badMagic)
{
}

static const char* __Ice__BadMagicException_name = "Ice::BadMagicException";

const ::std::string
Ice::BadMagicException::ice_name() const
{
    return __Ice__BadMagicException_name;
}

::Ice::Exception*
Ice::BadMagicException::ice_clone() const
{
    return new BadMagicException(*this);
}

void
Ice::BadMagicException::ice_throw() const
{
    throw *this;
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const char* __file, int __line, ::Ice::Int __badMajor, ::Ice::Int __badMinor, ::Ice::Int __major, ::Ice::Int __minor) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::Ice::ProtocolException(__file, __line),
#endif
    badMajor(__badMajor),
    badMinor(__badMinor),
    major(__major),
    minor(__minor)
{
}

static const char* __Ice__UnsupportedProtocolException_name = "Ice::UnsupportedProtocolException";

const ::std::string
Ice::UnsupportedProtocolException::ice_name() const
{
    return __Ice__UnsupportedProtocolException_name;
}

::Ice::Exception*
Ice::UnsupportedProtocolException::ice_clone() const
{
    return new UnsupportedProtocolException(*this);
}

void
Ice::UnsupportedProtocolException::ice_throw() const
{
    throw *this;
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const char* __file, int __line, ::Ice::Int __badMajor, ::Ice::Int __badMinor, ::Ice::Int __major, ::Ice::Int __minor) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::Ice::ProtocolException(__file, __line),
#endif
    badMajor(__badMajor),
    badMinor(__badMinor),
    major(__major),
    minor(__minor)
{
}

static const char* __Ice__UnsupportedEncodingException_name = "Ice::UnsupportedEncodingException";

const ::std::string
Ice::UnsupportedEncodingException::ice_name() const
{
    return __Ice__UnsupportedEncodingException_name;
}

::Ice::Exception*
Ice::UnsupportedEncodingException::ice_clone() const
{
    return new UnsupportedEncodingException(*this);
}

void
Ice::UnsupportedEncodingException::ice_throw() const
{
    throw *this;
}

Ice::UnknownMessageException::UnknownMessageException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownMessageException_name = "Ice::UnknownMessageException";

const ::std::string
Ice::UnknownMessageException::ice_name() const
{
    return __Ice__UnknownMessageException_name;
}

::Ice::Exception*
Ice::UnknownMessageException::ice_clone() const
{
    return new UnknownMessageException(*this);
}

void
Ice::UnknownMessageException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionNotValidatedException::ConnectionNotValidatedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectionNotValidatedException_name = "Ice::ConnectionNotValidatedException";

const ::std::string
Ice::ConnectionNotValidatedException::ice_name() const
{
    return __Ice__ConnectionNotValidatedException_name;
}

::Ice::Exception*
Ice::ConnectionNotValidatedException::ice_clone() const
{
    return new ConnectionNotValidatedException(*this);
}

void
Ice::ConnectionNotValidatedException::ice_throw() const
{
    throw *this;
}

Ice::UnknownRequestIdException::UnknownRequestIdException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownRequestIdException_name = "Ice::UnknownRequestIdException";

const ::std::string
Ice::UnknownRequestIdException::ice_name() const
{
    return __Ice__UnknownRequestIdException_name;
}

::Ice::Exception*
Ice::UnknownRequestIdException::ice_clone() const
{
    return new UnknownRequestIdException(*this);
}

void
Ice::UnknownRequestIdException::ice_throw() const
{
    throw *this;
}

Ice::UnknownReplyStatusException::UnknownReplyStatusException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownReplyStatusException_name = "Ice::UnknownReplyStatusException";

const ::std::string
Ice::UnknownReplyStatusException::ice_name() const
{
    return __Ice__UnknownReplyStatusException_name;
}

::Ice::Exception*
Ice::UnknownReplyStatusException::ice_clone() const
{
    return new UnknownReplyStatusException(*this);
}

void
Ice::UnknownReplyStatusException::ice_throw() const
{
    throw *this;
}

Ice::CloseConnectionException::CloseConnectionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__CloseConnectionException_name = "Ice::CloseConnectionException";

const ::std::string
Ice::CloseConnectionException::ice_name() const
{
    return __Ice__CloseConnectionException_name;
}

::Ice::Exception*
Ice::CloseConnectionException::ice_clone() const
{
    return new CloseConnectionException(*this);
}

void
Ice::CloseConnectionException::ice_throw() const
{
    throw *this;
}

Ice::ForcedCloseConnectionException::ForcedCloseConnectionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__ForcedCloseConnectionException_name = "Ice::ForcedCloseConnectionException";

const ::std::string
Ice::ForcedCloseConnectionException::ice_name() const
{
    return __Ice__ForcedCloseConnectionException_name;
}

::Ice::Exception*
Ice::ForcedCloseConnectionException::ice_clone() const
{
    return new ForcedCloseConnectionException(*this);
}

void
Ice::ForcedCloseConnectionException::ice_throw() const
{
    throw *this;
}

Ice::IllegalMessageSizeException::IllegalMessageSizeException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__IllegalMessageSizeException_name = "Ice::IllegalMessageSizeException";

const ::std::string
Ice::IllegalMessageSizeException::ice_name() const
{
    return __Ice__IllegalMessageSizeException_name;
}

::Ice::Exception*
Ice::IllegalMessageSizeException::ice_clone() const
{
    return new IllegalMessageSizeException(*this);
}

void
Ice::IllegalMessageSizeException::ice_throw() const
{
    throw *this;
}

Ice::CompressionNotSupportedException::CompressionNotSupportedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__CompressionNotSupportedException_name = "Ice::CompressionNotSupportedException";

const ::std::string
Ice::CompressionNotSupportedException::ice_name() const
{
    return __Ice__CompressionNotSupportedException_name;
}

::Ice::Exception*
Ice::CompressionNotSupportedException::ice_clone() const
{
    return new CompressionNotSupportedException(*this);
}

void
Ice::CompressionNotSupportedException::ice_throw() const
{
    throw *this;
}

Ice::MarshalException::MarshalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

Ice::MarshalException::MarshalException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::Ice::ProtocolException(__file, __line),
#endif
    reason(__reason)
{
}

static const char* __Ice__MarshalException_name = "Ice::MarshalException";

const ::std::string
Ice::MarshalException::ice_name() const
{
    return __Ice__MarshalException_name;
}

::Ice::Exception*
Ice::MarshalException::ice_clone() const
{
    return new MarshalException(*this);
}

void
Ice::MarshalException::ice_throw() const
{
    throw *this;
}

Ice::ProxyUnmarshalException::ProxyUnmarshalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::ProxyUnmarshalException::ProxyUnmarshalException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__ProxyUnmarshalException_name = "Ice::ProxyUnmarshalException";

const ::std::string
Ice::ProxyUnmarshalException::ice_name() const
{
    return __Ice__ProxyUnmarshalException_name;
}

::Ice::Exception*
Ice::ProxyUnmarshalException::ice_clone() const
{
    return new ProxyUnmarshalException(*this);
}

void
Ice::ProxyUnmarshalException::ice_throw() const
{
    throw *this;
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__UnmarshalOutOfBoundsException_name = "Ice::UnmarshalOutOfBoundsException";

const ::std::string
Ice::UnmarshalOutOfBoundsException::ice_name() const
{
    return __Ice__UnmarshalOutOfBoundsException_name;
}

::Ice::Exception*
Ice::UnmarshalOutOfBoundsException::ice_clone() const
{
    return new UnmarshalOutOfBoundsException(*this);
}

void
Ice::UnmarshalOutOfBoundsException::ice_throw() const
{
    throw *this;
}

Ice::MemoryLimitException::MemoryLimitException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::MemoryLimitException::MemoryLimitException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__MemoryLimitException_name = "Ice::MemoryLimitException";

const ::std::string
Ice::MemoryLimitException::ice_name() const
{
    return __Ice__MemoryLimitException_name;
}

::Ice::Exception*
Ice::MemoryLimitException::ice_clone() const
{
    return new MemoryLimitException(*this);
}

void
Ice::MemoryLimitException::ice_throw() const
{
    throw *this;
}

Ice::EncapsulationException::EncapsulationException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::EncapsulationException::EncapsulationException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__EncapsulationException_name = "Ice::EncapsulationException";

const ::std::string
Ice::EncapsulationException::ice_name() const
{
    return __Ice__EncapsulationException_name;
}

::Ice::Exception*
Ice::EncapsulationException::ice_clone() const
{
    return new EncapsulationException(*this);
}

void
Ice::EncapsulationException::ice_throw() const
{
    throw *this;
}

Ice::NegativeSizeException::NegativeSizeException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::NegativeSizeException::NegativeSizeException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__NegativeSizeException_name = "Ice::NegativeSizeException";

const ::std::string
Ice::NegativeSizeException::ice_name() const
{
    return __Ice__NegativeSizeException_name;
}

::Ice::Exception*
Ice::NegativeSizeException::ice_clone() const
{
    return new NegativeSizeException(*this);
}

void
Ice::NegativeSizeException::ice_throw() const
{
    throw *this;
}
