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

#ifndef __Filesystem_h__
#define __Filesystem_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/Direct.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/FactoryTable.h>
#include <Ice/StreamF.h>
#include <Ice/UndefSysMacros.h>

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

namespace IceProxy
{

namespace Filesystem
{

class Node;

class File;

class Directory;

class PersistentNode;

class PersistentFile;

class PersistentDirectory;

}

}

namespace Filesystem
{

class Node;
bool operator==(const Node&, const Node&);
bool operator<(const Node&, const Node&);

class File;
bool operator==(const File&, const File&);
bool operator<(const File&, const File&);

class Directory;
bool operator==(const Directory&, const Directory&);
bool operator<(const Directory&, const Directory&);

class PersistentNode;
bool operator==(const PersistentNode&, const PersistentNode&);
bool operator<(const PersistentNode&, const PersistentNode&);

class PersistentFile;
bool operator==(const PersistentFile&, const PersistentFile&);
bool operator<(const PersistentFile&, const PersistentFile&);

class PersistentDirectory;
bool operator==(const PersistentDirectory&, const PersistentDirectory&);
bool operator<(const PersistentDirectory&, const PersistentDirectory&);

}

namespace IceInternal
{

::Ice::Object* upCast(::Filesystem::Node*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::Node*);

::Ice::Object* upCast(::Filesystem::File*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::File*);

::Ice::Object* upCast(::Filesystem::Directory*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::Directory*);

::Ice::Object* upCast(::Filesystem::PersistentNode*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::PersistentNode*);

::Ice::Object* upCast(::Filesystem::PersistentFile*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::PersistentFile*);

::Ice::Object* upCast(::Filesystem::PersistentDirectory*);
::IceProxy::Ice::Object* upCast(::IceProxy::Filesystem::PersistentDirectory*);

}

namespace Filesystem
{

typedef ::IceInternal::Handle< ::Filesystem::Node> NodePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::Node> NodePrx;

void __read(::IceInternal::BasicStream*, NodePrx&);
void __patch__NodePtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::Filesystem::File> FilePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::File> FilePrx;

void __read(::IceInternal::BasicStream*, FilePrx&);
void __patch__FilePtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::Filesystem::Directory> DirectoryPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::Directory> DirectoryPrx;

void __read(::IceInternal::BasicStream*, DirectoryPrx&);
void __patch__DirectoryPtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::Filesystem::PersistentNode> PersistentNodePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::PersistentNode> PersistentNodePrx;

void __read(::IceInternal::BasicStream*, PersistentNodePrx&);
void __patch__PersistentNodePtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::Filesystem::PersistentFile> PersistentFilePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::PersistentFile> PersistentFilePrx;

void __read(::IceInternal::BasicStream*, PersistentFilePrx&);
void __patch__PersistentFilePtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::Filesystem::PersistentDirectory> PersistentDirectoryPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Filesystem::PersistentDirectory> PersistentDirectoryPrx;

void __read(::IceInternal::BasicStream*, PersistentDirectoryPrx&);
void __patch__PersistentDirectoryPtr(void*, ::Ice::ObjectPtr&);

}

namespace Filesystem
{

class GenericError : public ::Ice::UserException
{
public:

    GenericError() {}
    explicit GenericError(const ::std::string&);
    virtual ~GenericError() throw();

    virtual ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    ::std::string reason;

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

static GenericError __GenericError_init;

class PermissionDenied : public ::Filesystem::GenericError
{
public:

    PermissionDenied() {}
    explicit PermissionDenied(const ::std::string&);
    virtual ~PermissionDenied() throw();

    virtual ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

class NameInUse : public ::Filesystem::GenericError
{
public:

    NameInUse() {}
    explicit NameInUse(const ::std::string&);
    virtual ~NameInUse() throw();

    virtual ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

class NoSuchName : public ::Filesystem::GenericError
{
public:

    NoSuchName() {}
    explicit NoSuchName(const ::std::string&);
    virtual ~NoSuchName() throw();

