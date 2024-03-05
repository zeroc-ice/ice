//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PATCH2_CLIENT_UTIL_H
#define ICE_PATCH2_CLIENT_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch2/FileServer.h>

#include <stdio.h>

namespace IcePatch2
{

//
// The IcePatch2::PatcherFeedback class is implemented by IcePatch2 clients to
// allow the user to interact with the patching and report progress
// on the patching.
//
class ICEPATCH2_API PatcherFeedback
{
public:

    virtual ~PatcherFeedback();

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
    virtual bool fileListProgress(std::int32_t) = 0;
    virtual bool fileListEnd() = 0;

    //
    // These methods are called to report on the progression of the
    // file patching. Files to be updated are downloaded from the
    // server, uncompressed and written to the local data directory.
    // These methods should return false to interrupt the patching,
    // false otherwise.
    //
    virtual bool patchStart(const std::string&, std::int64_t, std::int64_t, std::int64_t) = 0;
    virtual bool patchProgress(std::int64_t, std::int64_t, std::int64_t, std::int64_t) = 0;
    virtual bool patchEnd() = 0;
};
typedef std::shared_ptr<PatcherFeedback> PatcherFeedbackPtr;

//
// IcePatch2 clients instantiate the IcePatch2::Patcher class to patch
// a given local data directory.
//
class ICEPATCH2_API Patcher
{
public:

    virtual ~Patcher();

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
typedef std::shared_ptr<Patcher> PatcherPtr;

// IcePatch2 clients instantiate the IcePatch2::Patcher class using the patcher factory.
class ICEPATCH2_API PatcherFactory
{
public:

    /// Create a patcher with the given parameters.
    /// @server The proxy to the IcePath2 server.
    /// @feedback The feedback object to use to report progress.
    /// @dataDir The local data directory to patch.
    /// @thorough If true, a thorough patch is performed and IcePatch2 client recomputes all checksums.
    /// @chunkSize The size (in kilobytes) of the chunks to use when downloading files.
    /// @remove Whether to delete files that exist locally, but not on the server. A negative or zero value prevents
    /// removal of files. A value of 1 enables removal and causes the client to halt with an error if removal of a file
    /// fails. A value of 2 or greater also enables removal, but causes the client to silently ignore errors during
    /// removal.
    static PatcherPtr create(
        const FileServerPrx& server,
        const PatcherFeedbackPtr&feedback,
        const std::string& dataDir,
        bool thorough,
        std::int32_t chunkSize,
        std::int32_t remove);
};

}

#endif
