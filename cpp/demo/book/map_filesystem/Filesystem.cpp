// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.0
// Generated from file `Filesystem.ice'

#include <Filesystem.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/ScopedArray.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 303
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 > 50
#       error Beta header file detected
#   endif
#   if ICE_INT_VERSION % 100 < 0
#       error Ice patch level mismatch!
#   endif
#endif

static const ::std::string __Filesystem__Node__name_name = "name";

static const ::std::string __Filesystem__Node__destroy_name = "destroy";

static const ::std::string __Filesystem__File__read_name = "read";

static const ::std::string __Filesystem__File__write_name = "write";

static const ::std::string __Filesystem__Directory__list_name = "list";

static const ::std::string __Filesystem__Directory__find_name = "find";

static const ::std::string __Filesystem__Directory__createFile_name = "createFile";

static const ::std::string __Filesystem__Directory__createDirectory_name = "createDirectory";

::Ice::Object* IceInternal::upCast(::Filesystem::Node* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::Node* p) { return p; }

::Ice::Object* IceInternal::upCast(::Filesystem::File* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::File* p) { return p; }

::Ice::Object* IceInternal::upCast(::Filesystem::Directory* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::Directory* p) { return p; }

::Ice::Object* IceInternal::upCast(::Filesystem::PersistentNode* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::PersistentNode* p) { return p; }

::Ice::Object* IceInternal::upCast(::Filesystem::PersistentFile* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::PersistentFile* p) { return p; }

::Ice::Object* IceInternal::upCast(::Filesystem::PersistentDirectory* p) { return p; }
::IceProxy::Ice::Object* IceInternal::upCast(::IceProxy::Filesystem::PersistentDirectory* p) { return p; }

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::NodePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::Node;
        v->__copyFrom(proxy);
    }
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::FilePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::File;
        v->__copyFrom(proxy);
    }
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::DirectoryPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::Directory;
        v->__copyFrom(proxy);
    }
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::PersistentNodePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::PersistentNode;
        v->__copyFrom(proxy);
    }
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::PersistentFilePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::PersistentFile;
        v->__copyFrom(proxy);
    }
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::PersistentDirectoryPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::Filesystem::PersistentDirectory;
        v->__copyFrom(proxy);
    }
}

Filesystem::GenericError::GenericError(const ::std::string& __ice_reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    UserException(),
#else
    ::Ice::UserException(),
#endif
    reason(__ice_reason)
{
}

Filesystem::GenericError::~GenericError() throw()
{
}

static const char* __Filesystem__GenericError_name = "Filesystem::GenericError";

::std::string
Filesystem::GenericError::ice_name() const
{
    return __Filesystem__GenericError_name;
}

::Ice::Exception*
Filesystem::GenericError::ice_clone() const
{
    return new GenericError(*this);
}

void
Filesystem::GenericError::ice_throw() const
{
    throw *this;
}

void
Filesystem::GenericError::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::Filesystem::GenericError"), false);
    __os->startWriteSlice();
    __os->write(reason);
    __os->endWriteSlice();
}

void
Filesystem::GenericError::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->read(reason);
    __is->endReadSlice();
}

void
Filesystem::GenericError::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::GenericError was not generated with stream support";
    throw ex;
}

void
Filesystem::GenericError::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::GenericError was not generated with stream support";
    throw ex;
}

struct __F__Filesystem__GenericError : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::Filesystem::GenericError();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__Filesystem__GenericError__Ptr = new __F__Filesystem__GenericError;

const ::IceInternal::UserExceptionFactoryPtr&
Filesystem::GenericError::ice_factory()
{
    return __F__Filesystem__GenericError__Ptr;
}

class __F__Filesystem__GenericError__Init
{
public:

    __F__Filesystem__GenericError__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::Filesystem::GenericError", ::Filesystem::GenericError::ice_factory());
    }

    ~__F__Filesystem__GenericError__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::Filesystem::GenericError");
    }
};

static __F__Filesystem__GenericError__Init __F__Filesystem__GenericError__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__GenericError__initializer() {} }
#endif

Filesystem::PermissionDenied::PermissionDenied(const ::std::string& __ice_reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError(__ice_reason)
#else
    ::Filesystem::GenericError(__ice_reason)
#endif
{
}

Filesystem::PermissionDenied::~PermissionDenied() throw()
{
}

static const char* __Filesystem__PermissionDenied_name = "Filesystem::PermissionDenied";

::std::string
Filesystem::PermissionDenied::ice_name() const
{
    return __Filesystem__PermissionDenied_name;
}

::Ice::Exception*
Filesystem::PermissionDenied::ice_clone() const
{
    return new PermissionDenied(*this);
}

void
Filesystem::PermissionDenied::ice_throw() const
{
    throw *this;
}

void
Filesystem::PermissionDenied::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::Filesystem::PermissionDenied"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__write(__os);
#else
    ::Filesystem::GenericError::__write(__os);
#endif
}

void
Filesystem::PermissionDenied::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__read(__is, true);
#else
    ::Filesystem::GenericError::__read(__is, true);
#endif
}

void
Filesystem::PermissionDenied::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::PermissionDenied was not generated with stream support";
    throw ex;
}

void
Filesystem::PermissionDenied::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::PermissionDenied was not generated with stream support";
    throw ex;
}

struct __F__Filesystem__PermissionDenied : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::Filesystem::PermissionDenied();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__Filesystem__PermissionDenied__Ptr = new __F__Filesystem__PermissionDenied;