    virtual ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

typedef ::std::vector< ::std::string> Lines;

enum NodeType
{
    DirType,
    FileType
};

void __write(::IceInternal::BasicStream*, NodeType);
void __read(::IceInternal::BasicStream*, NodeType&);

struct NodeDesc
{
    ::std::string name;
    ::Filesystem::NodeType type;
    ::Filesystem::NodePrx proxy;

    bool operator==(const NodeDesc&) const;
    bool operator<(const NodeDesc&) const;
    bool operator!=(const NodeDesc& __rhs) const
    {
        return !operator==(__rhs);
    }
    bool operator<=(const NodeDesc& __rhs) const
    {
        return operator<(__rhs) || operator==(__rhs);
    }
    bool operator>(const NodeDesc& __rhs) const
    {
        return !operator<(__rhs) && !operator==(__rhs);
    }
    bool operator>=(const NodeDesc& __rhs) const
    {
        return !operator<(__rhs);
    }

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

typedef ::std::vector< ::Filesystem::NodeDesc> NodeDescSeq;
void __writeNodeDescSeq(::IceInternal::BasicStream*, const ::Filesystem::NodeDesc*, const ::Filesystem::NodeDesc*);
void __readNodeDescSeq(::IceInternal::BasicStream*, NodeDescSeq&);

typedef ::std::map< ::std::string, ::Filesystem::NodeDesc> NodeDict;
void __writeNodeDict(::IceInternal::BasicStream*, const NodeDict&);
void __readNodeDict(::IceInternal::BasicStream*, NodeDict&);

}

namespace IceProxy
{

namespace Filesystem
{

class Node : virtual public ::IceProxy::Ice::Object
{
public:

    ::std::string name()
    {
        return name(0);
    }
    ::std::string name(const ::Ice::Context& __ctx)
    {
        return name(&__ctx);
    }
    
private:

    ::std::string name(const ::Ice::Context*);
    
public:

    void destroy()
    {
        destroy(0);
    }
    void destroy(const ::Ice::Context& __ctx)
    {
        destroy(&__ctx);
    }
    
private:

    void destroy(const ::Ice::Context*);
    
public:
    
