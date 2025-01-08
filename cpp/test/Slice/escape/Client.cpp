//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Clash.h"
#include "Ice/Ice.h"
#include "Key.h"
#include "TestHelper.h"

using namespace std;

class breakI : public cpp_and::_cpp_break
{
public:
    void
    _cpp_caseAsync(::int32_t, function<void(int)> response, function<void(exception_ptr)>, const ::Ice::Current&) override
    {
        response(0);
    }

#ifndef NDEBUG
    void _cpp_explicit(const ::Ice::Current& current) override
#else
    virtual void _cpp_explicit(const ::Ice::Current&) override
#endif
    {
        assert(current.operation == "explicit");
    }
};

class switchI : public cpp_and::_cpp_switch
{
public:
    virtual void foo(optional<cpp_and::_cpp_breakPrx>, int32_t&, const ::Ice::Current&) {}
};

class doI : public cpp_and::_cpp_do
{
public:
    void _cpp_caseAsync(int, ::std::function<void(int)>, ::std::function<void(::std::exception_ptr)>, const ::Ice::Current&)
        override
    {
    }

    void _cpp_explicit(const ::Ice::Current&) override {}

    virtual void foo(const cpp_and::_cpp_breakPrx&, int32_t&, const ::Ice::Current&) {}
};

class friendI : public cpp_and::_cpp_friend
{
public:
    cpp_and::_cpp_auto _cpp_goto(
        cpp_and::_cpp_continue,
        cpp_and::_cpp_auto,
        cpp_and::_cpp_switchPtr,
        optional<cpp_and::_cpp_doPrx>,
        optional<cpp_and::_cpp_breakPrx>,
        cpp_and::_cpp_switchPtr,
        ::int32_t,
        const ::Ice::Current&) override
    {
        return cpp_and::_cpp_auto();
    }
};

//
// This section of the test is present to ensure that the C++ types
// are named correctly. It is not expected to run.
//
void
testtypes(const Ice::CommunicatorPtr& communicator)
{
    cpp_and::_cpp_continue a = cpp_and::_cpp_continue::_cpp_asm;
    test(a == cpp_and::_cpp_continue::_cpp_asm);

    cpp_and::_cpp_auto b, b2;
    b._cpp_default = 0;
    b2._cpp_default = b._cpp_default;
    b._cpp_default = b2._cpp_default;

    cpp_and::_cpp_breakPrx d(communicator, "hello:tcp -h 127.0.0.1 -p 12010");
    int d2;
    d->_cpp_case(0, d2);
    d->_cpp_explicit();
    cpp_and::_cpp_breakPtr d1 = std::make_shared<breakI>();

    cpp_and::_cpp_switchPtr f1 = std::make_shared<switchI>();

    optional<cpp_and::_cpp_doPrx> g;

// Work-around for:
// error: array subscript -6 is outside array bounds of ‘int (* [1152921504606846975])(...)’ [-Werror=array-bounds]
#if defined(NDEBUG) && defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    g->_cpp_case(0, d2);
    g->_cpp_explicit();
#if defined(NDEBUG) && defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    cpp_and::_cpp_doPtr g1 = std::make_shared<doI>();

    cpp_and::_cpp_extern h;
    cpp_and::_cpp_for i;
    cpp_and::_cpp_return j;
    j._cpp_signed = 0;
    cpp_and::_cpp_sizeof k;
    k._cpp_static = 0;
    k._cpp_switch = 1;
    k._cpp_signed = 2;

    cpp_and::_cpp_friendPtr l = std::make_shared<friendI>();

    const int m = cpp_and::_cpp_template;
    test(m == cpp_and::_cpp_template);
    test(cpp_and::_cpp_template == 0);
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
    auto p = adapter->createProxy<cpp_and::_cpp_breakPrx>(Ice::stringToIdentity("test"));
    p->_cpp_explicit();
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
