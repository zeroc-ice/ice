// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePatch2/Util.h>
#include <IceGrid/Util.h>
#include <IceGrid/Admin.h>

#include <fstream>

using namespace std;
using namespace IceGrid;

string 
IceGrid::toString(const vector<string>& v, const string& sep)
{
    ostringstream os;
    Ice::StringSeq::const_iterator p = v.begin();
    while(p != v.end())
    {
        os << *p;
        ++p;
        if(p != v.end())
        {
            os << sep;
        }
    }
    return os.str();
}

string
IceGrid::toString(const Ice::Exception& exception)
{
    std::ostringstream os;
    try
    {
        exception.ice_throw();
    }
    catch(const NodeUnreachableException& ex)
    {
        os << ex << ":";
        os << "\nnode: " << ex.name;
        os << "\nreason: " << ex.reason;
    }
    catch(const DeploymentException& ex)
    {
        os << ex << ":";
        os << "\nreason: " << ex.reason;
    }
    catch(const Ice::Exception& ex)
    {
        os << ex;
    }
    return os.str();
}

string
IceGrid::getProperty(const PropertyDescriptorSeq& properties, const string& name, const string& def)
{    
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            return q->value;
        }
    }
    return def;
}

PropertyDescriptor
IceGrid::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

