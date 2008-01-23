// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <GreetI.h>

using namespace std;

string decodeString(const string& str)
{
    ostringstream result;
    for(string::const_iterator p = str.begin(); p != str.end(); ++p)
    {
        if(isprint(*p))
        {
            result << *p;
        }
        else
        {
            result << "\\"
                   << oct << (unsigned int)(unsigned char)(*p);
        }
    }
    return result.str();
}


string
GreetI::exchangeGreeting(const std::string& msg, const Ice::Current&) const
{
    cout << "Received (UTF-8): \"" << decodeString(msg) << '\"' << endl;
    return "Bonne journ\303\251e";
}

void
GreetI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
