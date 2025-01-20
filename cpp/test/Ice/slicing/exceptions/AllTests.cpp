// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

TestIntfPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx testPrx(communicator, "Test:" + helper->getTestEndpoint());

    cout << "base... " << flush;
    {
        try
        {
            testPrx->baseAsBase();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "Base.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base (AMI)... " << flush;
    {
        auto result = testPrx->baseAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "Base.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown derived... " << flush;
    {
        try
        {
            testPrx->unknownDerivedAsBase();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownDerived.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown derived (AMI)... " << flush;
    {
        auto result = testPrx->unknownDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownDerived.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as base... " << flush;
    {
        try
        {
            testPrx->knownDerivedAsBase();
            test(false);
        }
        catch (const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(string{k.ice_id()} == "::Test::KnownDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as base (AMI)... " << flush;
    {
        auto result = testPrx->knownDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(string{k.ice_id()} == "::Test::KnownDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as derived... " << flush;
    {
        try
        {
            testPrx->knownDerivedAsKnownDerived();
            test(false);
        }
        catch (const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(string{k.ice_id()} == "::Test::KnownDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as derived (AMI)... " << flush;
    {
        auto result = testPrx->knownDerivedAsKnownDerivedAsync();
        try
        {
            result.get();
        }
        catch (const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(string{k.ice_id()} == "::Test::KnownDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown intermediate as base... " << flush;
    {
        try
        {
            testPrx->unknownIntermediateAsBase();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownIntermediate.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown intermediate as base (AMI)... " << flush;
    {
        auto result = testPrx->unknownIntermediateAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownIntermediate.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known intermediate as base... " << flush;
    {
        try
        {
            testPrx->knownIntermediateAsBase();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known intermediate as base (AMI)... " << flush;
    {
        auto result = testPrx->knownIntermediateAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known most derived as base... " << flush;
    {
        try
        {
            testPrx->knownMostDerivedAsBase();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known most derived as base (AMI)... " << flush;
    {
        auto result = testPrx->knownMostDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known intermediate as intermediate... " << flush;
    {
        try
        {
            testPrx->knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known intermediate as intermediate (AMI)... " << flush;
    {
        auto result = testPrx->knownIntermediateAsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived exception as intermediate... " << flush;
    {
        try
        {
            testPrx->knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as intermediate (AMI)... " << flush;
    {
        auto result = testPrx->knownMostDerivedAsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as most derived... " << flush;
    {
        try
        {
            testPrx->knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as most derived (AMI)... " << flush;
    {
        auto result = testPrx->knownMostDerivedAsKnownMostDerivedAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(string{kmd.ice_id()} == "::Test::KnownMostDerived");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as base... " << flush;
    {
        try
        {
            testPrx->unknownMostDerived1AsBase();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as base (AMI)... " << flush;
    {
        auto result = testPrx->unknownMostDerived1AsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as intermediate... " << flush;
    {
        try
        {
            testPrx->unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as intermediate (AMI)... " << flush;
    {
        auto result = testPrx->unknownMostDerived1AsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(string{ki.ice_id()} == "::Test::KnownIntermediate");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, unknown intermediate as base... " << flush;
    {
        try
        {
            testPrx->unknownMostDerived2AsBase();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownMostDerived2.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, unknown intermediate as base (AMI)... " << flush;
    {
        auto result = testPrx->unknownMostDerived2AsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch (const Base& b)
        {
            test(b.b == "UnknownMostDerived2.b");
            test(string{b.ice_id()} == "::Test::Base");
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    return testPrx;
}
