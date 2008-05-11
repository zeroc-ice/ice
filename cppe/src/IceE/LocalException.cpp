// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalException.h>
#include <IceE/SafeStdio.h>
#include <IceE/StringUtil.h>
#include <IceE/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::LocalException::~LocalException() throw()
{
}

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

Ice::AlreadyRegisteredException::~AlreadyRegisteredException() throw()
{
}

static const char* __Ice__AlreadyRegisteredException_name = "Ice::AlreadyRegisteredException";

::std::string
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

Ice::NotRegisteredException::~NotRegisteredException() throw()
{
}

static const char* __Ice__NotRegisteredException_name = "Ice::NotRegisteredException";

::std::string
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

Ice::TwowayOnlyException::~TwowayOnlyException() throw()
{
}

static const char* __Ice__TwowayOnlyException_name = "Ice::TwowayOnlyException";

::std::string
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

Ice::CloneNotImplementedException::~CloneNotImplementedException() throw()
{
}

static const char* __Ice__CloneNotImplementedException_name = "Ice::CloneNotImplementedException";

::std::string
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

Ice::UnknownException::~UnknownException() throw()
{
}

static const char* __Ice__UnknownException_name = "Ice::UnknownException";

::std::string
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

Ice::UnknownLocalException::~UnknownLocalException() throw()
{
}

static const char* __Ice__UnknownLocalException_name = "Ice::UnknownLocalException";

::std::string
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

Ice::UnknownUserException::~UnknownUserException() throw()
{
}

static const char* __Ice__UnknownUserException_name = "Ice::UnknownUserException";

::std::string
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

Ice::VersionMismatchException::~VersionMismatchException() throw()
{
}

static const char* __Ice__VersionMismatchException_name = "Ice::VersionMismatchException";

::std::string
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

Ice::CommunicatorDestroyedException::~CommunicatorDestroyedException() throw()
{
}

static const char* __Ice__CommunicatorDestroyedException_name = "Ice::CommunicatorDestroyedException";

::std::string
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

#ifndef ICEE_PURE_CLIENT

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

Ice::ObjectAdapterDeactivatedException::~ObjectAdapterDeactivatedException() throw()
{
}

static const char* __Ice__ObjectAdapterDeactivatedException_name = "Ice::ObjectAdapterDeactivatedException";

