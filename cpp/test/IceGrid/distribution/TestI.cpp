// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

#include <fstream>

using namespace std;

TestI::TestI(const Ice::PropertiesPtr& properties) :
    _properties(properties)
{
}

string
TestI::getServerFile(const string& path, const Ice::Current&)
{
    string file = _properties->getProperty("ServerDistrib") + "/" + path;
    ifstream is(file.c_str());
    string content;
    if(is.good())
    {
        is >> content;
    }
    return content;
}

string
TestI::getApplicationFile(const string& path, const Ice::Current&)
{
    string file = _properties->getProperty("ApplicationDistrib") + "/" + path;
    ifstream is(file.c_str());
    string content;
    if(is.good())
    {
        is >> content;
    }
    return content;
}
