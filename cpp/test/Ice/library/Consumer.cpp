// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("gencode"))
#endif

using namespace std;

ICE_DECLSPEC_EXPORT void
consume(const Ice::ObjectPtr& o, const Ice::ObjectPrx& p)
{
    cout << "testing dynamic cast across libraries... " << flush;

    //
    // Make sure dynamic cast works as expected
    // and exception raised by a shared library can be caught by another
    // shared library
    //

    Test::MyInterfacePtr servant = dynamic_pointer_cast<Test::MyInterface>(o);
    test(servant);

    auto proxy = dynamic_cast<const Test::MyInterfacePrx*>(&p);
    test(proxy);

    proxy->op(false);
#include "Ice/PushDisableWarnings.h"
    // emptyCurrent is deprecated
    servant->op(false, Ice::emptyCurrent);
#include "Ice/PopDisableWarnings.h"

    cout << "ok" << endl;

    cout << "testing exceptions thrown across libraries... " << flush;

    try
    {
        proxy->op(true);
    }
    catch (const Test::UserError&)
    {
        // expected
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        servant->op(true, Ice::Current{});
    }
    catch (const Test::UserError&)
    {
        // expected
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;
}
