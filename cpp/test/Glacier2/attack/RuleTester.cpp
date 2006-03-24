// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// From src/Glacier2
//
#include <ProxyVerifier.h>

#include <iostream>
#include <memory>
using namespace std;

bool 
startsWith(const string& s, const char* prefix)
{
    if(strncmp(s.c_str(), prefix, strlen(prefix)) == 0)
    {
	return true;
    }
    return false;
}

void 
dump(const Ice::PropertiesPtr& p)
{
    cerr << "Reject Rule: " <<  p->getPropertyWithDefault("Glacier2.Client.AddProxy.Reject", "(unset)") << '\n';
    cerr << "Accept Rule: " <<  p->getPropertyWithDefault("Glacier2.Client.AddProxy.Accept", "(unset)") << '\n';
    cerr << "MaxEndpoints Rule: " <<  p->getPropertyWithDefault("Glacier2.Client.AddProxy.MaxEndpoints", "(unset)") 
	 << '\n';
}

void
parseLine(const string& line, string& prefix, string& data)
{
    istringstream is(line);

    if(!(is >> prefix) || is.eof())
    {
	string msg = "Test definition error: malformed test case: ";
	msg += line;
	throw msg;
    }

    while((is.peek() == ' ') && !is.eof())
    {
	is.get();
    }

    if(is.eof())
    {
	string msg = "Test definition error: malformed test case: ";
	msg += line;
	throw msg;
    }

    getline(is, data);
}

//
// Test driver is meant to be driven from a script.
//
int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator = Ice::initialize(argc, argv);
    Ice::PropertiesPtr props = communicator->getProperties();
    auto_ptr<Glacier2::ProxyVerifier> v; 
    string input;
    while(true)
    {
	getline(cin, input);
	if(cin.eof())
	{
	    cerr << "input terminated" << endl;
	    break;
	}
	if(input == "EOF")
	{
	    break;
	}
	else if(input[0] == '#')
	{
	    continue;
	}
	else if(startsWith(input, "nr "))
	{
	    string prefix;
	    string data;
	    parseLine(input, prefix, data);
	    if(startsWith(data, "unset"))
	    {
		data = "";
	    }
	    props->setProperty("Glacier2.Client.AddProxy.Reject", data);
	}
	else if(startsWith(input, "na"))
	{
	    string prefix;
	    string data;
	    parseLine(input, prefix, data);
	    if(startsWith(data, "unset"))
	    {
		data = "";
	    }
	    props->setProperty("Glacier2.Client.AddProxy.Accept", data);
	}
	else if(startsWith(input, "nm"))
	{
	    string prefix;
	    string data;
	    parseLine(input, prefix, data);
	    if(startsWith(data, "unset"))
	    {
		data = "";
	    }
	    props->setProperty("Glacier2.Client.AddProxy.MaxEndpoints", data);
	}
	else if(input == "init")
	{
	    v.reset(new Glacier2::ProxyVerifier(communicator, ""));
	    dump(props);
	}
	else
	{
	    if(v.get() == 0)
	    {
		cerr << "Test definition error: 'init' must appear before test cases." << endl;
		return 1;
	    }

	    //
	    // Test case
	    //
	    // Format is [expected result] [proxy] where [expected result]
	    // is 'r' for reject and 'a' for accept. Proxy strings must all
	    // be valid proxy strings.
	    //
	    string prefix;
	    string data;
	    try
	    {
		parseLine(input, prefix, data);
	    }
	    catch(const string& ex)
	    {
		cerr << ex << endl;
		cout << "error" << endl;
		throw;
	    }
	    bool accept;
	    if(prefix == "r")
	    {
		accept = false;
	    }
	    else if(prefix == "a")
	    {
		accept = true;
	    }
	    else
	    {
		cerr << "Test definition error: test case line must being with an 'r' or 'a' followed by a space." 
		     << endl;
		cout << "error" << endl;
		return 1;
	    }
	    Ice::ObjectPrx p = communicator->stringToProxy(data);
	    bool result = v->verify(p);
	    if(accept && !result)
	    {
		cerr << "Test failed: " << data << " should've been accepted." << endl;
		dump(props);
		cout << "error" << endl;
		return 1;
	    }
	    else if(!accept && result)
	    {
		cerr << "Test failed: " << data << " should've been rejected." << endl;
		dump(props);
		cout << "error" << endl;
		return 1;
	    }
	    else
	    {
		cerr << "Test succeeded: " << data << " was " << (accept ? "accepted" : "rejected") << endl;
	    }
	}
    }
    cout << "ok" << endl;
    return 0;
}
