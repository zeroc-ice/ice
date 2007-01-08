// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PHP_MARSHAL_H
#define ICE_PHP_MARSHAL_H

#include <Config.h>

namespace IcePHP
{

//
// The object map associates a Zend object handle to an Ice object.
//
typedef std::map<unsigned int, Ice::ObjectPtr> ObjectMap;

class Marshaler;
typedef IceUtil::Handle<Marshaler> MarshalerPtr;

class Marshaler : public IceUtil::SimpleShared
{
public:
    virtual ~Marshaler();

    static MarshalerPtr createMarshaler(const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createMemberMarshaler(const std::string&, const Slice::TypePtr& TSRMLS_DC);
    static MarshalerPtr createExceptionMarshaler(const Slice::ExceptionPtr& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC) = 0;
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC) = 0;

    virtual void destroy() = 0;

protected:
    Marshaler();
};

//
// Associates a scoped type id to its marshaler.
//
typedef std::map<std::string, MarshalerPtr> MarshalerMap;

//
// Associates a scoped type id to its factory.
//
typedef std::map<std::string, zval*> ObjectFactoryMap;

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

#ifdef ZTS
private:
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<PHPObjectFactory> PHPObjectFactoryPtr;

} // End of namespace IcePHP

#endif