::std::string
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
string
Ice::ObjectAdapterDeactivatedException::toString() const
{
    string out = Exception::toString();
    out +=  ":\nobject adapter `";
    out += name;
    out += "' deactivated";
    return out;
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

Ice::ObjectAdapterIdInUseException::~ObjectAdapterIdInUseException() throw()
{
}

static const char* __Ice__ObjectAdapterIdInUseException_name = "Ice::ObjectAdapterIdInUseException";

::std::string
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

string
Ice::ObjectAdapterIdInUseException::toString() const
{
#ifdef ICEE_HAS_LOCATOR
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

Ice::NoEndpointException::~NoEndpointException() throw()
{
}

static const char* __Ice__NoEndpointException_name = "Ice::NoEndpointException";

::std::string
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

Ice::EndpointParseException::~EndpointParseException() throw()
{
}

static const char* __Ice__EndpointParseException_name = "Ice::EndpointParseException";

::std::string
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

Ice::IdentityParseException::~IdentityParseException() throw()
{
}

static const char* __Ice__IdentityParseException_name = "Ice::IdentityParseException";

::std::string
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

Ice::ProxyParseException::~ProxyParseException() throw()
{
}

static const char* __Ice__ProxyParseException_name = "Ice::ProxyParseException";

::std::string
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

Ice::IllegalIdentityException::~IllegalIdentityException() throw()
{
}

static const char* __Ice__IllegalIdentityException_name = "Ice::IllegalIdentityException";

::std::string
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

Ice::RequestFailedException::~RequestFailedException() throw()
{
}

static const char* __Ice__RequestFailedException_name = "Ice::RequestFailedException";

::std::string
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

Ice::ObjectNotExistException::~ObjectNotExistException() throw()
{
}

static const char* __Ice__ObjectNotExistException_name = "Ice::ObjectNotExistException";

::std::string
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

Ice::FacetNotExistException::~FacetNotExistException() throw()
{
}

static const char* __Ice__FacetNotExistException_name = "Ice::FacetNotExistException";

::std::string
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

Ice::OperationNotExistException::~OperationNotExistException() throw()
{
}

static const char* __Ice__OperationNotExistException_name = "Ice::OperationNotExistException";

::std::string
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

Ice::SyscallException::~SyscallException() throw()
{
}

static const char* __Ice__SyscallException_name = "Ice::SyscallException";

::std::string
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

Ice::SocketException::~SocketException() throw()
{
}

static const char* __Ice__SocketException_name = "Ice::SocketException";

::std::string
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

Ice::FileException::~FileException() throw()
{
}

static const char* __Ice__FileException_name = "Ice::FileException";

::std::string
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

Ice::ConnectFailedException::~ConnectFailedException() throw()
{
}

static const char* __Ice__ConnectFailedException_name = "Ice::ConnectFailedException";

::std::string
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

Ice::ConnectionRefusedException::~ConnectionRefusedException() throw()
{
}

static const char* __Ice__ConnectionRefusedException_name = "Ice::ConnectionRefusedException";

::std::string
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

Ice::ConnectionLostException::~ConnectionLostException() throw()
{
}

static const char* __Ice__ConnectionLostException_name = "Ice::ConnectionLostException";

::std::string
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

Ice::DNSException::~DNSException() throw()
{
}

static const char* __Ice__DNSException_name = "Ice::DNSException";

::std::string
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

Ice::TimeoutException::~TimeoutException() throw()
{
}

static const char* __Ice__TimeoutException_name = "Ice::TimeoutException";

::std::string
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

Ice::ConnectTimeoutException::~ConnectTimeoutException() throw()
{
}

static const char* __Ice__ConnectTimeoutException_name = "Ice::ConnectTimeoutException";

::std::string
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

Ice::CloseTimeoutException::~CloseTimeoutException() throw()
{
}

static const char* __Ice__CloseTimeoutException_name = "Ice::CloseTimeoutException";

::std::string
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

Ice::ProtocolException::ProtocolException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::ProtocolException::ProtocolException(const char* __file, int __line, const ::std::string& __reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    reason(__reason)
{
}

Ice::ProtocolException::~ProtocolException() throw()
{
}

static const char* __Ice__ProtocolException_name = "Ice::ProtocolException";

::std::string
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

Ice::CloseConnectionException::CloseConnectionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ProtocolException(__file, __line)
#else
    ::Ice::ProtocolException(__file, __line)
#endif
{
}

Ice::CloseConnectionException::~CloseConnectionException() throw()
{
}

static const char* __Ice__CloseConnectionException_name = "Ice::CloseConnectionException";

::std::string
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

Ice::ForcedCloseConnectionException::~ForcedCloseConnectionException() throw()
{
}

static const char* __Ice__ForcedCloseConnectionException_name = "Ice::ForcedCloseConnectionException";

::std::string
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

void
Ice::throwBadMagicException(const char* file, int line, const ::Ice::ByteSeq& badMagic)
{
    string out = "unknown magic number: ";
    out += Ice::printfToString("0x%2X, 0x%2X, 0x%2X, 0x%2X", badMagic[0], badMagic[1], badMagic[2], badMagic[3]);
    throw ProtocolException(__FILE__, __LINE__, out);
}

void
Ice::throwUnsupportedProtocolException(const char* file, int line, Int badMajor, Int badMinor, Int major, Int minor)
{
    string out = "unsupported protocol version: ";
    out += Ice::printfToString("%d.%d", badMajor, badMinor);
    out += "\n(can only support protocols compatible with version ";
    out += Ice::printfToString("%d.%d", major, minor) + ")";
    throw ProtocolException(__FILE__, __LINE__, out);
}

void
Ice::throwUnsupportedEncodingException(const char* file, int line, Int badMajor, Int badMinor, Int major, Int minor)
{
    string out = "unsupported encoding version: ";
    out += Ice::printfToString("%d.%d", badMajor, badMinor);
    out += "\n(can only support protocols compatible with version ";
    out += Ice::printfToString("%d.%d", major, minor) + ")";
    throw ProtocolException(__FILE__, __LINE__, out);
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
    ProtocolException(__file, __line, __reason)
#else
    ::Ice::ProtocolException(__file, __line, __reason)
#endif
{
}

Ice::MarshalException::~MarshalException() throw()
{
}

static const char* __Ice__MarshalException_name = "Ice::MarshalException";

::std::string
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

Ice::MemoryLimitException::~MemoryLimitException() throw()
{
}

static const char* __Ice__MemoryLimitException_name = "Ice::MemoryLimitException";

::std::string
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

void
Ice::throwMemoryLimitException(const char* file, int line)
{
     throw MemoryLimitException(file, line);
}

void
Ice::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
     throw MarshalException(file, line, "out of bounds during unmarshaling");
}

void
Ice::throwNegativeSizeException(const char* file, int line)
{
    throw MarshalException(file, line, "negative size for sequence, dictionary, etc.");
}

Ice::FeatureNotSupportedException::FeatureNotSupportedException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::FeatureNotSupportedException::FeatureNotSupportedException(const char* __file, int __line, const string& __unsupportedFeature) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line),
#else
    ::Ice::LocalException(__file, __line),
#endif
    unsupportedFeature(__unsupportedFeature)
{
}

Ice::FeatureNotSupportedException::~FeatureNotSupportedException() throw()
{
}

static const char* __Ice__FeatureNotSupportedException_name = "Ice::FeatureNotSupportedException";

::std::string
Ice::FeatureNotSupportedException::ice_name() const
{
    return __Ice__FeatureNotSupportedException_name;
}

::Ice::Exception*
Ice::FeatureNotSupportedException::ice_clone() const
{
    return new FeatureNotSupportedException(*this);
}

void
Ice::FeatureNotSupportedException::ice_throw() const
{
    throw *this;
}

Ice::FixedProxyException::FixedProxyException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    LocalException(__file, __line)
#else
    ::Ice::LocalException(__file, __line)
#endif
{
}

Ice::FixedProxyException::~FixedProxyException() throw()
{
}

static const char* __Ice__FixedProxyException_name = "Ice::FixedProxyException";

::std::string
Ice::FixedProxyException::ice_name() const
{
    return __Ice__FixedProxyException_name;
}

::Ice::Exception*
Ice::FixedProxyException::ice_clone() const
{
    return new FixedProxyException(*this);
}

void
Ice::FixedProxyException::ice_throw() const
{
    throw *this;
}

#ifdef ICEE_HAS_WSTRING
Ice::StringConversionException::StringConversionException(const char* __file, int __line) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line)
#else
    ::Ice::MarshalException(__file, __line)
#endif
{
}

Ice::StringConversionException::StringConversionException(const char* __file, int __line, const ::std::string& __reason) :  
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    MarshalException(__file, __line, __reason)
#else
    ::Ice::MarshalException(__file, __line, __reason)
#endif
{
}

Ice::StringConversionException::~StringConversionException() throw()
{
}

static const char* __Ice__StringConversionException_name = "Ice::StringConversionException";

::std::string
Ice::StringConversionException::ice_name() const
{
    return __Ice__StringConversionException_name;
}

::Ice::Exception*
Ice::StringConversionException::ice_clone() const
{
    return new StringConversionException(*this);
}

void
Ice::StringConversionException::ice_throw() const
{
    throw *this;
}
#endif

string
Ice::UnknownException::toString() const
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
Ice::UnknownLocalException::toString() const
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
Ice::UnknownUserException::toString() const
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
Ice::VersionMismatchException::toString() const
{
    string out = Exception::toString();
    out += ":\nIce library version mismatch";
    return out;
}

string
Ice::CommunicatorDestroyedException::toString() const
{
    string out = Exception::toString();
    out += ":\ncommunicator object destroyed";
    return out;
}


#ifdef never
#endif

string
Ice::NoEndpointException::toString() const
{
    string out = Exception::toString();
    out += ":\nno suitable endpoint available for proxy `";
    out += proxy;
    out += "'";
    return out;
}

string
Ice::EndpointParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing endpoint `";
    out += str;
    out += "'";
    return out;
}

string
Ice::IdentityParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing identity `";
    out += str; 
    out += "'";
    return out;
}