const ::IceInternal::UserExceptionFactoryPtr&
Filesystem::PermissionDenied::ice_factory()
{
    return __F__Filesystem__PermissionDenied__Ptr;
}

class __F__Filesystem__PermissionDenied__Init
{
public:

    __F__Filesystem__PermissionDenied__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::Filesystem::PermissionDenied", ::Filesystem::PermissionDenied::ice_factory());
    }

    ~__F__Filesystem__PermissionDenied__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::Filesystem::PermissionDenied");
    }
};

static __F__Filesystem__PermissionDenied__Init __F__Filesystem__PermissionDenied__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__PermissionDenied__initializer() {} }
#endif

Filesystem::NameInUse::NameInUse(const ::std::string& __ice_reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError(__ice_reason)
#else
    ::Filesystem::GenericError(__ice_reason)
#endif
{
}

Filesystem::NameInUse::~NameInUse() throw()
{
}

static const char* __Filesystem__NameInUse_name = "Filesystem::NameInUse";

::std::string
Filesystem::NameInUse::ice_name() const
{
    return __Filesystem__NameInUse_name;
}

::Ice::Exception*
Filesystem::NameInUse::ice_clone() const
{
    return new NameInUse(*this);
}

void
Filesystem::NameInUse::ice_throw() const
{
    throw *this;
}

void
Filesystem::NameInUse::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::Filesystem::NameInUse"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__write(__os);
#else
    ::Filesystem::GenericError::__write(__os);
#endif
}

void
Filesystem::NameInUse::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__read(__is, true);
#else
    ::Filesystem::GenericError::__read(__is, true);
#endif
}

void
Filesystem::NameInUse::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::NameInUse was not generated with stream support";
    throw ex;
}

void
Filesystem::NameInUse::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::NameInUse was not generated with stream support";
    throw ex;
}

struct __F__Filesystem__NameInUse : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::Filesystem::NameInUse();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__Filesystem__NameInUse__Ptr = new __F__Filesystem__NameInUse;

const ::IceInternal::UserExceptionFactoryPtr&
Filesystem::NameInUse::ice_factory()
{
    return __F__Filesystem__NameInUse__Ptr;
}

class __F__Filesystem__NameInUse__Init
{
public:

    __F__Filesystem__NameInUse__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::Filesystem::NameInUse", ::Filesystem::NameInUse::ice_factory());
    }

    ~__F__Filesystem__NameInUse__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::Filesystem::NameInUse");
    }
};

static __F__Filesystem__NameInUse__Init __F__Filesystem__NameInUse__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__NameInUse__initializer() {} }
#endif

Filesystem::NoSuchName::NoSuchName(const ::std::string& __ice_reason) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError(__ice_reason)
#else
    ::Filesystem::GenericError(__ice_reason)
#endif
{
}

Filesystem::NoSuchName::~NoSuchName() throw()
{
}

static const char* __Filesystem__NoSuchName_name = "Filesystem::NoSuchName";

::std::string
Filesystem::NoSuchName::ice_name() const
{
    return __Filesystem__NoSuchName_name;
}

::Ice::Exception*
Filesystem::NoSuchName::ice_clone() const
{
    return new NoSuchName(*this);
}

void
Filesystem::NoSuchName::ice_throw() const
{
    throw *this;
}

void
Filesystem::NoSuchName::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::Filesystem::NoSuchName"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__write(__os);
#else
    ::Filesystem::GenericError::__write(__os);
#endif
}

void
Filesystem::NoSuchName::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    GenericError::__read(__is, true);
#else
    ::Filesystem::GenericError::__read(__is, true);
#endif
}

void
Filesystem::NoSuchName::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::NoSuchName was not generated with stream support";
    throw ex;
}

void
Filesystem::NoSuchName::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception Filesystem::NoSuchName was not generated with stream support";
    throw ex;
}

struct __F__Filesystem__NoSuchName : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::Filesystem::NoSuchName();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__Filesystem__NoSuchName__Ptr = new __F__Filesystem__NoSuchName;

const ::IceInternal::UserExceptionFactoryPtr&
Filesystem::NoSuchName::ice_factory()
{
    return __F__Filesystem__NoSuchName__Ptr;
}

class __F__Filesystem__NoSuchName__Init
{
public:

    __F__Filesystem__NoSuchName__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::Filesystem::NoSuchName", ::Filesystem::NoSuchName::ice_factory());
    }

    ~__F__Filesystem__NoSuchName__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::Filesystem::NoSuchName");
    }
};

static __F__Filesystem__NoSuchName__Init __F__Filesystem__NoSuchName__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__NoSuchName__initializer() {} }
#endif

void
Filesystem::__write(::IceInternal::BasicStream* __os, ::Filesystem::NodeType v)
{
    __os->write(static_cast< ::Ice::Byte>(v), 2);
}

void
Filesystem::__read(::IceInternal::BasicStream* __is, ::Filesystem::NodeType& v)
{
    ::Ice::Byte val;
    __is->read(val, 2);
    v = static_cast< ::Filesystem::NodeType>(val);
}

bool
Filesystem::NodeDesc::operator==(const NodeDesc& __rhs) const
{
    if(this == &__rhs)
    {
        return true;
    }
    if(name != __rhs.name)
    {
        return false;
    }
    if(type != __rhs.type)
    {
        return false;
    }
    if(proxy != __rhs.proxy)
    {
        return false;
    }
    return true;
}

