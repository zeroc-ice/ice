// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

Test::MyClassPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "test:" + helper->getTestEndpoint();
    Test::MyClassPrx cl(communicator, ref);
    Test::MyDerivedClassPrx derived(communicator, ref);

    cout << "testing twoway operations... " << flush;
    void twoways(const Ice::CommunicatorPtr&, Test::TestHelper*, const Test::MyClassPrx&);
    twoways(communicator, helper, cl);
    twoways(communicator, helper, derived);
    derived->opDerived();
    cout << "ok" << endl;

    cout << "testing oneway operations... " << flush;
    void oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    oneways(communicator, cl);
    cout << "ok" << endl;

    cout << "testing twoway operations with AMI... " << flush;
    void twowaysAMI(const Ice::CommunicatorPtr&, Test::TestHelper*, const Test::MyClassPrx&);
    twowaysAMI(communicator, helper, cl);
    twowaysAMI(communicator, helper, derived);
    cout << "ok" << endl;

    cout << "testing oneway operations with AMI... " << flush;
    void onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    onewaysAMI(communicator, cl);
    cout << "ok" << endl;

    cout << "testing batch oneway operations... " << flush;
    void batchOneways(const Test::MyClassPrx&, Test::TestHelper*);
    batchOneways(cl, helper);
    batchOneways(derived, helper);
    cout << "ok" << endl;

    cout << "testing batch AMI oneway operations... " << flush;
    void batchOnewaysAMI(const Test::MyClassPrx&, Test::TestHelper*);
    batchOnewaysAMI(cl, helper);
    batchOnewaysAMI(derived, helper);
    cout << "ok" << endl;

    return cl;
}
