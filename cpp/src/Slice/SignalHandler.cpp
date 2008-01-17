// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Slice/SignalHandler.h>
#include <vector>

#ifndef _WIN32
#    include <signal.h>
#endif


using namespace std;

//
// Signal handler routine to unlink output files in case of translator
// being interrupted.
//
static vector<string> _fileList;

#ifdef _WIN32
static BOOL WINAPI signalHandler(DWORD dwCtrlType)
#else
static void signalHandler(int signal)
#endif
{
    for(unsigned int i = 0; i < _fileList.size(); ++i)
    {
        remove(_fileList[i].c_str());
    }

    exit(1);
}


Slice::SignalHandler::SignalHandler()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(signalHandler, TRUE);
#else
    sigset(SIGHUP, signalHandler);
    sigset(SIGINT, signalHandler);
    sigset(SIGQUIT, signalHandler);
#endif
}

Slice::SignalHandler::~SignalHandler()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(signalHandler, FALSE);
#else
    sigset(SIGHUP, SIG_DFL);
    sigset(SIGINT, SIG_DFL);
    sigset(SIGQUIT, SIG_DFL);
#endif

    _fileList.clear();
}

void
Slice::SignalHandler::addFile(const string& file)
{
    _fileList.push_back(file);
}
