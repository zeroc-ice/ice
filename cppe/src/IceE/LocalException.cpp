// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalException.h>
#include <IceE/SafeStdio.h>
#include <IceE/IdentityUtil.h>
#include <IceE/Network.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

IceE::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

IceE::AlreadyRegisteredException::AlreadyRegisteredException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::AlreadyRegisteredException::AlreadyRegisteredException(const char* __file, int __line, const ::std::string& __kindOfObject, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    kindOfObject(__kindOfObject),
    id(__id)
{
}

static const char* __Ice__AlreadyRegisteredException_name = "IceE::AlreadyRegisteredException";

const ::std::string
IceE::AlreadyRegisteredException::ice_name() const
{
    return __Ice__AlreadyRegisteredException_name;
}

::IceE::Exception*
IceE::AlreadyRegisteredException::ice_clone() const
{
    return new AlreadyRegisteredException(*this);
}

void
IceE::AlreadyRegisteredException::ice_throw() const
{
    throw *this;
}

IceE::NotRegisteredException::NotRegisteredException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::NotRegisteredException::NotRegisteredException(const char* __file, int __line, const ::std::string& __kindOfObject, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    kindOfObject(__kindOfObject),
    id(__id)
{
}

static const char* __Ice__NotRegisteredException_name = "IceE::NotRegisteredException";

const ::std::string
IceE::NotRegisteredException::ice_name() const
{
    return __Ice__NotRegisteredException_name;
}

::IceE::Exception*
IceE::NotRegisteredException::ice_clone() const
{
    return new NotRegisteredException(*this);
}

void
IceE::NotRegisteredException::ice_throw() const
{
    throw *this;
}

IceE::TwowayOnlyException::TwowayOnlyException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::TwowayOnlyException::TwowayOnlyException(const char* __file, int __line, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    operation(__operation)
{
}

static const char* __Ice__TwowayOnlyException_name = "IceE::TwowayOnlyException";

const ::std::string
IceE::TwowayOnlyException::ice_name() const
{
    return __Ice__TwowayOnlyException_name;
}

::IceE::Exception*
IceE::TwowayOnlyException::ice_clone() const
{
    return new TwowayOnlyException(*this);
}

void
IceE::TwowayOnlyException::ice_throw() const
{
    throw *this;
}

IceE::CloneNotImplementedException::CloneNotImplementedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__CloneNotImplementedException_name = "IceE::CloneNotImplementedException";

const ::std::string
IceE::CloneNotImplementedException::ice_name() const
{
    return __Ice__CloneNotImplementedException_name;
}

::IceE::Exception*
IceE::CloneNotImplementedException::ice_clone() const
{
    return new CloneNotImplementedException(*this);
}

void
IceE::CloneNotImplementedException::ice_throw() const
{
    throw *this;
}

IceE::UnknownException::UnknownException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::UnknownException::UnknownException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    unknown(__unknown)
{
}

static const char* __Ice__UnknownException_name = "IceE::UnknownException";

const ::std::string
IceE::UnknownException::ice_name() const
{
    return __Ice__UnknownException_name;
}

::IceE::Exception*
IceE::UnknownException::ice_clone() const
{
    return new UnknownException(*this);
}

void
IceE::UnknownException::ice_throw() const
{
    throw *this;
}

IceE::UnknownLocalException::UnknownLocalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line)
#else
    ::IceE::UnknownException(__file, __line)
#endif
{
}

IceE::UnknownLocalException::UnknownLocalException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line, __unknown)
#else
    ::IceE::UnknownException(__file, __line, __unknown)
#endif
{
}

static const char* __Ice__UnknownLocalException_name = "IceE::UnknownLocalException";

const ::std::string
IceE::UnknownLocalException::ice_name() const
{
    return __Ice__UnknownLocalException_name;
}

::IceE::Exception*
IceE::UnknownLocalException::ice_clone() const
{
    return new UnknownLocalException(*this);
}

void
IceE::UnknownLocalException::ice_throw() const
{
    throw *this;
}

IceE::UnknownUserException::UnknownUserException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line)
#else
    ::IceE::UnknownException(__file, __line)
#endif
{
}

IceE::UnknownUserException::UnknownUserException(const char* __file, int __line, const ::std::string& __unknown) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UnknownException(__file, __line, __unknown)
#else
    ::IceE::UnknownException(__file, __line, __unknown)
#endif
{
}

static const char* __Ice__UnknownUserException_name = "IceE::UnknownUserException";

const ::std::string
IceE::UnknownUserException::ice_name() const
{
    return __Ice__UnknownUserException_name;
}

::IceE::Exception*
IceE::UnknownUserException::ice_clone() const
{
    return new UnknownUserException(*this);
}

void
IceE::UnknownUserException::ice_throw() const
{
    throw *this;
}

