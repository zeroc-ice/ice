// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/SignalHandler.h>
#include <signal.h>
#include <vector>

using namespace std;

//
// Signal handler routine to unlink output files in case of translator
// being interrupted.
//
static vector<string> _fileList;

static void
signalHandler(int signal)
{
    for(unsigned int i = 0; i < _fileList.size(); ++i)
    {
        unlink(_fileList[i].c_str());
    }

    exit(1);
}

Slice::SignalHandler::SignalHandler()
{
    sigset(SIGHUP, signalHandler);
    sigset(SIGINT, signalHandler);
    sigset(SIGQUIT, signalHandler);
}

Slice::SignalHandler::~SignalHandler()
{
    sigset(SIGHUP, SIG_DFL);
    sigset(SIGINT, SIG_DFL);
    sigset(SIGQUIT, SIG_DFL);

    _fileList.clear();
}

void
Slice::SignalHandler::addFile(const string& file)
{
    _fileList.push_back(file);
}
