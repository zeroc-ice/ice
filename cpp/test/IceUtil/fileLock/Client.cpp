// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/FileUtil.h>
#include <IceUtil/Exception.h>
#include <TestCommon.h>

using namespace IceUtil;
using namespace std;

int
main(int, char**)
{
    {
        IceUtilInternal::FileLockPtr lock;
        try
        {
            lock = new IceUtilInternal::FileLock("file.lock");
            test(true);
        }
        catch(const exception& ex)
        {
            cerr << "exception:\n" << ex.what() << endl;
            test(false);
        }
        catch(...)
        {
            test(false);
        }

        cout << "File lock acquired.\n"
             << "Enter some input and press enter, to release the lock and terminate the program." << endl;
        //
        // Block the test waiting for IO, so the file lock is preserved.
        //
        string dummy;
        cin >> dummy;
    }
    cout << "File lock released." << endl;
    return EXIT_SUCCESS;
}