bool
Filesystem::NodeDesc::operator<(const NodeDesc& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name < __rhs.name)
    {
        return true;
    }
    else if(__rhs.name < name)
    {
        return false;
    }
    if(type < __rhs.type)
    {
        return true;
    }
    else if(__rhs.type < type)
    {
        return false;
    }
    if(proxy < __rhs.proxy)
    {
        return true;
    }
    else if(__rhs.proxy < proxy)
    {
        return false;
    }
    return false;
}

void
Filesystem::NodeDesc::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    ::Filesystem::__write(__os, type);
    __os->write(::Ice::ObjectPrx(::IceInternal::upCast(proxy.get())));
}

void
Filesystem::NodeDesc::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    ::Filesystem::__read(__is, type);
    ::Filesystem::__read(__is, proxy);
}

void
Filesystem::__writeNodeDescSeq(::IceInternal::BasicStream* __os, const ::Filesystem::NodeDesc* begin, const ::Filesystem::NodeDesc* end)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(end - begin);
    __os->writeSize(size);
    for(int i = 0; i < size; ++i)
    {
        begin[i].__write(__os);
    }
}

void
Filesystem::__readNodeDescSeq(::IceInternal::BasicStream* __is, ::Filesystem::NodeDescSeq& v)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 4);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
        v[i].__read(__is);
        __is->checkSeq();
        __is->endElement();
    }
    __is->endSeq(sz);
}

void
Filesystem::__writeNodeDict(::IceInternal::BasicStream* __os, const ::Filesystem::NodeDict& v)
{
    __os->writeSize(::Ice::Int(v.size()));
    ::Filesystem::NodeDict::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
        __os->write(p->first);
        p->second.__write(__os);
    }
}

void
Filesystem::__readNodeDict(::IceInternal::BasicStream* __is, ::Filesystem::NodeDict& v)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    while(sz--)
    {
        ::std::pair<const  ::std::string, ::Filesystem::NodeDesc> pair;
        __is->read(const_cast< ::std::string&>(pair.first));
        ::Filesystem::NodeDict::iterator __i = v.insert(v.end(), pair);
        __i->second.__read(__is);
    }
}

