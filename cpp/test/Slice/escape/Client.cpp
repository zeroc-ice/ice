// Copyright (c) ZeroC, Inc.

#include "Clash.h"
#include "Ice/Ice.h"
#include "Key.h"
#include "TestHelper.h"

using namespace std;

class breakI : public cpp_and::cpp_break
{
public:
    void
    cpp_caseAsync(::int32_t, function<void(int)> response, function<void(exception_ptr)>, const Ice::Current&) override
    {
        response(0);
    }

    void cpp_explicit([[maybe_unused]] const Ice::Current& current) override
    {
        assert(current.operation == "explicit");
    }
};

class switchI : public cpp_and::cpp_switch
{
};

class doI : public cpp_and::cpp_do
{
public:
    void
    cpp_caseAsync(int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&) override
    {
    }

    void cpp_explicit(const Ice::Current&) override {}
};

class friendI : public cpp_and::cpp_friend
{
public:
    cpp_and::cpp_auto cpp_goto(
        cpp_and::cpp_continue,
        cpp_and::cpp_auto,
        cpp_and::cpp_switchPtr,
        optional<cpp_and::cpp_doPrx>,
        optional<cpp_and::cpp_breakPrx>,
        cpp_and::cpp_switchPtr,
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
    cpp_and::cpp_continue a = cpp_and::cpp_continue::cpp_asm;
    test(a == cpp_and::cpp_continue::cpp_asm);

    cpp_and::cpp_auto b, b2;
    b.cpp_default = 0;
    b2.cpp_default = b.cpp_default;
    b.cpp_default = b2.cpp_default;

    cpp_and::cpp_breakPrx d(communicator, "hello:tcp -h 127.0.0.1 -p 12010");
    int d2;
    d->cpp_case(0, d2);
    d->cpp_explicit();
    cpp_and::cpp_breakPtr d1 = std::make_shared<breakI>();

    cpp_and::cpp_switchPtr f1 = std::make_shared<switchI>();

    optional<cpp_and::cpp_doPrx> g;

// Work-around for:
// error: array subscript -6 is outside array bounds of ‘int (* [1152921504606846975])(...)’ [-Werror=array-bounds]
#if defined(NDEBUG) && defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    g->cpp_case(0, d2);
    g->cpp_explicit();
#if defined(NDEBUG) && defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    cpp_and::cpp_doPtr g1 = std::make_shared<doI>();

    cpp_and::cpp_extern h;
    cpp_and::cpp_for i;
    cpp_and::cpp_return j;
    j.cpp_signed = 0;
    cpp_and::cpp_sizeof k;
    k.cpp_static = 0;
    k.cpp_switch = 1;
    k.cpp_signed = 2;

    cpp_and::cpp_friendPtr l = std::make_shared<friendI>();

    const int m = cpp_and::cpp_template;
    test(m == cpp_and::cpp_template);
    test(cpp_and::cpp_template == 0);
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
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<breakI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    cout << "Testing operation name... " << flush;
    auto p = adapter->createProxy<cpp_and::cpp_breakPrx>(Ice::stringToIdentity("test"));
    p->cpp_explicit();
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