IceE::VersionMismatchException::VersionMismatchException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__VersionMismatchException_name = "IceE::VersionMismatchException";

const ::std::string
IceE::VersionMismatchException::ice_name() const
{
    return __Ice__VersionMismatchException_name;
}

::IceE::Exception*
IceE::VersionMismatchException::ice_clone() const
{
    return new VersionMismatchException(*this);
}

void
IceE::VersionMismatchException::ice_throw() const
{
    throw *this;
}

IceE::CommunicatorDestroyedException::CommunicatorDestroyedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__CommunicatorDestroyedException_name = "IceE::CommunicatorDestroyedException";

const ::std::string
IceE::CommunicatorDestroyedException::ice_name() const
{
    return __Ice__CommunicatorDestroyedException_name;
}

::IceE::Exception*
IceE::CommunicatorDestroyedException::ice_clone() const
{
    return new CommunicatorDestroyedException(*this);
}

void
IceE::CommunicatorDestroyedException::ice_throw() const
{
    throw *this;
}

#ifndef ICEE_PURE_CLIENT

IceE::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* __file, int __line, const ::std::string& __name) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    name(__name)
{
}

static const char* __Ice__ObjectAdapterDeactivatedException_name = "IceE::ObjectAdapterDeactivatedException";

const ::std::string
IceE::ObjectAdapterDeactivatedException::ice_name() const
{
    return __Ice__ObjectAdapterDeactivatedException_name;
}

::IceE::Exception*
IceE::ObjectAdapterDeactivatedException::ice_clone() const
{
    return new ObjectAdapterDeactivatedException(*this);
}

void
IceE::ObjectAdapterDeactivatedException::ice_throw() const
{
    throw *this;
}
string
IceE::ObjectAdapterDeactivatedException::toString() const
{
    string out = Exception::toString();
    out +=  ":\nobject adapter `";
    out += name;
    out += "' deactivated";
    return out;
}

IceE::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* __file, int __line, const ::std::string& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    id(__id)
{
}

static const char* __Ice__ObjectAdapterIdInUseException_name = "IceE::ObjectAdapterIdInUseException";

const ::std::string
IceE::ObjectAdapterIdInUseException::ice_name() const
{
    return __Ice__ObjectAdapterIdInUseException_name;
}

::IceE::Exception*
IceE::ObjectAdapterIdInUseException::ice_clone() const
{
    return new ObjectAdapterIdInUseException(*this);
}

void
IceE::ObjectAdapterIdInUseException::ice_throw() const
{
    throw *this;
}

string
IceE::ObjectAdapterIdInUseException::toString() const
{
#ifndef ICEE_NO_LOCATOR
    string out = Exception::toString();
    out += ":\nobject adapter with id `";
    out += id; 
    out += "' is already in use";
    return out;
#else
    return string("");
#endif
}

#endif // ICEE_PURE_CLIENT

IceE::NoEndpointException::NoEndpointException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::NoEndpointException::NoEndpointException(const char* __file, int __line, const ::std::string& __proxy) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    proxy(__proxy)
{
}

static const char* __Ice__NoEndpointException_name = "IceE::NoEndpointException";

const ::std::string
IceE::NoEndpointException::ice_name() const
{
    return __Ice__NoEndpointException_name;
}

::IceE::Exception*
IceE::NoEndpointException::ice_clone() const
{
    return new NoEndpointException(*this);
}

void
IceE::NoEndpointException::ice_throw() const
{
    throw *this;
}

IceE::EndpointParseException::EndpointParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::EndpointParseException::EndpointParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__EndpointParseException_name = "IceE::EndpointParseException";

const ::std::string
IceE::EndpointParseException::ice_name() const
{
    return __Ice__EndpointParseException_name;
}

::IceE::Exception*
IceE::EndpointParseException::ice_clone() const
{
    return new EndpointParseException(*this);
}

void
IceE::EndpointParseException::ice_throw() const
{
    throw *this;
}

IceE::IdentityParseException::IdentityParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::IdentityParseException::IdentityParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__IdentityParseException_name = "IceE::IdentityParseException";

const ::std::string
IceE::IdentityParseException::ice_name() const
{
    return __Ice__IdentityParseException_name;
}

::IceE::Exception*
IceE::IdentityParseException::ice_clone() const
{
    return new IdentityParseException(*this);
}

void
IceE::IdentityParseException::ice_throw() const
{
    throw *this;
}

IceE::ProxyParseException::ProxyParseException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::ProxyParseException::ProxyParseException(const char* __file, int __line, const ::std::string& __str) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    str(__str)
{
}

static const char* __Ice__ProxyParseException_name = "IceE::ProxyParseException";

const ::std::string
IceE::ProxyParseException::ice_name() const
{
    return __Ice__ProxyParseException_name;
}