::std::string
IceProxy::Filesystem::Node::name(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Node__name_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Node* __del = dynamic_cast< ::IceDelegate::Filesystem::Node*>(__delBase.get());
            return __del->name(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, 0, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

void
IceProxy::Filesystem::Node::destroy(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Node__destroy_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Node* __del = dynamic_cast< ::IceDelegate::Filesystem::Node*>(__delBase.get());
            __del->destroy(__ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex, 0);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

const ::std::string&
IceProxy::Filesystem::Node::ice_staticId()
{
    return ::Filesystem::Node::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::Node::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::Node);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::Node::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::Node);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::Node::__newInstance() const
{
    return new Node;
}

::Filesystem::Lines
IceProxy::Filesystem::File::read(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__File__read_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::File* __del = dynamic_cast< ::IceDelegate::Filesystem::File*>(__delBase.get());
            return __del->read(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, 0, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

void
IceProxy::Filesystem::File::write(const ::Filesystem::Lines& text, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__File__write_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::File* __del = dynamic_cast< ::IceDelegate::Filesystem::File*>(__delBase.get());
            __del->write(text, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, 0, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

const ::std::string&
IceProxy::Filesystem::File::ice_staticId()
{
    return ::Filesystem::File::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::File::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::File);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::File::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::File);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::File::__newInstance() const
{
    return new File;
}

::Filesystem::NodeDescSeq
IceProxy::Filesystem::Directory::list(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Directory__list_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Directory* __del = dynamic_cast< ::IceDelegate::Filesystem::Directory*>(__delBase.get());
            return __del->list(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, 0, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

::Filesystem::NodeDesc
IceProxy::Filesystem::Directory::find(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Directory__find_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Directory* __del = dynamic_cast< ::IceDelegate::Filesystem::Directory*>(__delBase.get());
            return __del->find(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, 0, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

::Filesystem::FilePrx
IceProxy::Filesystem::Directory::createFile(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Directory__createFile_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Directory* __del = dynamic_cast< ::IceDelegate::Filesystem::Directory*>(__delBase.get());
            return __del->createFile(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex, 0);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

::Filesystem::DirectoryPrx
IceProxy::Filesystem::Directory::createDirectory(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__Filesystem__Directory__createDirectory_name);
            __delBase = __getDelegate(false);
            ::IceDelegate::Filesystem::Directory* __del = dynamic_cast< ::IceDelegate::Filesystem::Directory*>(__delBase.get());
            return __del->createDirectory(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex, 0);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, 0, __cnt);
        }
    }
}

const ::std::string&
IceProxy::Filesystem::Directory::ice_staticId()
{
    return ::Filesystem::Directory::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::Directory::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::Directory);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::Directory::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::Directory);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::Directory::__newInstance() const
{
    return new Directory;
}

const ::std::string&
IceProxy::Filesystem::PersistentNode::ice_staticId()
{
    return ::Filesystem::PersistentNode::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::PersistentNode::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::PersistentNode);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::PersistentNode::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::PersistentNode);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::PersistentNode::__newInstance() const
{
    return new PersistentNode;
}

const ::std::string&
IceProxy::Filesystem::PersistentFile::ice_staticId()
{
    return ::Filesystem::PersistentFile::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::PersistentFile::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::PersistentFile);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::PersistentFile::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::PersistentFile);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::PersistentFile::__newInstance() const
{
    return new PersistentFile;
}

const ::std::string&
IceProxy::Filesystem::PersistentDirectory::ice_staticId()
{
    return ::Filesystem::PersistentDirectory::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Filesystem::PersistentDirectory::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Filesystem::PersistentDirectory);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Filesystem::PersistentDirectory::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Filesystem::PersistentDirectory);
}

::IceProxy::Ice::Object*
IceProxy::Filesystem::PersistentDirectory::__newInstance() const
{
    return new PersistentDirectory;
}

::std::string
IceDelegateM::Filesystem::Node::name(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Node__name_name, ::Ice::Idempotent, __context);
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::std::string __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        __is->read(__ret);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::Filesystem::Node::destroy(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Node__destroy_name, ::Ice::Normal, __context);
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Filesystem::PermissionDenied&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        __is->endReadEncaps();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Filesystem::Lines
IceDelegateM::Filesystem::File::read(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__File__read_name, ::Ice::Idempotent, __context);
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::Filesystem::Lines __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        __is->read(__ret);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::Filesystem::File::write(const ::Filesystem::Lines& text, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__File__write_name, ::Ice::Idempotent, __context);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        if(text.size() == 0)
        {
            __os->writeSize(0);
        }
        else
        {
            __os->write(&text[0], &text[0] + text.size());
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Filesystem::GenericError&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        __is->endReadEncaps();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Filesystem::NodeDescSeq
IceDelegateM::Filesystem::Directory::list(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Directory__list_name, ::Ice::Idempotent, __context);
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::Filesystem::NodeDescSeq __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        ::Filesystem::__readNodeDescSeq(__is, __ret);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Filesystem::NodeDesc
IceDelegateM::Filesystem::Directory::find(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Directory__find_name, ::Ice::Idempotent, __context);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Filesystem::NoSuchName&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::Filesystem::NodeDesc __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        __ret.__read(__is);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Filesystem::FilePrx
IceDelegateM::Filesystem::Directory::createFile(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Directory__createFile_name, ::Ice::Normal, __context);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Filesystem::NameInUse&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::Filesystem::FilePrx __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        ::Filesystem::__read(__is, __ret);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Filesystem::DirectoryPrx
IceDelegateM::Filesystem::Directory::createDirectory(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__handler.get(), __Filesystem__Directory__createDirectory_name, ::Ice::Normal, __context);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Filesystem::NameInUse&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                ::Ice::UnknownUserException __uue(__FILE__, __LINE__, __ex.ice_name());
                throw __uue;
            }
        }
        ::Filesystem::DirectoryPrx __ret;
        ::IceInternal::BasicStream* __is = __og.is();
        __is->startReadEncaps();
        ::Filesystem::__read(__is, __ret);
        __is->endReadEncaps();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateD::Filesystem::Node::name(const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::std::string& __result, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Node* servant = dynamic_cast< ::Filesystem::Node*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            _result = servant->name(_current);
            return ::Ice::DispatchOK;
        }
        
    private:
        
        ::std::string& _result;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Node__name_name, ::Ice::Idempotent, __context);
    ::std::string __result;
    try
    {
        _DirectI __direct(__result, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

void
IceDelegateD::Filesystem::Node::destroy(const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Node* servant = dynamic_cast< ::Filesystem::Node*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                servant->destroy(_current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Node__destroy_name, ::Ice::Normal, __context);
    try
    {
        _DirectI __direct(__current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Filesystem::PermissionDenied&)
    {
        throw;
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
}

::Filesystem::Lines
IceDelegateD::Filesystem::File::read(const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::Filesystem::Lines& __result, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::File* servant = dynamic_cast< ::Filesystem::File*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            _result = servant->read(_current);
            return ::Ice::DispatchOK;
        }
        
    private:
        
        ::Filesystem::Lines& _result;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__File__read_name, ::Ice::Idempotent, __context);
    ::Filesystem::Lines __result;
    try
    {
        _DirectI __direct(__result, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

void
IceDelegateD::Filesystem::File::write(const ::Filesystem::Lines& text, const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(const ::Filesystem::Lines& text, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _m_text(text)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::File* servant = dynamic_cast< ::Filesystem::File*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                servant->write(_m_text, _current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
        const ::Filesystem::Lines& _m_text;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__File__write_name, ::Ice::Idempotent, __context);
    try
    {
        _DirectI __direct(text, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Filesystem::GenericError&)
    {
        throw;
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
}

::Filesystem::NodeDescSeq
IceDelegateD::Filesystem::Directory::list(const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::Filesystem::NodeDescSeq& __result, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Directory* servant = dynamic_cast< ::Filesystem::Directory*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            _result = servant->list(_current);
            return ::Ice::DispatchOK;
        }
        
    private:
        
        ::Filesystem::NodeDescSeq& _result;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Directory__list_name, ::Ice::Idempotent, __context);
    ::Filesystem::NodeDescSeq __result;
    try
    {
        _DirectI __direct(__result, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

::Filesystem::NodeDesc
IceDelegateD::Filesystem::Directory::find(const ::std::string& name, const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::Filesystem::NodeDesc& __result, const ::std::string& name, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result),
            _m_name(name)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Directory* servant = dynamic_cast< ::Filesystem::Directory*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                _result = servant->find(_m_name, _current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
        ::Filesystem::NodeDesc& _result;
        const ::std::string& _m_name;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Directory__find_name, ::Ice::Idempotent, __context);
    ::Filesystem::NodeDesc __result;
    try
    {
        _DirectI __direct(__result, name, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Filesystem::NoSuchName&)
    {
        throw;
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

::Filesystem::FilePrx
IceDelegateD::Filesystem::Directory::createFile(const ::std::string& name, const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::Filesystem::FilePrx& __result, const ::std::string& name, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result),
            _m_name(name)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Directory* servant = dynamic_cast< ::Filesystem::Directory*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                _result = servant->createFile(_m_name, _current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
        ::Filesystem::FilePrx& _result;
        const ::std::string& _m_name;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Directory__createFile_name, ::Ice::Normal, __context);
    ::Filesystem::FilePrx __result;
    try
    {
        _DirectI __direct(__result, name, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Filesystem::NameInUse&)
    {
        throw;
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

::Filesystem::DirectoryPrx
IceDelegateD::Filesystem::Directory::createDirectory(const ::std::string& name, const ::Ice::Context* __context)
{
    class _DirectI : public ::IceInternal::Direct
    {
    public:

        _DirectI(::Filesystem::DirectoryPrx& __result, const ::std::string& name, const ::Ice::Current& __current) : 
            ::IceInternal::Direct(__current),
            _result(__result),
            _m_name(name)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            ::Filesystem::Directory* servant = dynamic_cast< ::Filesystem::Directory*>(object);
            if(!servant)
            {
                throw ::Ice::OperationNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            try
            {
                _result = servant->createDirectory(_m_name, _current);
                return ::Ice::DispatchOK;
            }
            catch(const ::Ice::UserException& __ex)
            {
                setUserException(__ex);
                return ::Ice::DispatchUserException;
            }
        }
        
    private:
        
        ::Filesystem::DirectoryPrx& _result;
        const ::std::string& _m_name;
    };
    
    ::Ice::Current __current;
    __initCurrent(__current, __Filesystem__Directory__createDirectory_name, ::Ice::Normal, __context);
    ::Filesystem::DirectoryPrx __result;
    try
    {
        _DirectI __direct(__result, name, __current);
        try
        {
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const ::Filesystem::NameInUse&)
    {
        throw;
    }
    catch(const ::Ice::SystemException&)
    {
        throw;
    }
    catch(const ::IceInternal::LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        ::IceInternal::LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw ::IceInternal::LocalExceptionWrapper(::Ice::UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

::Ice::ObjectPtr
Filesystem::Node::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __Filesystem__Node_ids[2] =
{
    "::Filesystem::Node",
    "::Ice::Object"
};

bool
Filesystem::Node::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__Node_ids, __Filesystem__Node_ids + 2, _s);
}

::std::vector< ::std::string>
Filesystem::Node::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__Node_ids[0], &__Filesystem__Node_ids[2]);
}

const ::std::string&
Filesystem::Node::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__Node_ids[0];
}

const ::std::string&
Filesystem::Node::ice_staticId()
{
    return __Filesystem__Node_ids[0];
}

::Ice::DispatchStatus
Filesystem::Node::___name(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    __inS.is()->skipEmptyEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = name(__current);
    __os->write(__ret);
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Filesystem::Node::___destroy(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    __inS.is()->skipEmptyEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        destroy(__current);
    }
    catch(const ::Filesystem::PermissionDenied& __ex)
    {
        __os->write(__ex);
        return ::Ice::DispatchUserException;
    }
    return ::Ice::DispatchOK;
}

static ::std::string __Filesystem__Node_all[] =
{
    "destroy",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "name"
};

::Ice::DispatchStatus
Filesystem::Node::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Filesystem__Node_all, __Filesystem__Node_all + 6, current.operation);
    if(r.first == r.second)
    {
        throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - __Filesystem__Node_all)
    {
        case 0:
        {
            return ___destroy(in, current);
        }
        case 1:
        {
            return ___ice_id(in, current);
        }
        case 2:
        {
            return ___ice_ids(in, current);
        }
        case 3:
        {
            return ___ice_isA(in, current);
        }
        case 4:
        {
            return ___ice_ping(in, current);
        }
        case 5:
        {
            return ___name(in, current);
        }
    }

    assert(false);
    throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
Filesystem::Node::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
Filesystem::Node::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
Filesystem::Node::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::Node was not generated with stream support";
    throw ex;
}

void
Filesystem::Node::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::Node was not generated with stream support";
    throw ex;
}

void 
Filesystem::__patch__NodePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::NodePtr* p = static_cast< ::Filesystem::NodePtr*>(__addr);
    assert(p);
    *p = ::Filesystem::NodePtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::Node::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::Node& l, const ::Filesystem::Node& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::Node& l, const ::Filesystem::Node& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

::Ice::ObjectPtr
Filesystem::File::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __Filesystem__File_ids[3] =
{
    "::Filesystem::File",
    "::Filesystem::Node",
    "::Ice::Object"
};

bool
Filesystem::File::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__File_ids, __Filesystem__File_ids + 3, _s);
}

::std::vector< ::std::string>
Filesystem::File::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__File_ids[0], &__Filesystem__File_ids[3]);
}

const ::std::string&
Filesystem::File::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__File_ids[0];
}

const ::std::string&
Filesystem::File::ice_staticId()
{
    return __Filesystem__File_ids[0];
}

::Ice::DispatchStatus
Filesystem::File::___read(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    __inS.is()->skipEmptyEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Filesystem::Lines __ret = read(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Filesystem::File::___write(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    __is->startReadEncaps();
    ::Filesystem::Lines text;
    __is->read(text);
    __is->endReadEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        write(text, __current);
    }
    catch(const ::Filesystem::GenericError& __ex)
    {
        __os->write(__ex);
        return ::Ice::DispatchUserException;
    }
    return ::Ice::DispatchOK;
}

static ::std::string __Filesystem__File_all[] =
{
    "destroy",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "name",
    "read",
    "write"
};

::Ice::DispatchStatus
Filesystem::File::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Filesystem__File_all, __Filesystem__File_all + 8, current.operation);
    if(r.first == r.second)
    {
        throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - __Filesystem__File_all)
    {
        case 0:
        {
            return ___destroy(in, current);
        }
        case 1:
        {
            return ___ice_id(in, current);
        }
        case 2:
        {
            return ___ice_ids(in, current);
        }
        case 3:
        {
            return ___ice_isA(in, current);
        }
        case 4:
        {
            return ___ice_ping(in, current);
        }
        case 5:
        {
            return ___name(in, current);
        }
        case 6:
        {
            return ___read(in, current);
        }
        case 7:
        {
            return ___write(in, current);
        }
    }

    assert(false);
    throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
Filesystem::File::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
Filesystem::File::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
Filesystem::File::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::File was not generated with stream support";
    throw ex;
}

void
Filesystem::File::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::File was not generated with stream support";
    throw ex;
}

void 
Filesystem::__patch__FilePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::FilePtr* p = static_cast< ::Filesystem::FilePtr*>(__addr);
    assert(p);
    *p = ::Filesystem::FilePtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::File::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::File& l, const ::Filesystem::File& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::File& l, const ::Filesystem::File& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

::Ice::ObjectPtr
Filesystem::Directory::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __Filesystem__Directory_ids[3] =
{
    "::Filesystem::Directory",
    "::Filesystem::Node",
    "::Ice::Object"
};

bool
Filesystem::Directory::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__Directory_ids, __Filesystem__Directory_ids + 3, _s);
}

