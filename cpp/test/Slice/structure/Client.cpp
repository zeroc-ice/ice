// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing equals() for Slice structures... " << flush;

    //
    // Define some default values.
    //
    S2 def_s2;
    def_s2.bo = true;
    def_s2.by = 98;
    def_s2.sh = 99;
    def_s2.i = 100;
    def_s2.l = 101;
    def_s2.f = 1.0;
    def_s2.d = 2.0;
    def_s2.str = "string";
    def_s2.ss.push_back("one");
    def_s2.ss.push_back("two");
    def_s2.ss.push_back("three");
    def_s2.il.push_back(1);
    def_s2.il.push_back(2);
    def_s2.il.push_back(3);
    def_s2.sd["abc"] = "def";
#ifdef ICE_CPP11_MAPPING
    def_s2.s = {"name"};
#else
    def_s2.s = new S1("name");
#endif
    def_s2.cls = ICE_MAKE_SHARED(C, 5);
    def_s2.prx = communicator->stringToProxy("test");

#ifndef ICE_CPP11_MAPPING
    //
    // cpp:comparable required by tests bellow is only
    // supported with C++98 mapping.
    //

    //
    // Change one primitive member at a time.
    //
    {
        S2 v;

        v = def_s2;
        test(v == def_s2);

        v = def_s2;
        v.bo = false;
        test(v != def_s2);

        v = def_s2;
        v.by--;
        test(v != def_s2);

        v = def_s2;
        v.sh--;
        test(v != def_s2);

        v = def_s2;
        v.i--;
        test(v != def_s2);

        v = def_s2;
        v.l--;
        test(v != def_s2);

        v = def_s2;
        v.f--;
        test(v != def_s2);

        v = def_s2;
        v.d--;
        test(v != def_s2);

        v = def_s2;
        v.str = "";
        test(v != def_s2);
    }

    //
    // String member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.str = "string";
        test(v1 == def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.str.clear();
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.str.clear();
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v1.str.clear();
        v2.str.clear();
        test(v1 == v2);
    }

    //
    // Sequence member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.ss = def_s2.ss;
        test(v1 == def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.ss.clear();
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.ss.clear();
        test(v1 != v2);
    }

    //
    // Custom sequence member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.il = def_s2.il;
        test(v1 == def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.il.clear();
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.il.clear();
        test(v1 != v2);
    }

    //
    // Dictionary member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.sd = def_s2.sd;
        test(v1 == def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.sd.clear();
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.sd.clear();
        test(v1 != v2);
    }

    //
    // Struct member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.s = new S1(def_s2.s->name);
        test(v1 == def_s2);

        v1 = def_s2;
        v1.s = new S1("name");
        test(v1 == def_s2);

        v1 = def_s2;
        v1.s = new S1("noname");
        test(v1 != def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.s = 0;
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.s = 0;
        test(v1 != v2);
    }

    //
    // Class member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.cls = new C(def_s2.cls->i);
        test(v1 != def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.cls = 0;
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.cls = 0;
        test(v1 != v2);
    }

    //
    // Proxy member
    //
    {
        S2 v1, v2;

        v1 = def_s2;
        v1.prx = communicator->stringToProxy("test");
        test(v1 == def_s2);

        v1 = def_s2;
        v1.prx = communicator->stringToProxy("test2");
        test(v1 != def_s2);

        v1 = def_s2;
        v2 = def_s2;
        v1.prx = 0;
        test(v1 != v2);

        v1 = def_s2;
        v2 = def_s2;
        v2.prx = 0;
        test(v1 != v2);
    }
#endif
    cout << "ok" << endl;
}

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv);
    allTests(ich.communicator());
}

DEFINE_TEST(Client)
