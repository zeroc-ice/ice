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
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::PicklerI::pickle(const ObjectPtr& servant, std::ostream& out)
{
    BasicStream s(_instance);
    s.startWriteEncaps();
    s.write(servant);
    s.endWriteEncaps();
    out.write(s.b.begin(), s.b.size());
    if (!out)
    {
	throw SystemException(__FILE__, __LINE__);
    }
}

ObjectPtr
Ice::PicklerI::unpickle(const string& type, std::istream& in)
{
    BasicStream s(_instance);
    s.b.resize(4); // Encapsulation length == Ice::Int
    in.read(s.b.begin(), 4);
    if (in.eof())
    {
	throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (!in)
    {
	throw SystemException(__FILE__, __LINE__);
    }

    s.i = s.b.begin();	    
    Int sz;
    s.read(sz);

    // Don't use s.b.resize() here, otherwise no size sanity checks
    // will be done
    s.resize(4 + sz);
    in.read(s.b.begin() + 4, sz);
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
    ObjectPtr servant;
    s.read(servant, type);
    s.endReadEncaps();
    if (!servant)
    {
	throw NoServantFactoryException(__FILE__, __LINE__);
    }
    return servant;    
}

Ice::PicklerI::PicklerI(const InstancePtr& instance) :
    _instance(instance)
{
}