::IceE::Exception*
IceE::ProxyParseException::ice_clone() const
{
    return new ProxyParseException(*this);
}

void
IceE::ProxyParseException::ice_throw() const
{
    throw *this;
}

IceE::IllegalIdentityException::IllegalIdentityException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::IllegalIdentityException::IllegalIdentityException(const char* __file, int __line, const ::IceE::Identity& __id) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    id(__id)
{
}

static const char* __Ice__IllegalIdentityException_name = "IceE::IllegalIdentityException";

const ::std::string
IceE::IllegalIdentityException::ice_name() const
{
    return __Ice__IllegalIdentityException_name;
}

::IceE::Exception*
IceE::IllegalIdentityException::ice_clone() const
{
    return new IllegalIdentityException(*this);
}

void
IceE::IllegalIdentityException::ice_throw() const
{
    throw *this;
}

IceE::RequestFailedException::RequestFailedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::RequestFailedException::RequestFailedException(const char* __file, int __line, const ::IceE::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    id(__id),
    facet(__facet),
    operation(__operation)
{
}

static const char* __Ice__RequestFailedException_name = "IceE::RequestFailedException";

const ::std::string
IceE::RequestFailedException::ice_name() const
{
    return __Ice__RequestFailedException_name;
}

::IceE::Exception*
IceE::RequestFailedException::ice_clone() const
{
    return new RequestFailedException(*this);
}

void
IceE::RequestFailedException::ice_throw() const
{
    throw *this;
}

IceE::ObjectNotExistException::ObjectNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::IceE::RequestFailedException(__file, __line)
#endif
{
}

IceE::ObjectNotExistException::ObjectNotExistException(const char* __file, int __line, const ::IceE::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::IceE::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__ObjectNotExistException_name = "IceE::ObjectNotExistException";

const ::std::string
IceE::ObjectNotExistException::ice_name() const
{
    return __Ice__ObjectNotExistException_name;
}

::IceE::Exception*
IceE::ObjectNotExistException::ice_clone() const
{
    return new ObjectNotExistException(*this);
}

void
IceE::ObjectNotExistException::ice_throw() const
{
    throw *this;
}

IceE::FacetNotExistException::FacetNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::IceE::RequestFailedException(__file, __line)
#endif
{
}

IceE::FacetNotExistException::FacetNotExistException(const char* __file, int __line, const ::IceE::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::IceE::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__FacetNotExistException_name = "IceE::FacetNotExistException";

const ::std::string
IceE::FacetNotExistException::ice_name() const
{
    return __Ice__FacetNotExistException_name;
}

::IceE::Exception*
IceE::FacetNotExistException::ice_clone() const
{
    return new FacetNotExistException(*this);
}

void
IceE::FacetNotExistException::ice_throw() const
{
    throw *this;
}

IceE::OperationNotExistException::OperationNotExistException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line)
#else
    ::IceE::RequestFailedException(__file, __line)
#endif
{
}

IceE::OperationNotExistException::OperationNotExistException(const char* __file, int __line, const ::IceE::Identity& __id, const ::std::string& __facet, const ::std::string& __operation) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    RequestFailedException(__file, __line, __id, __facet, __operation)
#else
    ::IceE::RequestFailedException(__file, __line, __id, __facet, __operation)
#endif
{
}

static const char* __Ice__OperationNotExistException_name = "IceE::OperationNotExistException";

const ::std::string
IceE::OperationNotExistException::ice_name() const
{
    return __Ice__OperationNotExistException_name;
}

::IceE::Exception*
IceE::OperationNotExistException::ice_clone() const
{
    return new OperationNotExistException(*this);
}

void
IceE::OperationNotExistException::ice_throw() const
{
    throw *this;
}

IceE::SyscallException::SyscallException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::SyscallException::SyscallException(const char* __file, int __line, ::IceE::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    error(__error)
{
}

static const char* __Ice__SyscallException_name = "IceE::SyscallException";

const ::std::string
IceE::SyscallException::ice_name() const
{
    return __Ice__SyscallException_name;
}

::IceE::Exception*
IceE::SyscallException::ice_clone() const
{
    return new SyscallException(*this);
}

void
IceE::SyscallException::ice_throw() const
{
    throw *this;
}

IceE::SocketException::SocketException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line)
#else
    ::IceE::SyscallException(__file, __line)
#endif
{
}

IceE::SocketException::SocketException(const char* __file, int __line, ::IceE::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line, __error)
#else
    ::IceE::SyscallException(__file, __line, __error)
#endif
{
}

static const char* __Ice__SocketException_name = "IceE::SocketException";

const ::std::string
IceE::SocketException::ice_name() const
{
    return __Ice__SocketException_name;
}

::IceE::Exception*
IceE::SocketException::ice_clone() const
{
    return new SocketException(*this);
}

void
IceE::SocketException::ice_throw() const
{
    throw *this;
}

IceE::FileException::FileException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line)
#else
    ::IceE::SyscallException(__file, __line)
#endif
{
}

IceE::FileException::FileException(const char* __file, int __line, ::IceE::Int __error, const ::std::string& __path) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SyscallException(__file, __line, __error),
#else
    ::IceE::SyscallException(__file, __line, __error),
#endif
    path(__path)
{
}

static const char* __Ice__FileException_name = "IceE::FileException";

const ::std::string
IceE::FileException::ice_name() const
{
    return __Ice__FileException_name;
}

::IceE::Exception*
IceE::FileException::ice_clone() const
{
    return new FileException(*this);
}

void
IceE::FileException::ice_throw() const
{
    throw *this;
}

IceE::ConnectFailedException::ConnectFailedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line)
#else
    ::IceE::SocketException(__file, __line)
