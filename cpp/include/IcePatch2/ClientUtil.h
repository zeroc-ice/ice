// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_CLIENT_UTIL_H
#define ICE_PATCH2_CLIENT_UTIL_H

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <IcePatch2/FileServer.h>
#include <fstream>
#include <list>

namespace IcePatch2
{

class ICE_PATCH2_API PatcherFeedback : public IceUtil::Shared
{
public:

    virtual bool noFileSummary(const std::string&) = 0;

    virtual bool fileListStart() = 0;
    virtual bool fileListProgress(Ice::Int) = 0;
    virtual bool fileListEnd() = 0;

    virtual bool patchStart(const std::string&, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchProgress(Ice::Long, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchEnd() = 0;
};

typedef IceUtil::Handle<PatcherFeedback> PatcherFeedbackPtr;

class ICE_PATCH2_API Patcher : public IceUtil::Thread, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Patcher(const Ice::CommunicatorPtr&, const PatcherFeedbackPtr&);
    ~Patcher();

    void patch();

    virtual void run();
    
private:

    const Ice::CommunicatorPtr _communicator;
    const PatcherFeedbackPtr _feedback;

    bool _stop;
    std::string _exception;
    std::list<FileInfo> _infoSeq;
    std::ofstream _fileLog;
};

typedef IceUtil::Handle<Patcher> PatcherPtr;

}

#endif
