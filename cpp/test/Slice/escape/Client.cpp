// Copyright (c) ZeroC, Inc.

#include "Clash.h"
#include "Ice/Ice.h"
#include "Key.h"
#include "TestHelper.h"

using namespace std;

class breakI : public escaped_and::escaped_break
{
public:
    void escaped_caseAsync(::int32_t, function<void(int)> response, function<void(exception_ptr)>, const Ice::Current&)
        override
    {
        response(0);
    }

    void escaped_explicit([[maybe_unused]] const Ice::Current& current) override
    {
        assert(current.operation == "explicit");
    }
};

class switchI : public escaped_and::escaped_switch
{
};

class doI : public escaped_and::escaped_do
{
public:
    void escaped_caseAsync(int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override
    {
    }

    void escaped_explicit(const Ice::Current&) override {}
};

class friendI : public escaped_and::escaped_friend
{
public:
    escaped_and::escaped_auto escaped_goto(
        escaped_and::escaped_continue,
        escaped_and::escaped_auto,
        escaped_and::escaped_switchPtr,
        optional<escaped_and::escaped_doPrx>,
        optional<escaped_and::escaped_breakPrx>,
        escaped_and::escaped_switchPtr,
        ::int32_t,
        const Ice::Current&) override
    {
        return {};
    }
};

//
// This section of the test is present to ensure that the C++ types
// are named correctly. It is not expected to run.
//
void
testtypes(const Ice::CommunicatorPtr& communicator)
{
    escaped_and::escaped_continue a = escaped_and::escaped_continue::escaped_asm;
    test(a == escaped_and::escaped_continue::escaped_asm);

    escaped_and::escaped_auto b, b2;
    b.escaped_default = 0;
    b2.escaped_default = b.escaped_default;
    b.escaped_default = b2.escaped_default;

    escaped_and::escaped_breakPrx d(communicator, "hello:tcp -h 127.0.0.1 -p 12010");
    int d2;
    d->escaped_case(0, d2);
    d->escaped_explicit();
    escaped_and::escaped_breakPtr d1 = std::make_shared<breakI>();

    escaped_and::escaped_switchPtr f1 = std::make_shared<switchI>();

    optional<escaped_and::escaped_doPrx> g;

// Work-around for:
// error: array subscript -6 is outside array bounds of ‘int (* [1152921504606846975])(...)’ [-Werror=array-bounds]
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    g->escaped_case(0, d2);
    g->escaped_explicit();
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    escaped_and::escaped_doPtr g1 = std::make_shared<doI>();

    escaped_and::escaped_extern h;
    escaped_and::escaped_for i;
    escaped_and::escaped_return j;
    j.escaped_signed = 0;
    escaped_and::escaped_sizeof k;
    k.escaped_static = 0;
    k.escaped_switch = 1;
    k.escaped_signed = 2;

    escaped_and::escaped_friendPtr l = std::make_shared<friendI>();

    const int m = escaped_and::escaped_template;
    test(m == escaped_and::escaped_template);
    test(escaped_and::escaped_template == 0);
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<breakI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    cout << "Testing operation name... " << flush;
    auto p = adapter->createProxy<escaped_and::escaped_breakPrx>(Ice::stringToIdentity("test"));
    p->escaped_explicit();
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