#endif
{
}

IceE::ConnectFailedException::ConnectFailedException(const char* __file, int __line, ::IceE::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line, __error)
#else
    ::IceE::SocketException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectFailedException_name = "IceE::ConnectFailedException";

const ::std::string
IceE::ConnectFailedException::ice_name() const
{
    return __Ice__ConnectFailedException_name;
}

::IceE::Exception*
IceE::ConnectFailedException::ice_clone() const
{
    return new ConnectFailedException(*this);
}

void
IceE::ConnectFailedException::ice_throw() const
{
    throw *this;
}

IceE::ConnectionRefusedException::ConnectionRefusedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ConnectFailedException(__file, __line)
#else
    ::IceE::ConnectFailedException(__file, __line)
#endif
{
}

IceE::ConnectionRefusedException::ConnectionRefusedException(const char* __file, int __line, ::IceE::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ConnectFailedException(__file, __line, __error)
#else
    ::IceE::ConnectFailedException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectionRefusedException_name = "IceE::ConnectionRefusedException";

const ::std::string
IceE::ConnectionRefusedException::ice_name() const
{
    return __Ice__ConnectionRefusedException_name;
}

::IceE::Exception*
IceE::ConnectionRefusedException::ice_clone() const
{
    return new ConnectionRefusedException(*this);
}

void
IceE::ConnectionRefusedException::ice_throw() const
{
    throw *this;
}

IceE::ConnectionLostException::ConnectionLostException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line)
#else
    ::IceE::SocketException(__file, __line)
#endif
{
}

IceE::ConnectionLostException::ConnectionLostException(const char* __file, int __line, ::IceE::Int __error) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    SocketException(__file, __line, __error)
#else
    ::IceE::SocketException(__file, __line, __error)
#endif
{
}

static const char* __Ice__ConnectionLostException_name = "IceE::ConnectionLostException";

const ::std::string
IceE::ConnectionLostException::ice_name() const
{
    return __Ice__ConnectionLostException_name;
}

::IceE::Exception*
IceE::ConnectionLostException::ice_clone() const
{
    return new ConnectionLostException(*this);
}

void
IceE::ConnectionLostException::ice_throw() const
{
    throw *this;
}

IceE::DNSException::DNSException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

IceE::DNSException::DNSException(const char* __file, int __line, ::IceE::Int __error, const ::std::string& __host) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::IceE::LocalException(__file, __line),
#endif
    error(__error),
    host(__host)
{
}

static const char* __Ice__DNSException_name = "IceE::DNSException";

const ::std::string
IceE::DNSException::ice_name() const
{
    return __Ice__DNSException_name;
}

::IceE::Exception*
IceE::DNSException::ice_clone() const
{
    return new DNSException(*this);
}

void
IceE::DNSException::ice_throw() const
{
    throw *this;
}

IceE::TimeoutException::TimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__TimeoutException_name = "IceE::TimeoutException";

const ::std::string
IceE::TimeoutException::ice_name() const
{
    return __Ice__TimeoutException_name;
}

::IceE::Exception*
IceE::TimeoutException::ice_clone() const
{
    return new TimeoutException(*this);
}

void
IceE::TimeoutException::ice_throw() const
{
    throw *this;
}

IceE::ConnectTimeoutException::ConnectTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::IceE::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectTimeoutException_name = "IceE::ConnectTimeoutException";

const ::std::string
IceE::ConnectTimeoutException::ice_name() const
{
    return __Ice__ConnectTimeoutException_name;
}

::IceE::Exception*
IceE::ConnectTimeoutException::ice_clone() const
{
    return new ConnectTimeoutException(*this);
}

void
IceE::ConnectTimeoutException::ice_throw() const
{
    throw *this;
}

