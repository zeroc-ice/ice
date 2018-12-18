// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr com = helper->communicator();
    TestIntfPrxPtr service1 = ICE_UNCHECKED_CAST(TestIntfPrx, com->stringToProxy("test:" + helper->getTestEndpoint(0)));
    TestIntfPrxPtr service2 = ICE_UNCHECKED_CAST(TestIntfPrx, com->stringToProxy("test:" + helper->getTestEndpoint(1)));
    TestIntfPrxPtr service3 = ICE_UNCHECKED_CAST(TestIntfPrx, com->stringToProxy("test:" + helper->getTestEndpoint(2)));
    TestIntfPrxPtr service4 = ICE_UNCHECKED_CAST(TestIntfPrx, com->stringToProxy("test:" + helper->getTestEndpoint(3)));

    if(service1->getProperty("IceBox.InheritProperties") == "")
    {
        cout << "testing service properties... " << flush;

        test(service1->getProperty("Ice.ProgramName") == "IceBox-Service1");
        test(service1->getProperty("Service") == "1");
        test(service1->getProperty("Service1.Ovrd") == "2");
        test(service1->getProperty("Service1.Unset") == "");
        test(service1->getProperty("Arg") == "1");

        Ice::StringSeq args1;
        args1.push_back("-a");
        args1.push_back("--Arg=2");
        test(service1->getArgs() == args1);

        test(service2->getProperty("Ice.ProgramName") == "Test");
        test(service2->getProperty("Service") == "2");
        test(service2->getProperty("Service1.ArgProp") == "");
        test(service2->getProperty("IceBox.InheritProperties") == "1");

        Ice::StringSeq args2;
        args2.push_back("--Service1.ArgProp=1");
        test(service2->getArgs() == args2);

        cout << "ok" << endl;

        cout << "testing with shared communicator... " << flush;

        test(service3->getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
        test(service3->getProperty("Service") == "4");
        test(service3->getProperty("Prop") == "");
        test(service3->getProperty("Service3.Prop") == "1");
        test(service3->getProperty("Ice.Trace.Slicing") == "3");

        test(service4->getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
        test(service4->getProperty("Service") == "4");
        test(service4->getProperty("Prop") == "");
        test(service4->getProperty("Service3.Prop") == "1");
        test(service4->getProperty("Ice.Trace.Slicing") == "3");

        Ice::StringSeq args4;
        args4.push_back("--Service3.Prop=2");
        test(service4->getArgs() == args4);

        cout << "ok" << endl;
    }
    else
    {
        cout << "testing property inheritance... " << flush;

        test(service1->getProperty("Ice.ProgramName") == "IceBox2-Service1");
        test(service1->getProperty("ServerProp") == "1");
        test(service1->getProperty("OverrideMe") == "2");
        test(service1->getProperty("UnsetMe") == "");
        test(service1->getProperty("Service1.Prop") == "1");
        test(service1->getProperty("Service1.ArgProp") == "2");

        test(service2->getProperty("Ice.ProgramName") == "IceBox2-SharedCommunicator");
        test(service2->getProperty("ServerProp") == "1");
        test(service2->getProperty("OverrideMe") == "3");
        test(service2->getProperty("UnsetMe") == "");
        test(service2->getProperty("Service2.Prop") == "1");

        cout << "ok" << endl;
    }
}
