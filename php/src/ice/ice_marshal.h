// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PHP_ICE_MARSHAL_H
#define ICE_PHP_ICE_MARSHAL_H

#include "ice_common.h"
#include <Slice/Parser.h>

namespace IcePHP
{

class Marshaler;
typedef IceUtil::Handle<Marshaler> MarshalerPtr;

class Marshaler : public IceUtil::SimpleShared
{
public:
    virtual ~Marshaler();

    static MarshalerPtr createMarshaler(const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createMemberMarshaler(const std::string&, const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createExceptionMarshaler(const Slice::ExceptionPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC) = 0;

    virtual void destroy() = 0;

protected:
    Marshaler();
};

//
// Associates a scoped type id to its marshaler.
//
typedef std::map<std::string, MarshalerPtr> MarshalerMap;

//
// We must subclass BasicStream in order to associate some information with it.
//
class PHPStream : public IceInternal::BasicStream
{
public:
    PHPStream(IceInternal::Instance*);
    ~PHPStream();

    //
    // The object map associates a Zend object handle to an Ice object.
    //
    typedef std::map<unsigned int, Ice::ObjectPtr> ObjectMap;

    //
    // Types for the patch list.
    //
    struct PatchInfo
    {
        zend_class_entry* ce; // The formal type
        zval* zv;             // The destination zval
    };
    typedef std::vector<PatchInfo*> PatchInfoList;

    ObjectMap* objectMap;
    PatchInfoList* patchList;
};

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
class AbortMarshaling
{
};

//
// PHPObjectFactory is an implementation of Ice::ObjectFactory that creates PHP objects.
// It is also the registry for user-defined PHP factory implementations. A single instance
// can be used for all types.
//
class PHPObjectFactory : public Ice::ObjectFactory
{
public:
    PHPObjectFactory(TSRMLS_D);

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    void addObjectFactory(zval*, const std::string& TSRMLS_DC);
    void removeObjectFactory(const std::string& TSRMLS_DC);
    void findObjectFactory(const std::string&, zval* TSRMLS_DC);

private:
#ifdef ZTS
    TSRMLS_D;
#endif
    std::map<std::string, zval*> _factories;
};
typedef IceUtil::Handle<PHPObjectFactory> PHPObjectFactoryPtr;

} // End of namespace IcePHP

#endif
