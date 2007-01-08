// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <stdio.h>

namespace IcePatch2
{

class ICE_PATCH2_API PatcherFeedback : public IceUtil::Shared
{
public:

    virtual bool noFileSummary(const std::string&) = 0;

    virtual bool checksumStart() = 0;
    virtual bool checksumProgress(const std::string&) = 0;
    virtual bool checksumEnd() = 0;

    virtual bool fileListStart() = 0;
    virtual bool fileListProgress(Ice::Int) = 0;
    virtual bool fileListEnd() = 0;

    virtual bool patchStart(const std::string&, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchProgress(Ice::Long, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchEnd() = 0;
};

typedef IceUtil::Handle<PatcherFeedback> PatcherFeedbackPtr;

class Decompressor;
typedef IceUtil::Handle<Decompressor> DecompressorPtr;

class ICE_PATCH2_API Patcher : public IceUtil::Shared
{
public:

    Patcher(const Ice::CommunicatorPtr&, const PatcherFeedbackPtr&);
    Patcher(const FileServerPrx&, const PatcherFeedbackPtr&, const std::string&, bool, Ice::Int, Ice::Int);
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
    bool patch(const std::string&);

    void finish();

private:

    void init(const FileServerPrx&);
    bool removeFiles(const FileInfoSeq&);
    bool updateFiles(const FileInfoSeq&);
    bool updateFilesInternal(const FileInfoSeq&, const DecompressorPtr&);
    bool updateFlags(const FileInfoSeq&);

    const PatcherFeedbackPtr _feedback;
    const std::string _dataDir;
    const bool _thorough;
    const Ice::Int _chunkSize;
    const Ice::Int _remove;
    const FileServerPrx _serverCompress;
    const FileServerPrx _serverNoCompress;

    FileInfoSeq _localFiles;
    FileInfoSeq _updateFiles;
    FileInfoSeq _updateFlags;
    FileInfoSeq _removeFiles;

    FILE* _log;
};

typedef IceUtil::Handle<Patcher> PatcherPtr;

}

#endif
