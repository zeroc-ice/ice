// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <EchoI.h>

using namespace std;

string
EchoI::echoString(const std::string& uber, const Ice::Current&) const
{
    cout << "Received (UTF-8): \"" << IceUtil::escapeString(uber, "") << '\"' << endl;
    return "Bonne journ\303\251e";
}

void
EchoI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
