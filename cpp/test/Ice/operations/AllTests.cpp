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
    void twoways(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    twoways(communicator, cl);
    twoways(communicator, derived);
    derived->opDerived();
    cout << "ok" << endl;

    cout << "testing oneway operations... " << flush;
    void oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    oneways(communicator, cl);
    cout << "ok" << endl;

    cout << "testing twoway operations with AMI... " << flush;
    void twowaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    twowaysAMI(communicator, cl);
    twowaysAMI(communicator, derived);
    cout << "ok" << endl;

    cout << "testing oneway operations with AMI... " << flush;
    void onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    onewaysAMI(communicator, cl);
    cout << "ok" << endl;

    cout << "testing batch oneway operations... " << flush;
    void batchOneways(const Test::MyClassPrx&);
    batchOneways(cl);
    batchOneways(derived);
    cout << "ok" << endl;

    cout << "testing batch AMI oneway operations... " << flush;
    void batchOnewaysAMI(const Test::MyClassPrx&);
    batchOnewaysAMI(cl);
    batchOnewaysAMI(derived);
    cout << "ok" << endl;

    return cl;
}
