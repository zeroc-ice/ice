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
    IceUtilInternal::FileLockPtr lock;
    try
    {
        lock = new IceUtilInternal::FileLock("file.lock");
        test(false);
    }
    catch(const IceUtil::FileLockException&)
    {
        cout << "File lock not acquired." << endl;
//
// Try to read the pid of the lock owner from
// the lock file.
//
// In windows we don't write pid to lock files.
//
#ifndef _WIN32
        IceUtilInternal::ifstream is(string("file.lock"));
        if(!is.good())
        {
            test(false);
        }
        string pid;
        is >> pid;
        test(pid.size() != 0);
        cout << "Lock owned by: " << pid << endl;
#endif
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
    return EXIT_SUCCESS;
}