::std::vector< ::std::string>
Filesystem::Directory::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__Directory_ids[0], &__Filesystem__Directory_ids[3]);
}

const ::std::string&
Filesystem::Directory::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__Directory_ids[0];
}

const ::std::string&
Filesystem::Directory::ice_staticId()
{
    return __Filesystem__Directory_ids[0];
}

::Ice::DispatchStatus
Filesystem::Directory::___list(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    __inS.is()->skipEmptyEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Filesystem::NodeDescSeq __ret = list(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        ::Filesystem::__writeNodeDescSeq(__os, &__ret[0], &__ret[0] + __ret.size());
    }
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Filesystem::Directory::___find(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    __is->startReadEncaps();
    ::std::string name;
    __is->read(name);
    __is->endReadEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::Filesystem::NodeDesc __ret = find(name, __current);
        __ret.__write(__os);
    }
    catch(const ::Filesystem::NoSuchName& __ex)
    {
        __os->write(__ex);
        return ::Ice::DispatchUserException;
    }
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Filesystem::Directory::___createFile(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    __is->startReadEncaps();
    ::std::string name;
    __is->read(name);
    __is->endReadEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::Filesystem::FilePrx __ret = createFile(name, __current);
        __os->write(::Ice::ObjectPrx(::IceInternal::upCast(__ret.get())));
    }
    catch(const ::Filesystem::NameInUse& __ex)
    {
        __os->write(__ex);
        return ::Ice::DispatchUserException;
    }
    return ::Ice::DispatchOK;
}

