// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

Test::ChecksumPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);

    Test::ChecksumPrxPtr checksum = ICE_CHECKED_CAST(Test::ChecksumPrx, base);
    test(checksum);

    Ice::SliceChecksumDict::const_iterator p;

    //
    // Verify that no checksums are present for local types.
    //
    cout << "testing checksums... " << flush;
    Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
    test(!localChecksums.empty());
    for(p = localChecksums.begin(); p != localChecksums.end(); ++p)
    {
        string::size_type pos = p->first.find("Local");
        test(pos == string::npos);
    }

    //
    // Get server's Slice checksums.
    //
    Ice::SliceChecksumDict d = checksum->getSliceChecksums();

    //
    // Compare the checksums. For a type FooN whose name ends in an integer N,
    // we assume that the server's type does not change for N = 1, and does
    // change for N > 1.
    //
    for(p = d.begin(); p != d.end(); ++p)
    {
        int n = 0;
        string::size_type pos = p->first.find_first_of("0123456789");
        if(pos != string::npos)
        {
            n = atoi(p->first.c_str() + pos);
        }

        Ice::SliceChecksumDict::const_iterator q = localChecksums.find(p->first);
        test(q != localChecksums.end());

        if(n <= 1)
        {
            test(q->second == p->second);
        }
        else
        {
            test(q->second != p->second);
        }
    }
    cout << "ok" << endl;

    return checksum;
}
