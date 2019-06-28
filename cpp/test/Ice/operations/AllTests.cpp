//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

Test::MyClassPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    Test::MyClassPrxPtr cl = ICE_CHECKED_CAST(Test::MyClassPrx, base);
    Test::MyDerivedClassPrxPtr derived = ICE_CHECKED_CAST(Test::MyDerivedClassPrx, cl);

    cout << "testing twoway operations... " << flush;
    void twoways(const Ice::CommunicatorPtr&, Test::TestHelper*, const Test::MyClassPrxPtr&);
    twoways(communicator, helper, cl);
    twoways(communicator, helper, derived);
    derived->opDerived();
    cout << "ok" << endl;

    cout << "testing oneway operations... " << flush;
    void oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr&);
    oneways(communicator, cl);
    cout << "ok" << endl;

    cout << "testing twoway operations with AMI... " << flush;
    void twowaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr&);
    twowaysAMI(communicator, cl);
    twowaysAMI(communicator, derived);
    cout << "ok" << endl;

    cout << "testing oneway operations with AMI... " << flush;
    void onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr&);
    onewaysAMI(communicator, cl);
    cout << "ok" << endl;

    cout << "testing batch oneway operations... " << flush;
    void batchOneways(const Test::MyClassPrxPtr&);
    batchOneways(cl);
    batchOneways(derived);
    cout << "ok" << endl;

    cout << "testing batch AMI oneway operations... " << flush;
    void batchOnewaysAMI(const Test::MyClassPrxPtr&);
    batchOnewaysAMI(cl);
    batchOnewaysAMI(derived);
    cout << "ok" << endl;

    return cl;
}
