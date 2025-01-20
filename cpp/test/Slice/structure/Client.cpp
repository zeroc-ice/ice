// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

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
    def_s2.ss.emplace_back("one");
    def_s2.ss.emplace_back("two");
    def_s2.ss.emplace_back("three");
    def_s2.il.push_back(1);
    def_s2.il.push_back(2);
    def_s2.il.push_back(3);
    def_s2.sd["abc"] = "def";
    def_s2.s = {"name"};
    def_s2.cls = make_shared<C>(5);
    def_s2.prx = communicator->stringToProxy("test");
    cout << "ok" << endl;
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv);
    allTests(ich.communicator());
}

DEFINE_TEST(Client)
