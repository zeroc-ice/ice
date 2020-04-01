//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Key.h>
#include <Clash.h>

using namespace std;

class breakI : public _cpp_and::_cpp_break
{
public:

    virtual void caseAsync(::Ice::Int,
                           function<void(int)> response,
                           function<void(exception_ptr)>,
                           const ::Ice::Current&)
    {
        response(0);
    }
};

class charI: public _cpp_and::_cpp_char
{
public:

#ifndef NDEBUG
    virtual void _cpp_explicit(const ::Ice::Current& current)
#else
    virtual void _cpp_explicit(const ::Ice::Current&)
#endif
    {
        assert(current.operation == "explicit");
    }
};

class switchI: public _cpp_and::_cpp_switch
{
public:
    virtual void foo(shared_ptr<_cpp_and::charPrx>, Ice::Int&, const ::Ice::Current&)
    {
    }
};

class doI : public _cpp_and::_cpp_do
{
public:
    virtual void caseAsync(int,
                           ::std::function<void(int)>,
                           ::std::function<void(::std::exception_ptr)>,
                           const ::Ice::Current&)
    {
    }
    virtual void _cpp_explicit(const ::Ice::Current&)
    {
    }
    virtual void foo(const _cpp_and::charPrx&, Ice::Int&, const ::Ice::Current&)
    {
    }
};

//
// This section of the test is present to ensure that the C++ types
// are named correctly. It is not expected to run.
//
void
testtypes()
{
    _cpp_and::_cpp_continue a = _cpp_and::_cpp_continue::_cpp_asm;
    test(a == _cpp_and::_cpp_continue::_cpp_asm);

    _cpp_and::_cpp_auto b, b2;
    b._cpp_default = 0;
    b2._cpp_default = b._cpp_default;
    b._cpp_default = b2._cpp_default;

    _cpp_and::_cpp_delete c;
    c._cpp_else = "";

    _cpp_and::breakPrxPtr d;
    int d2;
    d->_cpp_case(0, d2);
    _cpp_and::breakPtr d1 = std::make_shared<breakI>();

    _cpp_and::charPrxPtr e;
    e->_cpp_explicit();
    _cpp_and::charPtr e1 = std::make_shared<charI>();

    _cpp_and::switchPtr f1 = std::make_shared<switchI>();

    _cpp_and::doPrxPtr g;
    g->_cpp_case(0, d2);
    g->_cpp_explicit();
    _cpp_and::doPtr g1 = std::make_shared<doI>();

    _cpp_and::_cpp_extern h;
    _cpp_and::_cpp_for i;
    _cpp_and::_cpp_return j;
    j._cpp_signed = 0;
    _cpp_and::_cpp_sizeof k;
    k._cpp_static = 0;
    k._cpp_switch = 1;
    k._cpp_signed = 2;

    const int m  = _cpp_and::_cpp_template;
    test(m == _cpp_and::_cpp_template);

    test(_cpp_and::_cpp_xor_eq == 0);
}

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<charI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    cout << "Testing operation name... " << flush;
    _cpp_and::charPrxPtr p = ICE_UNCHECKED_CAST(_cpp_and::charPrx,
                                                adapter->createProxy(Ice::stringToIdentity("test")));
    p->_cpp_explicit();
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