IceE::CloseTimeoutException::CloseTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::IceE::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__CloseTimeoutException_name = "IceE::CloseTimeoutException";

const ::std::string
IceE::CloseTimeoutException::ice_name() const
{
    return __Ice__CloseTimeoutException_name;
}

::IceE::Exception*
IceE::CloseTimeoutException::ice_clone() const
{
    return new CloseTimeoutException(*this);
}

void
IceE::CloseTimeoutException::ice_throw() const
{
    throw *this;
}

IceE::ConnectionTimeoutException::ConnectionTimeoutException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    TimeoutException(__file, __line)
#else
    ::IceE::TimeoutException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectionTimeoutException_name = "IceE::ConnectionTimeoutException";

const ::std::string
IceE::ConnectionTimeoutException::ice_name() const
{
    return __Ice__ConnectionTimeoutException_name;
}

::IceE::Exception*
IceE::ConnectionTimeoutException::ice_clone() const
{
    return new ConnectionTimeoutException(*this);
}

void
IceE::ConnectionTimeoutException::ice_throw() const
{
    throw *this;
}

IceE::ProtocolException::ProtocolException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::IceE::LocalException(__file, __line)
#endif
{
}

static const char* __Ice__ProtocolException_name = "IceE::ProtocolException";

const ::std::string
IceE::ProtocolException::ice_name() const
{
    return __Ice__ProtocolException_name;
}

::IceE::Exception*
IceE::ProtocolException::ice_clone() const
{
    return new ProtocolException(*this);
}

void
IceE::ProtocolException::ice_throw() const
{
    throw *this;
}

IceE::BadMagicException::BadMagicException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

IceE::BadMagicException::BadMagicException(const char* __file, int __line, const ::IceE::ByteSeq& __badMagic) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::IceE::ProtocolException(__file, __line),
#endif
    badMagic(__badMagic)
{
}

static const char* __Ice__BadMagicException_name = "IceE::BadMagicException";

const ::std::string
IceE::BadMagicException::ice_name() const
{
    return __Ice__BadMagicException_name;
}

::IceE::Exception*
IceE::BadMagicException::ice_clone() const
{
    return new BadMagicException(*this);
}

void
IceE::BadMagicException::ice_throw() const
{
    throw *this;
}

IceE::UnsupportedProtocolException::UnsupportedProtocolException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

IceE::UnsupportedProtocolException::UnsupportedProtocolException(const char* __file, int __line, ::IceE::Int __badMajor, ::IceE::Int __badMinor, ::IceE::Int __major, ::IceE::Int __minor) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::IceE::ProtocolException(__file, __line),
#endif
    badMajor(__badMajor),
    badMinor(__badMinor),
    major(__major),
    minor(__minor)
{
}

static const char* __Ice__UnsupportedProtocolException_name = "IceE::UnsupportedProtocolException";

const ::std::string
IceE::UnsupportedProtocolException::ice_name() const
{
    return __Ice__UnsupportedProtocolException_name;
}

::IceE::Exception*
IceE::UnsupportedProtocolException::ice_clone() const
{
    return new UnsupportedProtocolException(*this);
}

void
IceE::UnsupportedProtocolException::ice_throw() const
{
    throw *this;
}

IceE::UnsupportedEncodingException::UnsupportedEncodingException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

IceE::UnsupportedEncodingException::UnsupportedEncodingException(const char* __file, int __line, ::IceE::Int __badMajor, ::IceE::Int __badMinor, ::IceE::Int __major, ::IceE::Int __minor) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::IceE::ProtocolException(__file, __line),
#endif
    badMajor(__badMajor),
    badMinor(__badMinor),
    major(__major),
    minor(__minor)
{
}

static const char* __Ice__UnsupportedEncodingException_name = "IceE::UnsupportedEncodingException";

const ::std::string
IceE::UnsupportedEncodingException::ice_name() const
{
    return __Ice__UnsupportedEncodingException_name;
}

::IceE::Exception*
IceE::UnsupportedEncodingException::ice_clone() const
{
    return new UnsupportedEncodingException(*this);
}

void
IceE::UnsupportedEncodingException::ice_throw() const
{
    throw *this;
}

IceE::UnknownMessageException::UnknownMessageException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownMessageException_name = "IceE::UnknownMessageException";

const ::std::string
IceE::UnknownMessageException::ice_name() const
{
    return __Ice__UnknownMessageException_name;
}

::IceE::Exception*
IceE::UnknownMessageException::ice_clone() const
{
    return new UnknownMessageException(*this);
}

void
IceE::UnknownMessageException::ice_throw() const
{
    throw *this;
}

IceE::ConnectionNotValidatedException::ConnectionNotValidatedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__ConnectionNotValidatedException_name = "IceE::ConnectionNotValidatedException";

