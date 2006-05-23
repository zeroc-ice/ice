// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/FileUserAccountMapperI.h>

#include <fstream>

using namespace std;
using namespace IceGrid;

FileUserAccountMapperI::FileUserAccountMapperI(const string& filename)
{
    ifstream file(filename.c_str());
    if(!file)
    {
	throw "cannot open `" + filename + "' for reading: " + strerror(errno);
    }
	    
    while(true)
    {
	string user;
	file >> user;
	if(!file)
	{
	    break;
	}
		
	string account;
	file >> account;
	if(!file)
	{
	    break;
	}
		
	assert(!user.empty());
	assert(!account.empty());
	_accounts.insert(make_pair(user, account));
    }
}

string
FileUserAccountMapperI::getUserAccount(const string& user, const Ice::Current&)
{
    map<string, string>::const_iterator p = _accounts.find(user);
    if(p == _accounts.end())
    {
	throw UserAccountNotFoundException();
    }
    return p->second;
}
