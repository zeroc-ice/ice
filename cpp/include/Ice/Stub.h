// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STUB_H
#define ICE_STUB_H

#include <Ice/StubF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Shared.h>
#include <Ice/Outgoing.h>

namespace _Ice
{

//
// Special handle class for Ice objects, which support the static
// member function cast().
//
template<typename T>
class ObjectHandle : public Handle<T>
{
public:
    
    ObjectHandle(T* p = 0) : Handle<T>(p) { }
    ObjectHandle(const ObjectHandle& r) : Handle<T>(r) { }

    ObjectHandle& operator=(const ObjectHandle& r)
    {
	Handle<T>::operator=(r);
	return *this;
    }

    static ObjectHandle<T> cast(const ::Ice::Object& r)
    {
	return T::_cast(r);
    }
};

}

namespace _IceObj { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::SimpleShared
{
public:

    bool operator==(const ObjectI&) const;
    bool operator!=(const ObjectI&) const;

    ::_Ice::Reference _reference() const;

protected:

    ObjectI();
    virtual ~ObjectI();

    ::_IceStub::Ice::Object _getStub();
    virtual ::_IceStubM::Ice::Object _createStubM();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::_Ice::Reference&);

    ::_Ice::Reference reference_;
    ::_IceStub::Ice::Object stub_;

    friend class ::_Ice::ObjectFactoryI; // May create and setup ObjectIs
};

} }

namespace _IceStub { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::SimpleShared
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    virtual void setup(const ::_Ice::Reference&) = 0;

    friend class ::_IceObj::Ice::ObjectI; // May create and setup ObjectIs
};

} }

namespace _IceStubM { namespace Ice
{

class ICE_API ObjectI : virtual public ::_IceStub::Ice::ObjectI
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

    ::_Ice::Reference reference_;

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::_Ice::Reference&);

    friend class ::_IceObj::Ice::ObjectI; // May create and setup ObjectIs
};

} }

#endif
