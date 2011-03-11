// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace std;

//
// We use different files for Win32 and Unix version of this test
// because they need to use different encoding.
//
// This file is encoded in UTF-8 with BOM, VC++ need to use BOM to 
// guest the right encoding for wstring literals.
//
int
main(int argc, char* argv[])
{
    cout << "testing load properties from UTF-8 path... " << flush;
    Ice::InitializationData id;
    id.properties = Ice::createProperties();
    id.properties->load(IceUtil::wstringToString(L".\\中国\\client.config"));
    test(id.properties->getProperty("Ice.Trace.Network") == "1");
    test(id.properties->getProperty("Ice.Trace.Protocol") == "1");
    cout << "ok" << flush;
    return EXIT_SUCCESS;
}