::Ice::DispatchStatus
Filesystem::Directory::___createDirectory(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    __is->startReadEncaps();
    ::std::string name;
    __is->read(name);
    __is->endReadEncaps();
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::Filesystem::DirectoryPrx __ret = createDirectory(name, __current);
        __os->write(::Ice::ObjectPrx(::IceInternal::upCast(__ret.get())));
    }
    catch(const ::Filesystem::NameInUse& __ex)
    {
        __os->write(__ex);
        return ::Ice::DispatchUserException;
    }
    return ::Ice::DispatchOK;
}

static ::std::string __Filesystem__Directory_all[] =
{
    "createDirectory",
    "createFile",
    "destroy",
    "find",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "list",
    "name"
};

::Ice::DispatchStatus
Filesystem::Directory::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Filesystem__Directory_all, __Filesystem__Directory_all + 10, current.operation);
    if(r.first == r.second)
    {
        throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - __Filesystem__Directory_all)
    {
        case 0:
        {
            return ___createDirectory(in, current);
        }
        case 1:
        {
            return ___createFile(in, current);
        }
        case 2:
        {
            return ___destroy(in, current);
        }
        case 3:
        {
            return ___find(in, current);
        }
        case 4:
        {
            return ___ice_id(in, current);
        }
        case 5:
        {
            return ___ice_ids(in, current);
        }
        case 6:
        {
            return ___ice_isA(in, current);
        }
        case 7:
        {
            return ___ice_ping(in, current);
        }
        case 8:
        {
            return ___list(in, current);
        }
        case 9:
        {
            return ___name(in, current);
        }
    }

    assert(false);
    throw ::Ice::OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
}

void
Filesystem::Directory::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
Filesystem::Directory::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
Filesystem::Directory::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::Directory was not generated with stream support";
    throw ex;
}

void
Filesystem::Directory::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::Directory was not generated with stream support";
    throw ex;
}