string
Ice::ProxyParseException::toString() const
{
    string out = Exception::toString();
    out += ":\nerror while parsing proxy `";
    out += str;
    out += "'";
    return out;
}

string
Ice::IllegalIdentityException::toString() const
{
    string out = Exception::toString();
    out += ":\nillegal identity: `";
    if(id.category.empty())
    {
        out += IceUtil::escapeString(id.name, "/");
    }
    else
    {
        out += IceUtil::escapeString(id.category, "/") + '/' + IceUtil::escapeString(id.name, "/");
    }
    out += "'";
    return out;
}

static void
printFailedRequestData(string& out, const RequestFailedException& ex)
{
    out += "\nidentity: ";
    if(ex.id.category.empty())
    {
        out += IceUtil::escapeString(ex.id.name, "/");
    }
    else
    {
        out += IceUtil::escapeString(ex.id.category, "/") + '/' + IceUtil::escapeString(ex.id.name, "/");
    }
    out += "\nfacet: ";
    out += ex.facet;
    out += "\noperation: ";
    out += ex.operation;
}

string
Ice::RequestFailedException::toString() const
{
    string out = Exception::toString();
    out += ":\nrequest failed";
    printFailedRequestData(out, *this);
    return out;
}

string
Ice::ObjectNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\nobject does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
Ice::FacetNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\nfacet does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
Ice::OperationNotExistException::toString() const
{
    string out = Exception::toString();
    out += ":\noperation does not exist";
    printFailedRequestData(out, *this);
    return out;
}

