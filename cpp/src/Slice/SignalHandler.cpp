// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/SignalHandler.h>
#include <IceUtil/StaticMutex.h>
#include <map>

using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace std;

static StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;

static SignalHandlerCloseCallback _callback = 0;
static vector<string> _fileList;

void
SignalHandler::addFileForCleanup(const string& file)
{
    StaticMutex::Lock lock(_mutex);

    _fileList.push_back(file);
}

void
SignalHandler::setCloseCallback(Slice::SignalHandlerCloseCallback callback)
{
    StaticMutex::Lock lock(_mutex);

    _callback = callback;
}

void
SignalHandler::clearCleanupFileList()
{
    StaticMutex::Lock lock(_mutex);

    _fileList.clear();
    _callback = 0;
}

void
SignalHandler::removeFilesOnInterrupt(int signal)
{
    StaticMutex::Lock lock(_mutex);

    if(_callback != 0)
    {
        _callback();
    }

    for(unsigned int i = 0; i < _fileList.size(); ++i)
    {
        remove(_fileList[i].c_str());
    }

    exit(1);
}