void 
Filesystem::__patch__DirectoryPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::DirectoryPtr* p = static_cast< ::Filesystem::DirectoryPtr*>(__addr);
    assert(p);
    *p = ::Filesystem::DirectoryPtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::Directory::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::Directory& l, const ::Filesystem::Directory& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::Directory& l, const ::Filesystem::Directory& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

Filesystem::PersistentNode::PersistentNode(const ::std::string& __ice_name) :
    name(__ice_name)
{
}

::Ice::ObjectPtr
Filesystem::PersistentNode::ice_clone() const
{
    ::Filesystem::PersistentNodePtr __p = new ::Filesystem::PersistentNode(*this);
    return __p;
}

static const ::std::string __Filesystem__PersistentNode_ids[2] =
{
    "::Filesystem::PersistentNode",
    "::Ice::Object"
};

bool
Filesystem::PersistentNode::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__PersistentNode_ids, __Filesystem__PersistentNode_ids + 2, _s);
}

::std::vector< ::std::string>
Filesystem::PersistentNode::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__PersistentNode_ids[0], &__Filesystem__PersistentNode_ids[2]);
}

const ::std::string&
Filesystem::PersistentNode::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__PersistentNode_ids[0];
}

const ::std::string&
Filesystem::PersistentNode::ice_staticId()
{
    return __Filesystem__PersistentNode_ids[0];
}

void
Filesystem::PersistentNode::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->write(name);
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
Filesystem::PersistentNode::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->read(name);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
Filesystem::PersistentNode::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentNode was not generated with stream support";
    throw ex;
}

void
Filesystem::PersistentNode::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentNode was not generated with stream support";
    throw ex;
}

class __F__Filesystem__PersistentNode : public ::Ice::ObjectFactory
{
public:

    virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
    {
        assert(type == ::Filesystem::PersistentNode::ice_staticId());
        return new ::Filesystem::PersistentNode;
    }

    virtual void
    destroy()
    {
    }
};

static ::Ice::ObjectFactoryPtr __F__Filesystem__PersistentNode_Ptr = new __F__Filesystem__PersistentNode;

const ::Ice::ObjectFactoryPtr&
Filesystem::PersistentNode::ice_factory()
{
    return __F__Filesystem__PersistentNode_Ptr;
}

class __F__Filesystem__PersistentNode__Init
{
public:

    __F__Filesystem__PersistentNode__Init()
    {
        ::IceInternal::factoryTable->addObjectFactory(::Filesystem::PersistentNode::ice_staticId(), ::Filesystem::PersistentNode::ice_factory());
    }

    ~__F__Filesystem__PersistentNode__Init()
    {
        ::IceInternal::factoryTable->removeObjectFactory(::Filesystem::PersistentNode::ice_staticId());
    }
};

static __F__Filesystem__PersistentNode__Init __F__Filesystem__PersistentNode__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__PersistentNode__initializer() {} }
#endif

void 
Filesystem::__patch__PersistentNodePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::PersistentNodePtr* p = static_cast< ::Filesystem::PersistentNodePtr*>(__addr);
    assert(p);
    *p = ::Filesystem::PersistentNodePtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::PersistentNode::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::PersistentNode& l, const ::Filesystem::PersistentNode& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::PersistentNode& l, const ::Filesystem::PersistentNode& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

Filesystem::PersistentFile::PersistentFile(const ::std::string& __ice_name, const ::Filesystem::Lines& __ice_text) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode(__ice_name)
#else
    ::Filesystem::PersistentNode(__ice_name)
#endif
,
    text(__ice_text)
{
}

::Ice::ObjectPtr
Filesystem::PersistentFile::ice_clone() const
{
    ::Filesystem::PersistentFilePtr __p = new ::Filesystem::PersistentFile(*this);
    return __p;
}

static const ::std::string __Filesystem__PersistentFile_ids[3] =
{
    "::Filesystem::PersistentFile",
    "::Filesystem::PersistentNode",
    "::Ice::Object"
};

bool
Filesystem::PersistentFile::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__PersistentFile_ids, __Filesystem__PersistentFile_ids + 3, _s);
}

::std::vector< ::std::string>
Filesystem::PersistentFile::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__PersistentFile_ids[0], &__Filesystem__PersistentFile_ids[3]);
}

const ::std::string&
Filesystem::PersistentFile::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__PersistentFile_ids[0];
}

const ::std::string&
Filesystem::PersistentFile::ice_staticId()
{
    return __Filesystem__PersistentFile_ids[0];
}

void
Filesystem::PersistentFile::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    if(text.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&text[0], &text[0] + text.size());
    }
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode::__write(__os);
#else
    ::Filesystem::PersistentNode::__write(__os);
#endif
}

void
Filesystem::PersistentFile::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->read(text);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode::__read(__is, true);
#else
    ::Filesystem::PersistentNode::__read(__is, true);
#endif
}

void
Filesystem::PersistentFile::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentFile was not generated with stream support";
    throw ex;
}

void
Filesystem::PersistentFile::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentFile was not generated with stream support";
    throw ex;
}

class __F__Filesystem__PersistentFile : public ::Ice::ObjectFactory
{
public:

    virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
    {
        assert(type == ::Filesystem::PersistentFile::ice_staticId());
        return new ::Filesystem::PersistentFile;
    }

    virtual void
    destroy()
    {
    }
};

static ::Ice::ObjectFactoryPtr __F__Filesystem__PersistentFile_Ptr = new __F__Filesystem__PersistentFile;