const ::std::string
IceE::ConnectionNotValidatedException::ice_name() const
{
    return __Ice__ConnectionNotValidatedException_name;
}

::IceE::Exception*
IceE::ConnectionNotValidatedException::ice_clone() const
{
    return new ConnectionNotValidatedException(*this);
}

void
IceE::ConnectionNotValidatedException::ice_throw() const
{
    throw *this;
}

IceE::UnknownRequestIdException::UnknownRequestIdException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownRequestIdException_name = "IceE::UnknownRequestIdException";

const ::std::string
IceE::UnknownRequestIdException::ice_name() const
{
    return __Ice__UnknownRequestIdException_name;
}

::IceE::Exception*
IceE::UnknownRequestIdException::ice_clone() const
{
    return new UnknownRequestIdException(*this);
}

void
IceE::UnknownRequestIdException::ice_throw() const
{
    throw *this;
}

IceE::UnknownReplyStatusException::UnknownReplyStatusException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__UnknownReplyStatusException_name = "IceE::UnknownReplyStatusException";

const ::std::string
IceE::UnknownReplyStatusException::ice_name() const
{
    return __Ice__UnknownReplyStatusException_name;
}

::IceE::Exception*
IceE::UnknownReplyStatusException::ice_clone() const
{
    return new UnknownReplyStatusException(*this);
}

void
IceE::UnknownReplyStatusException::ice_throw() const
{
    throw *this;
}

IceE::CloseConnectionException::CloseConnectionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__CloseConnectionException_name = "IceE::CloseConnectionException";

const ::std::string
IceE::CloseConnectionException::ice_name() const
{
    return __Ice__CloseConnectionException_name;
}

::IceE::Exception*
IceE::CloseConnectionException::ice_clone() const
{
    return new CloseConnectionException(*this);
}

void
IceE::CloseConnectionException::ice_throw() const
{
    throw *this;
}

IceE::ForcedCloseConnectionException::ForcedCloseConnectionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__ForcedCloseConnectionException_name = "IceE::ForcedCloseConnectionException";

const ::std::string
IceE::ForcedCloseConnectionException::ice_name() const
{
    return __Ice__ForcedCloseConnectionException_name;
}

::IceE::Exception*
IceE::ForcedCloseConnectionException::ice_clone() const
{
    return new ForcedCloseConnectionException(*this);
}

void
IceE::ForcedCloseConnectionException::ice_throw() const
{
    throw *this;
}

IceE::IllegalMessageSizeException::IllegalMessageSizeException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__IllegalMessageSizeException_name = "IceE::IllegalMessageSizeException";

const ::std::string
IceE::IllegalMessageSizeException::ice_name() const
{
    return __Ice__IllegalMessageSizeException_name;
}

::IceE::Exception*
IceE::IllegalMessageSizeException::ice_clone() const
{
    return new IllegalMessageSizeException(*this);
}

void
IceE::IllegalMessageSizeException::ice_throw() const
{
    throw *this;
}

IceE::CompressionNotSupportedException::CompressionNotSupportedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

static const char* __Ice__CompressionNotSupportedException_name = "IceE::CompressionNotSupportedException";

const ::std::string
IceE::CompressionNotSupportedException::ice_name() const
{
    return __Ice__CompressionNotSupportedException_name;
}

::IceE::Exception*
IceE::CompressionNotSupportedException::ice_clone() const
{
    return new CompressionNotSupportedException(*this);
}

void
IceE::CompressionNotSupportedException::ice_throw() const
{
    throw *this;
}

IceE::MarshalException::MarshalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::IceE::ProtocolException(__file, __line)
#endif
{
}

IceE::MarshalException::MarshalException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line),
#else
    ::IceE::ProtocolException(__file, __line),
#endif
    reason(__reason)
{
}

static const char* __Ice__MarshalException_name = "IceE::MarshalException";

const ::std::string
IceE::MarshalException::ice_name() const
{
    return __Ice__MarshalException_name;
}

::IceE::Exception*
IceE::MarshalException::ice_clone() const
{
    return new MarshalException(*this);
}

void
IceE::MarshalException::ice_throw() const
{
    throw *this;
}

IceE::ProxyUnmarshalException::ProxyUnmarshalException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::IceE::MarshalException(__file, __line)
#endif
{
}

IceE::ProxyUnmarshalException::ProxyUnmarshalException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::IceE::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__ProxyUnmarshalException_name = "IceE::ProxyUnmarshalException";

const ::std::string
IceE::ProxyUnmarshalException::ice_name() const
{
    return __Ice__ProxyUnmarshalException_name;
}

::IceE::Exception*
IceE::ProxyUnmarshalException::ice_clone() const
{
    return new ProxyUnmarshalException(*this);
}

void
IceE::ProxyUnmarshalException::ice_throw() const
{
    throw *this;
}