    ::IceInternal::ProxyHandle<Node> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Node> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Node*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<Node*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class File : virtual public ::IceProxy::Filesystem::Node
{
public:

    ::Filesystem::Lines read()
    {
        return read(0);
    }
    ::Filesystem::Lines read(const ::Ice::Context& __ctx)
    {
        return read(&__ctx);
    }
    
private:

    ::Filesystem::Lines read(const ::Ice::Context*);
    
public:

    void write(const ::Filesystem::Lines& text)
    {
        write(text, 0);
    }
    void write(const ::Filesystem::Lines& text, const ::Ice::Context& __ctx)
    {
        write(text, &__ctx);
    }
    
private:

    void write(const ::Filesystem::Lines&, const ::Ice::Context*);
    
public:
    
    ::IceInternal::ProxyHandle<File> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<File> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<File*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<File*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class Directory : virtual public ::IceProxy::Filesystem::Node
{
public:

    ::Filesystem::NodeDescSeq list()
    {
        return list(0);
    }
    ::Filesystem::NodeDescSeq list(const ::Ice::Context& __ctx)
    {
        return list(&__ctx);
    }
    
private:

    ::Filesystem::NodeDescSeq list(const ::Ice::Context*);
    
public:

    ::Filesystem::NodeDesc find(const ::std::string& name)
    {
        return find(name, 0);
    }
    ::Filesystem::NodeDesc find(const ::std::string& name, const ::Ice::Context& __ctx)
    {
        return find(name, &__ctx);
    }
    
private:

    ::Filesystem::NodeDesc find(const ::std::string&, const ::Ice::Context*);
    
public:

    ::Filesystem::FilePrx createFile(const ::std::string& name)
    {
        return createFile(name, 0);
    }
    ::Filesystem::FilePrx createFile(const ::std::string& name, const ::Ice::Context& __ctx)
    {
        return createFile(name, &__ctx);
    }
    
private:

    ::Filesystem::FilePrx createFile(const ::std::string&, const ::Ice::Context*);
    
public:

    ::Filesystem::DirectoryPrx createDirectory(const ::std::string& name)
    {
        return createDirectory(name, 0);
    }
    ::Filesystem::DirectoryPrx createDirectory(const ::std::string& name, const ::Ice::Context& __ctx)
    {
        return createDirectory(name, &__ctx);
    }
    
private:

    ::Filesystem::DirectoryPrx createDirectory(const ::std::string&, const ::Ice::Context*);
    
public:
    
    ::IceInternal::ProxyHandle<Directory> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<Directory> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<Directory*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<Directory*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class PersistentNode : virtual public ::IceProxy::Ice::Object
{
public:
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentNode> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentNode*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<PersistentNode*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class PersistentFile : virtual public ::IceProxy::Filesystem::PersistentNode
{
public:
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentFile> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentFile*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<PersistentFile*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class PersistentDirectory : virtual public ::IceProxy::Filesystem::PersistentNode
{
public:
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PersistentDirectory> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PersistentDirectory*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<PersistentDirectory*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

}

}

namespace IceDelegate
{

namespace Filesystem
{

class Node : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::std::string name(const ::Ice::Context*) = 0;

    virtual void destroy(const ::Ice::Context*) = 0;
};

class File : virtual public ::IceDelegate::Filesystem::Node
{
public:

    virtual ::Filesystem::Lines read(const ::Ice::Context*) = 0;

    virtual void write(const ::Filesystem::Lines&, const ::Ice::Context*) = 0;
};

class Directory : virtual public ::IceDelegate::Filesystem::Node
{
public:

    virtual ::Filesystem::NodeDescSeq list(const ::Ice::Context*) = 0;

    virtual ::Filesystem::NodeDesc find(const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::Filesystem::FilePrx createFile(const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::Filesystem::DirectoryPrx createDirectory(const ::std::string&, const ::Ice::Context*) = 0;
};

class PersistentNode : virtual public ::IceDelegate::Ice::Object
{
public:
};

class PersistentFile : virtual public ::IceDelegate::Filesystem::PersistentNode
{
public:
};

class PersistentDirectory : virtual public ::IceDelegate::Filesystem::PersistentNode
{
public:
};

}

}

namespace IceDelegateM
{

namespace Filesystem
{

class Node : virtual public ::IceDelegate::Filesystem::Node,
             virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::std::string name(const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);
};

class File : virtual public ::IceDelegate::Filesystem::File,
             virtual public ::IceDelegateM::Filesystem::Node
{
public:

    virtual ::Filesystem::Lines read(const ::Ice::Context*);

    virtual void write(const ::Filesystem::Lines&, const ::Ice::Context*);
};

class Directory : virtual public ::IceDelegate::Filesystem::Directory,
                  virtual public ::IceDelegateM::Filesystem::Node
{
public:

    virtual ::Filesystem::NodeDescSeq list(const ::Ice::Context*);

    virtual ::Filesystem::NodeDesc find(const ::std::string&, const ::Ice::Context*);

    virtual ::Filesystem::FilePrx createFile(const ::std::string&, const ::Ice::Context*);

    virtual ::Filesystem::DirectoryPrx createDirectory(const ::std::string&, const ::Ice::Context*);
};

class PersistentNode : virtual public ::IceDelegate::Filesystem::PersistentNode,
                       virtual public ::IceDelegateM::Ice::Object
{
public:
};

class PersistentFile : virtual public ::IceDelegate::Filesystem::PersistentFile,
                       virtual public ::IceDelegateM::Filesystem::PersistentNode
{
public:
};

class PersistentDirectory : virtual public ::IceDelegate::Filesystem::PersistentDirectory,
                            virtual public ::IceDelegateM::Filesystem::PersistentNode
{
public:
};

}

}

namespace IceDelegateD
{

namespace Filesystem
{

class Node : virtual public ::IceDelegate::Filesystem::Node,
             virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::std::string name(const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);
};

class File : virtual public ::IceDelegate::Filesystem::File,
             virtual public ::IceDelegateD::Filesystem::Node
{
public:

    virtual ::Filesystem::Lines read(const ::Ice::Context*);

    virtual void write(const ::Filesystem::Lines&, const ::Ice::Context*);
};

class Directory : virtual public ::IceDelegate::Filesystem::Directory,
                  virtual public ::IceDelegateD::Filesystem::Node
{
public:

    virtual ::Filesystem::NodeDescSeq list(const ::Ice::Context*);

    virtual ::Filesystem::NodeDesc find(const ::std::string&, const ::Ice::Context*);

    virtual ::Filesystem::FilePrx createFile(const ::std::string&, const ::Ice::Context*);

    virtual ::Filesystem::DirectoryPrx createDirectory(const ::std::string&, const ::Ice::Context*);
};

class PersistentNode : virtual public ::IceDelegate::Filesystem::PersistentNode,
                       virtual public ::IceDelegateD::Ice::Object
{
public:
};

class PersistentFile : virtual public ::IceDelegate::Filesystem::PersistentFile,
                       virtual public ::IceDelegateD::Filesystem::PersistentNode
{
public:
};

class PersistentDirectory : virtual public ::IceDelegate::Filesystem::PersistentDirectory,
                            virtual public ::IceDelegateD::Filesystem::PersistentNode
{
public:
};

}

}

namespace Filesystem
{

class Node : virtual public ::Ice::Object
{
public:

    typedef NodePrx ProxyType;
    typedef NodePtr PointerType;
    
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::std::string name(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___name(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void destroy(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___destroy(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

class File : virtual public ::Filesystem::Node
{
public:

    typedef FilePrx ProxyType;
    typedef FilePtr PointerType;
    
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::Filesystem::Lines read(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___read(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void write(const ::Filesystem::Lines&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___write(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

class Directory : virtual public ::Filesystem::Node
{
public:

    typedef DirectoryPrx ProxyType;
    typedef DirectoryPtr PointerType;
    
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::Filesystem::NodeDescSeq list(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___list(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Filesystem::NodeDesc find(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___find(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Filesystem::FilePrx createFile(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___createFile(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Filesystem::DirectoryPrx createDirectory(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___createDirectory(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

class PersistentNode : virtual public ::Ice::Object
{
public:

    typedef PersistentNodePrx ProxyType;
    typedef PersistentNodePtr PointerType;
    
    PersistentNode() {}
    explicit PersistentNode(const ::std::string&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    static const ::Ice::ObjectFactoryPtr& ice_factory();

protected:

    virtual ~PersistentNode() {}

    friend class PersistentNode__staticInit;

public:

    ::std::string name;
};

class PersistentNode__staticInit
{
public:

    ::Filesystem::PersistentNode _init;
};

static ::Filesystem::PersistentNode__staticInit _PersistentNode_init;

class PersistentFile : virtual public ::Filesystem::PersistentNode
{
public:

    typedef PersistentFilePrx ProxyType;
    typedef PersistentFilePtr PointerType;
    
    PersistentFile() {}
    PersistentFile(const ::std::string&, const ::Filesystem::Lines&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    static const ::Ice::ObjectFactoryPtr& ice_factory();

protected:

    virtual ~PersistentFile() {}

public:

    ::Filesystem::Lines text;
};

class PersistentDirectory : virtual public ::Filesystem::PersistentNode
{
public:

    typedef PersistentDirectoryPrx ProxyType;
    typedef PersistentDirectoryPtr PointerType;
    
    PersistentDirectory() {}
    PersistentDirectory(const ::std::string&, const ::Filesystem::NodeDict&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    static const ::Ice::ObjectFactoryPtr& ice_factory();

protected:

    virtual ~PersistentDirectory() {}

public:

    ::Filesystem::NodeDict nodes;
};

}

#endif
