//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/Config.h>

#ifdef ICE_CPP11_MAPPING

namespace Ice
{

/** Creates and throws a user exception. */
using UserExceptionFactory = std::function<void(const std::string&)>;

}

namespace IceInternal
{

template<class E>
void
#ifdef NDEBUG
defaultUserExceptionFactory(const std::string&)
#else
defaultUserExceptionFactory(const std::string& typeId)
#endif
{
    assert(typeId == E::ice_staticId());
    throw E();
}

}
#else

namespace Ice
{

/**
 * Creates and throws a user exception.
 * \headerfile Ice/Ice.h
 */
class ICE_API UserExceptionFactory : public IceUtil::Shared
{
public:

    virtual void createAndThrow(const ::std::string&) = 0;
    virtual ~UserExceptionFactory();
};
typedef ::IceUtil::Handle<UserExceptionFactory> UserExceptionFactoryPtr;

}

namespace IceInternal
{

template<class E>
class DefaultUserExceptionFactory : public Ice::UserExceptionFactory
{
public:

    DefaultUserExceptionFactory(const ::std::string& typeId) :
        _typeId(typeId)
    {
    }

#ifdef NDEBUG
    virtual void createAndThrow(const ::std::string&)
#else
    virtual void createAndThrow(const ::std::string& typeId)
#endif
    {
        assert(typeId == _typeId);
        throw E();
    }

private:
    const ::std::string _typeId;
};

}

#endif
#endif
