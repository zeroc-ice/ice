// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

#ifdef WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref;
    
    Ice::PropertiesPtr properties = communicator->getProperties();

    string address = properties->getProperty("Ice.Address");
    string protocol = properties->getProperty("Ice.Protocol");
    string secure;

    if (protocol.empty())
    {
        protocol = "tcp";
    }

    if (protocol.compare("ssl") == 0)
    {
        secure = " -s ";
    }

    string endpts = protocol + " -p 12345 -t 2000";

    if (!address.empty())
    {
        endpts += " -h " + address;
    }

    ref = "test" + secure + ":" + endpts;

    cout << "ref: " << ref << endl;

    timeval tv1;
#ifdef WIN32
    struct _timeb tb1;
    _ftime(&tb1);
    tv1.tv_sec = tb1.time;
    tv1.tv_usec = tb1.millitm * 1000;
#else
    gettimeofday(&tv1, 0);
#endif

    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    cout << "ok" << endl;

    cout << "testing twoway operations... " << flush;
    void twoways(const Test::MyClassPrx&);
    twoways(cl);
    twoways(derived);
    derived->opDerived();
    cout << "ok" << endl;

    timeval tv2;
#ifdef WIN32
    struct _timeb tb2;
    _ftime(&tb2);
    tv2.tv_sec = tb2.time;
    tv2.tv_usec = tb2.millitm * 1000;
#else
    gettimeofday(&tv2, 0);
#endif

    timeval tv;

    tv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    tv.tv_usec = tv2.tv_usec - tv1.tv_usec;

    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec = tv.tv_usec % 1000000;

    if (tv.tv_usec < 0)
    {
        tv.tv_usec += 1000000;
        tv.tv_sec -= 1;
    }

    double total = (tv.tv_sec * 1000000.0 + tv.tv_usec) / 1000.0;

    cout << "elapsed time " << total  << "ms" << endl;

    return cl;
}