const ::Ice::ObjectFactoryPtr&
Filesystem::PersistentFile::ice_factory()
{
    return __F__Filesystem__PersistentFile_Ptr;
}

class __F__Filesystem__PersistentFile__Init
{
public:

    __F__Filesystem__PersistentFile__Init()
    {
        ::IceInternal::factoryTable->addObjectFactory(::Filesystem::PersistentFile::ice_staticId(), ::Filesystem::PersistentFile::ice_factory());
    }

    ~__F__Filesystem__PersistentFile__Init()
    {
        ::IceInternal::factoryTable->removeObjectFactory(::Filesystem::PersistentFile::ice_staticId());
    }
};

static __F__Filesystem__PersistentFile__Init __F__Filesystem__PersistentFile__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__PersistentFile__initializer() {} }
#endif

void 
Filesystem::__patch__PersistentFilePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::PersistentFilePtr* p = static_cast< ::Filesystem::PersistentFilePtr*>(__addr);
    assert(p);
    *p = ::Filesystem::PersistentFilePtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::PersistentFile::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::PersistentFile& l, const ::Filesystem::PersistentFile& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::PersistentFile& l, const ::Filesystem::PersistentFile& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

Filesystem::PersistentDirectory::PersistentDirectory(const ::std::string& __ice_name, const ::Filesystem::NodeDict& __ice_nodes) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode(__ice_name)
#else
    ::Filesystem::PersistentNode(__ice_name)
#endif
,
    nodes(__ice_nodes)
{
}

::Ice::ObjectPtr
Filesystem::PersistentDirectory::ice_clone() const
{
    ::Filesystem::PersistentDirectoryPtr __p = new ::Filesystem::PersistentDirectory(*this);
    return __p;
}

static const ::std::string __Filesystem__PersistentDirectory_ids[3] =
{
    "::Filesystem::PersistentDirectory",
    "::Filesystem::PersistentNode",
    "::Ice::Object"
};

bool
Filesystem::PersistentDirectory::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Filesystem__PersistentDirectory_ids, __Filesystem__PersistentDirectory_ids + 3, _s);
}

::std::vector< ::std::string>
Filesystem::PersistentDirectory::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Filesystem__PersistentDirectory_ids[0], &__Filesystem__PersistentDirectory_ids[3]);
}

const ::std::string&
Filesystem::PersistentDirectory::ice_id(const ::Ice::Current&) const
{
    return __Filesystem__PersistentDirectory_ids[0];
}

const ::std::string&
Filesystem::PersistentDirectory::ice_staticId()
{
    return __Filesystem__PersistentDirectory_ids[0];
}

void
Filesystem::PersistentDirectory::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    ::Filesystem::__writeNodeDict(__os, nodes);
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode::__write(__os);
#else
    ::Filesystem::PersistentNode::__write(__os);
#endif
}

void
Filesystem::PersistentDirectory::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    ::Filesystem::__readNodeDict(__is, nodes);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    PersistentNode::__read(__is, true);
#else
    ::Filesystem::PersistentNode::__read(__is, true);
#endif
}

void
Filesystem::PersistentDirectory::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentDirectory was not generated with stream support";
    throw ex;
}

void
Filesystem::PersistentDirectory::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Filesystem::PersistentDirectory was not generated with stream support";
    throw ex;
}

class __F__Filesystem__PersistentDirectory : public ::Ice::ObjectFactory
{
public:

    virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
    {
        assert(type == ::Filesystem::PersistentDirectory::ice_staticId());
        return new ::Filesystem::PersistentDirectory;
    }

    virtual void
    destroy()
    {
    }
};

static ::Ice::ObjectFactoryPtr __F__Filesystem__PersistentDirectory_Ptr = new __F__Filesystem__PersistentDirectory;

const ::Ice::ObjectFactoryPtr&
Filesystem::PersistentDirectory::ice_factory()
{
    return __F__Filesystem__PersistentDirectory_Ptr;
}

class __F__Filesystem__PersistentDirectory__Init
{
public:

    __F__Filesystem__PersistentDirectory__Init()
    {
        ::IceInternal::factoryTable->addObjectFactory(::Filesystem::PersistentDirectory::ice_staticId(), ::Filesystem::PersistentDirectory::ice_factory());
    }

    ~__F__Filesystem__PersistentDirectory__Init()
    {
        ::IceInternal::factoryTable->removeObjectFactory(::Filesystem::PersistentDirectory::ice_staticId());
    }
};

static __F__Filesystem__PersistentDirectory__Init __F__Filesystem__PersistentDirectory__i;

#ifdef __APPLE__
extern "C" { void __F__Filesystem__PersistentDirectory__initializer() {} }
#endif

void 
Filesystem::__patch__PersistentDirectoryPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Filesystem::PersistentDirectoryPtr* p = static_cast< ::Filesystem::PersistentDirectoryPtr*>(__addr);
    assert(p);
    *p = ::Filesystem::PersistentDirectoryPtr::dynamicCast(v);
    if(v && !*p)
    {
        IceInternal::Ex::throwUOE(::Filesystem::PersistentDirectory::ice_staticId(), v->ice_id());
    }
}

bool
Filesystem::operator==(const ::Filesystem::PersistentDirectory& l, const ::Filesystem::PersistentDirectory& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Filesystem::operator<(const ::Filesystem::PersistentDirectory& l, const ::Filesystem::PersistentDirectory& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}
