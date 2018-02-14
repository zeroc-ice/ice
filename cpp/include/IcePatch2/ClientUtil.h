// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

#include <stdio.h>

namespace IcePatch2
{

//
// The IcePatch2::PatcherFeedback class is implemented by IcePatch2 clients to
// allow the user to interact with the patching and report progress 
// on the patching.
//
class ICE_PATCH2_API PatcherFeedback : public IceUtil::Shared
{
public:

    //
    // The summary file can't be loaded for the given reason. This
    // should return true to accept doing a thorough patch, false
    // otherwise.
    //
    virtual bool noFileSummary(const std::string& reason) = 0;

    //
    // If no summary file is found and/or a thorough patch was
    // specified, the following checksum methods are called to report
    // the progression of the checksum computation for the local data
    // directory. These methods should return false to interrupt the
    // checksum, false otherwise.
    //
    virtual bool checksumStart() = 0;
    virtual bool checksumProgress(const std::string&) = 0;
    virtual bool checksumEnd() = 0;

    //
    // These methods are called to report on the progression of the
    // computation of the list of files to patch. This involves
    // comparing the local checksums with the server checksums. These
    // methods should return false to interrupt the computation, false
    // otherwise.
    //
    virtual bool fileListStart() = 0;
    virtual bool fileListProgress(Ice::Int) = 0;
    virtual bool fileListEnd() = 0;

    //
    // These methods are called to report on the progression of the
    // file patching. Files to be updated are downloaded from the
    // server, uncompressed and written to the local data directory.
    // These methods should return false to interrupt the patching,
    // false otherwise.
    //
    virtual bool patchStart(const std::string&, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchProgress(Ice::Long, Ice::Long, Ice::Long, Ice::Long) = 0;
    virtual bool patchEnd() = 0;
};
typedef IceUtil::Handle<PatcherFeedback> PatcherFeedbackPtr;

//
// IcePatch2 clients instantiate the IcePatch2::Patcher class to patch
// a given local data directory.
//
class ICE_PATCH2_API Patcher : public IceUtil::Shared
{
public:
    //
    // Prepare the patching. This involves creating the local checksum
    // files if no summary file exists or if a thorough patch was
    // specified. This method also computes the list of files to be
    // patched. This should be called once before any call to patch().
    // 
    // Returns true if the patch preparation was successful, false if
    // preparation failed (for example, because a thorough patch is
    // necessary, but the user chose not to patch thorough), or raises
    // std::string as an exception if there was an error.
    //
    virtual bool prepare() = 0;

    //
    // Patch the files from the given path.
    //
    // Returns true if patching was successful, false if patching was
    // aborted by the user, or raises std::string as an exception if
    // there was an error.
    //
    virtual bool patch(const std::string&) = 0;

    //
    // Finish the patching. This needs to be called once when the
    // patching is finished to write the local checksum files to the
    // disk.
    //
    virtual void finish() = 0;
};
typedef IceUtil::Handle<Patcher> PatcherPtr;

//
// IcePatch2 clients instantiate the IcePatch2::Patcher class
// using the patcher factory.
//
class ICE_PATCH2_API PatcherFactory : public IceUtil::noncopyable
{
public:
    
    //
    // Create a patcher using configuration properties. The following
    // properties are used to configure the patcher: 
    //
    // - IcePatch2.InstanceName
    // - IcePatch2.Endpoints
    // - IcePatch2.Directory
    // - IcePatch2.Thorough
    // - IcePatch2.ChunkSize
    // - IcePatch2.Remove
    //
    // See the Ice manual for more information on these properties.
    //
    static PatcherPtr create(const Ice::CommunicatorPtr&, const PatcherFeedbackPtr&);

    //
    // Create a patcher with the given parameters. These parameters
    // are equivalent to the configuration properties described above.
    //
    static PatcherPtr create(const FileServerPrx&, const PatcherFeedbackPtr&, const std::string&, bool, Ice::Int, Ice::Int);
};

}

#endif
