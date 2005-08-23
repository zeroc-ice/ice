// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