string
Ice::SyscallException::toString() const
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
Ice::SocketException::toString() const
{
    string out = Exception::toString();
    out += ":\nsocket exception: ";
    out += errorToString(error);
    return out;
}

string
Ice::FileException::toString() const
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
Ice::ConnectFailedException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnect failed: ";
    out += errorToString(error);
    return out;
}

string
Ice::ConnectionRefusedException::toString() const
{
    string out = Exception::toString();
    out += ":\nconnection refused: ";
    out += errorToString(error);
    return out;
}

string
Ice::ConnectionLostException::toString() const
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
Ice::DNSException::toString() const
{
    string out = Exception::toString();
    out += ":\nDNS error: ";
    out += errorToStringDNS(error);
    out += "\nhost: ";
    out += host;
    return out;
}

string
Ice::TimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while sending or receiving data";
    return out;
}

string
Ice::ConnectTimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while establishing a connection";
    return out;
}

string
Ice::CloseTimeoutException::toString() const
{
    string out = Exception::toString();
    out += ":\ntimeout while closing a connection";
    return out;
}

string
Ice::ProtocolException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: ";
    if(!reason.empty())
    {
	out += reason;
    }
    else
    {
	out += "unknown protocol exception";
    }
    return out;
}

string
Ice::CloseConnectionException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: connection closed";
    return out;
}

string
Ice::ForcedCloseConnectionException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: connection forcefully closed";
    return out;
}

string
Ice::FeatureNotSupportedException::toString() const
{
    string out = Exception::toString();
    out += ":\nfeature not supported";
    if(!unsupportedFeature.empty())
    {
        out += ": ";
        out += unsupportedFeature;
    }
    return out;
}

string
Ice::MarshalException::toString() const
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
Ice::MemoryLimitException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: memory limit exceeded";
    return out;
}

string
Ice::AlreadyRegisteredException::toString() const
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
Ice::NotRegisteredException::toString() const
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
Ice::TwowayOnlyException::toString() const
{
    string out = Exception::toString();
    out += ":\n operation `";
    out += operation; 
    out += "' can only be invoked as a twoway request";
    return out;
}

string
Ice::CloneNotImplementedException::toString() const
{
    string out = Exception::toString();
    out += ":\n ice_clone() must be implemented in classes derived from abstract base classes";
    return out;
}

string
Ice::FixedProxyException::toString() const
{
    string out = Exception::toString();
    out += ":\nfixed proxy exception";
    return out;
}

#ifdef ICEE_HAS_WSTRING
string
Ice::StringConversionException::toString() const
{
    string out = Exception::toString();
    out += ":\nprotocol error: string conversion failed";
    return out;
}
#endif