IceE::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::IceE::MarshalException(__file, __line)
#endif
{
}

IceE::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::IceE::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__UnmarshalOutOfBoundsException_name = "IceE::UnmarshalOutOfBoundsException";

const ::std::string
IceE::UnmarshalOutOfBoundsException::ice_name() const
{
    return __Ice__UnmarshalOutOfBoundsException_name;
}

::IceE::Exception*
IceE::UnmarshalOutOfBoundsException::ice_clone() const
{
    return new UnmarshalOutOfBoundsException(*this);
}

void
IceE::UnmarshalOutOfBoundsException::ice_throw() const
{
    throw *this;
}

IceE::MemoryLimitException::MemoryLimitException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::IceE::MarshalException(__file, __line)
#endif
{
}

IceE::MemoryLimitException::MemoryLimitException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::IceE::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__MemoryLimitException_name = "IceE::MemoryLimitException";

const ::std::string
IceE::MemoryLimitException::ice_name() const
{
    return __Ice__MemoryLimitException_name;
}

::IceE::Exception*
IceE::MemoryLimitException::ice_clone() const
{
    return new MemoryLimitException(*this);
}

void
IceE::MemoryLimitException::ice_throw() const
{
    throw *this;
}

IceE::EncapsulationException::EncapsulationException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::IceE::MarshalException(__file, __line)
#endif
{
}

IceE::EncapsulationException::EncapsulationException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::IceE::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__EncapsulationException_name = "IceE::EncapsulationException";

const ::std::string
IceE::EncapsulationException::ice_name() const
{
    return __Ice__EncapsulationException_name;
}

::IceE::Exception*
IceE::EncapsulationException::ice_clone() const
{
    return new EncapsulationException(*this);
}

void
IceE::EncapsulationException::ice_throw() const
{
    throw *this;
}

IceE::NegativeSizeException::NegativeSizeException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::IceE::MarshalException(__file, __line)
#endif
{
}

IceE::NegativeSizeException::NegativeSizeException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::IceE::MarshalException(__file, __line, __reason)
#endif
{
}

static const char* __Ice__NegativeSizeException_name = "IceE::NegativeSizeException";

const ::std::string
IceE::NegativeSizeException::ice_name() const
{
    return __Ice__NegativeSizeException_name;
}

::IceE::Exception*
IceE::NegativeSizeException::ice_clone() const
{
    return new NegativeSizeException(*this);
}

void
IceE::NegativeSizeException::ice_throw() const
{
    throw *this;
}

bool
IceE::UserException::__usesClasses() const
{
    return false;
}

string
IceE::UnknownException::toString() const
{
    string out = Exception::toString();
    out +=  ":\nunknown exception";
    if(!unknown.empty())
    {
	out += ":\n";
	out += unknown;
    }
    return out;
}

string
IceE::UnknownLocalException::toString() const
{
    string out = Exception::toString();
    out += ":\nunknown local exception";
    if(!unknown.empty())
    {
	out += ":\n";
	out += unknown;
    }
    return out;
}

string
IceE::UnknownUserException::toString() const
{
    string out = Exception::toString();
    out += ":\nunknown user exception";
    if(!unknown.empty())
    {
	out += ":\n";
	out += unknown;
    }
    return out;
}

string
IceE::VersionMismatchException::toString() const
{
    string out = Exception::toString();
    out += ":\nIce library version mismatch";
    return out;
}

string
IceE::CommunicatorDestroyedException::toString() const
{
    string out = Exception::toString();
    out += ":\ncommunicator object destroyed";
    return out;
}

#ifdef never
#endif

string
IceE::NoEndpointException::toString() const
{
    string out = Exception::toString();
    out += ":\nno suitable endpoint available for proxy `";
    out += proxy;
    out += "'";
    return out;
}

string
IceE::EndpointParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing endpoint `";
    out += str;
    out += "'";
    return out;
}

string
IceE::IdentityParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing identity `";
    out += str; 
    out += "'";
    return out;
}

string
IceE::ProxyParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing proxy `";
    out += str;
    out += "'";
    return out;
}

string
IceE::IllegalIdentityException::toString() const
{
    string out = Exception::toString();
    out += ":\nillegal identity: `";
    out += identityToString(id);
    out += "'";
    return out;
}

static void
printFailedRequestData(string& out, const RequestFailedException& ex)
{
    out += "\nidentity: ";
    out += identityToString(ex.id);
    out += "\nfacet: ";
    out += ex.facet;
    out += "\noperation: ";
    out += ex.operation;
}

string
IceE::RequestFailedException::toString() const
{
    string out = Exception::toString();
    out += ":\nrequest failed";
    printFailedRequestData(out, *this);
    return out;
}

