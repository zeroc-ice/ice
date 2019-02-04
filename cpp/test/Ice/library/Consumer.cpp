//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Test.h>
#include <TestHelper.h>

#if defined(_MSC_VER)
#   pragma comment(lib, ICE_LIBNAME("gencode"))
#endif

using namespace std;

ICE_DECLSPEC_EXPORT void
consume(const Ice::ObjectPtr& o, const Ice::ObjectPrxPtr& p)
{
    cout << "testing dynamic cast across libraries... " << flush;

    //
    // Make sure dynamic cast works as expected
    // and exception raised by a shared library can be caught by another
    // shared library
    //

    Test::MyInterfacePtr servant = ICE_DYNAMIC_CAST(Test::MyInterface, o);
    test(servant);

#ifdef ICE_CPP11_MAPPING
    auto proxy = dynamic_pointer_cast<Test::MyInterfacePrx>(p);
#else
    Test::MyInterfacePrx proxy = dynamic_cast<IceProxy::Test::MyInterface*>(p.get());
#endif
    test(proxy);

    proxy->op(false);
    servant->op(false, Ice::emptyCurrent);

    cout << "ok" << endl;

    cout << "testing exceptions thrown across libraries... " << flush;

    try
    {
        proxy->op(true);
    }
    catch(const Test::UserError&)
    {
        // expected
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        servant->op(true, Ice::emptyCurrent);
    }
    catch(const Test::UserError&)
    {
        // expected
    }
    catch(...)
    {
        test(false);
    }

    cout << "ok" << endl;
}
