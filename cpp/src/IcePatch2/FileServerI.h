//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PATCH2_FILE_SERVER_I_H
#define ICE_PATCH2_FILE_SERVER_I_H

#include <IcePatch2Lib/Util.h>
#include <IcePatch2/FileServer.h>

namespace IcePatch2
{

class FileServerI : public FileServer
{
public:

    FileServerI(const std::string&, const LargeFileInfoSeq&);

    FileInfoSeq getFileInfoSeq(Ice::Int, const Ice::Current&) const;

    LargeFileInfoSeq
    getLargeFileInfoSeq(Ice::Int, const Ice::Current&) const;

    ByteSeqSeq getChecksumSeq(const Ice::Current&) const;

    Ice::ByteSeq getChecksum(const Ice::Current&) const;

    void getFileCompressedAsync(
        std::string,
        Ice::Int,
        Ice::Int,
        std::function<void(const std::pair<const Ice::Byte*, const Ice::Byte*>& returnValue)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) const;

    void getLargeFileCompressedAsync(
        std::string,
        int64_t,
        Ice::Int,
        std::function<void(const std::pair<const Ice::Byte*, const Ice::Byte*>& returnValue)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) const;

private:

    void
    getFileCompressedInternal(
        std::string,
        int64_t,
        Ice::Int,
        std::vector<Ice::Byte>&,
        bool) const;

    const std::string _dataDir;
    const IcePatch2Internal::FileTree0 _tree0;
};

}

#endif