string
IceE::ObjectNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\nobject does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
IceE::FacetNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\nfacet does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
IceE::OperationNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\noperation does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
IceE::SyscallException::toString() const
{
    string out = Exception::toString();
    if(error != 0)
    {
        out += ":\nsyscall exception: ";
	out += errorToString(error);
    }
    return out;
}

string
IceE::SocketException::toString() const
{
    string out = Exception::toString();
    out += ":\nsocket exception: ";
    out += errorToString(error);
    return out;
}

string
IceE::FileException::toString() const
{
    string out = Exception::toString();
    out += ":\nfile exception: ";
    out += errorToString(error);
    if(!path.empty())
    {
	out += "\npath: ";
	out += path;
    }
    return out;
}

string
IceE::ConnectFailedException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnect failed: ";
    out += errorToString(error);
    return out;
}

string
IceE::ConnectionRefusedException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnection refused: ";
    out += errorToString(error);
    return out;
}

string
IceE::ConnectionLostException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnection lost: ";
    if(error == 0)
    {
	out += "recv() returned zero";
    }
    else
    {
	out += errorToString(error);
    }
    return out;
}

string
IceE::DNSException::toString() const
{
    string out = Exception::toString();
    out += ":\nDNS error: ";
    out += errorToStringDNS(error);
    out += "\nhost: ";
    out += host;
    return out;
}

string
IceE::TimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while sending or receiving data";
    return out;
}

string
IceE::ConnectTimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while establishing a connection";
    return out;
}

string
IceE::CloseTimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while closing a connection";
    return out;
}

string
IceE::ConnectionTimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnection has timed out";
    return out;
}

string
IceE::ProtocolException::toString() const
{
    string out = Exception::toString();
    out += ":\nunknown protocol exception";
    return out;
}

string
IceE::BadMagicException::toString() const
{
    string out = Exception::toString();
    out += ":\nunknown magic number: ";
    out += IceE::printfToString("0x%2X, 0x%2X, 0x%2X, 0x%2X", badMagic[0], badMagic[1], badMagic[2], badMagic[3]);

    return out;
}

string
IceE::UnsupportedProtocolException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: unsupported protocol version: ";
    out += IceE::printfToString("%d.%d", badMajor, badMinor);
    out += "\n(can only support protocols compatible with version ";
    out += IceE::printfToString("%d.%d", major, minor);
    out += ")";
    return out;
}

string
IceE::UnsupportedEncodingException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: unsupported encoding version: ";
    out += badMajor;
    out += ".";
    out += badMinor;
    out += "\n(can only support encodings compatible with version ";
    out += major;
    out += ".";
    out += minor;
    return out;
}

string
IceE::UnknownMessageException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: unknown message type";
    return out;
}

string
IceE::ConnectionNotValidatedException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: received message over unvalidated connection";
    return out;
}

string
IceE::UnknownRequestIdException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: unknown request id";
    return out;
}

string
IceE::UnknownReplyStatusException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: unknown reply status";
    return out;
}

string
IceE::CloseConnectionException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: connection closed";
    return out;
}

string
IceE::ForcedCloseConnectionException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: connection forcefully closed";
    return out;
}

string
IceE::IllegalMessageSizeException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: illegal message size";
    return out;
}

string
IceE::CompressionNotSupportedException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: compressed messages not supported";
    return out;
}

string
IceE::MarshalException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: error during marshaling or unmarshaling";
    if(!reason.empty())
    {
	out += ":\n";
	out += reason;
    }
    return out;
}

string
IceE::UnmarshalOutOfBoundsException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: out of bounds during unmarshaling";
    return out;
}

string
IceE::ProxyUnmarshalException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: inconsistent proxy data during unmarshaling";
    return out;
}

string
IceE::MemoryLimitException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: memory limit exceeded";
    return out;
}

string
IceE::EncapsulationException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: illegal encapsulation";
    return out;
}

string
IceE::NegativeSizeException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: negative size for sequence, dictionary, etc.";
    return out;
}

string
IceE::AlreadyRegisteredException::toString() const
{
    string out = Exception::toString();
    out += ":\n";
    out += kindOfObject;
    out += " with id `";
    out += id; 
    out += "' is already registered";
    return out;
}

string
IceE::NotRegisteredException::toString() const
{
    string out = Exception::toString();
    out += ":\n no ";
    out += kindOfObject;
    out += " with id `";
    out += id;
    out += "' is registered";
    return out;
}

string
IceE::TwowayOnlyException::toString() const
{
    string out = Exception::toString();
    out += ":\n operation `";
    out += operation; 
    out += "' can only be invoked as a twoway request";
    return out;
}

string
IceE::CloneNotImplementedException::toString() const
{
    string out = Exception::toString();
    out += ":\n ice_clone() must be implemented in classes derived from abstract base classes";
    return out;
}
