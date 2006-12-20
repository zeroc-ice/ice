// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

int
IceGrid::getMMVersion(const string& version)
{
    string::size_type minorPos = version.find('.');
    if(minorPos == string::npos || minorPos >= version.size())
    {
	return -1;
    }
    string::size_type patchPos = version.find('.', minorPos + 1);
    
    int v, ver;

    istringstream major(version.substr(0, minorPos));
    major >> v;
    if(major.fail() || v > 99 || v < 1)
    {
	return -1;
    }
    ver = v;
    ver *= 100;

    istringstream minor(version.substr(minorPos + 1, patchPos != string::npos ? patchPos : version.size()));
    minor >> v;
    if(minor.fail() || v > 99 || v < 0)
    {
	return -1;
    }
    ver += v;
    ver *= 100;

    //
    // No need to get the patch number, we're only interested in
    // MAJOR.MINOR
    //
//     if(patchPos != string::npos)
//     {
// 	istringstream patch(version.substr(patchPos + 1));
// 	patch >> v;
// 	if(patch.fail() || v > 99 || v < 0)
// 	{
// 	    return -1;
// 	}
// 	ver += v;
//     }

    return ver;
}
