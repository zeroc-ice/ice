// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace Test;
using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int, char**)
{
    cout << "testing Slice predefined macros... " << flush;
    DefaultPtr d = std::make_shared<Default>();
    test(d->x == 10);
    test(d->y == 10);

    CppOnlyPtr c = std::make_shared<CppOnly>();
    test(c->lang == "cpp");
    test(c->version == ICE_INT_VERSION);
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
