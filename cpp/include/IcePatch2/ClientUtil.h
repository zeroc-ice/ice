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
    virtual ~Patcher();

    //
    // Returns true if the patch preparation was successful, false if
    // preparation failed (for example, because a thorough patch is
    // necessary, but the user chose not to patch thorough), or raises
    // std::string as an exception if there was an error.
    //
    bool prepare();

    //
    // Returns true if patching was successful, false if patching was
    // aborted by the user, or raises std::string as an exception if
    // there was an error.
    //
    bool patch();

private:

    bool removeFiles(const FileInfoSeq&);
    bool updateFiles(const FileInfoSeq&);

    virtual void run();

    const PatcherFeedbackPtr _feedback;
    const std::string _dataDir;
    const bool _thorough;
    const bool _dryRun;
    const Ice::Int _chunkSize;
    const FileServerPrx _serverCompress;
    const FileServerPrx _serverNoCompress;

    FileInfoSeq _localFiles;
    FileInfoSeq _updateFiles;
    FileInfoSeq _removeFiles;

    std::ofstream _updateLog;

    bool _decompress;
    std::string _decompressException;
    std::list<FileInfo> _decompressList;
};

typedef IceUtil::Handle<Patcher> PatcherPtr;

}

#endif
