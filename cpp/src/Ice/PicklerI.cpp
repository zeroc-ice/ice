// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/PicklerI.h>
#include <Ice/Stream.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::PicklerI::pickle(const ObjectPtr& obj, std::ostream& out)
{
    Stream s(_instance);
    s.startWriteEncaps();
    s.write(obj);
    s.endWriteEncaps();
    out.write(s.b.begin(), s.b.size());
    if (!out)
    {
	throw SystemException(__FILE__, __LINE__);
    }
}

ObjectPtr
Ice::PicklerI::unpickle(std::istream& in)
{
    Stream s(_instance);
    s.b.resize(6);
    in.read(s.b.begin(), 6);
    if (in.eof())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (!in)
    {
	throw SystemException(__FILE__, __LINE__);
    }

    s.i = s.b.begin();	    
    bool bigendian;
    s.read(bigendian);
    s.pushBigendian(bigendian);
    Byte encVer;
    s.read(encVer);
    Int sz;
    s.read(sz);
    s.popBigendian();

    // Don't use s.b.resize() here, otherwise no size sanity checks
    // will be done
    s.resize(6 + sz);
    in.read(s.b.begin() + 6, sz);
    if (in.eof())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (!in)
    {
	throw SystemException(__FILE__, __LINE__);
    }

    s.i = s.b.begin();
    s.startReadEncaps();
    ObjectPtr obj;
    s.read(obj, "::Ice::Object");
    s.endReadEncaps();
    if (!obj)
    {
	throw NoFactoryException(__FILE__, __LINE__);
    }
    return obj;    
}

Ice::PicklerI::PicklerI(const InstancePtr& instance) :
    _instance(instance)
{
}
