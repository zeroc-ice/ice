// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

Test::MyInterfacePrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "test:" + helper->getTestEndpoint();
    Test::MyInterfacePrx cl(communicator, ref);
    Test::MyDerivedInterfacePrx derived(communicator, ref);

    cout << "testing twoway operations... " << flush;
    void twoways(const Ice::CommunicatorPtr&, const Test::MyInterfacePrx&);
    twoways(communicator, cl);
    twoways(communicator, derived);
    derived->opDerived();
    cout << "ok" << endl;

    cout << "testing oneway operations... " << flush;
    void oneways(const Ice::CommunicatorPtr&, const Test::MyInterfacePrx&);
    oneways(communicator, cl);
    cout << "ok" << endl;

    cout << "testing twoway operations with AMI... " << flush;
    void twowaysAMI(const Ice::CommunicatorPtr&, const Test::MyInterfacePrx&);
    twowaysAMI(communicator, cl);
    twowaysAMI(communicator, derived);
    cout << "ok" << endl;

    cout << "testing oneway operations with AMI... " << flush;
    void onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyInterfacePrx&);
    onewaysAMI(communicator, cl);
    cout << "ok" << endl;

    cout << "testing batch oneway operations... " << flush;
    void batchOneways(const Test::MyInterfacePrx&);
    batchOneways(cl);
    batchOneways(derived);
    cout << "ok" << endl;

    cout << "testing batch AMI oneway operations... " << flush;
    void batchOnewaysAMI(const Test::MyInterfacePrx&);
    batchOnewaysAMI(cl);
    batchOnewaysAMI(derived);
    cout << "ok" << endl;

    return cl;
}
