//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PATCH2_FILE_SERVER_I_H
#define ICE_PATCH2_FILE_SERVER_I_H

#include <IcePatch2Lib/Util.h>
#include <IcePatch2/FileServer.h>

namespace IcePatch2
{
    class FileServerI final : public FileServer
    {
    public:
        FileServerI(const std::string&, const LargeFileInfoSeq&);

        FileInfoSeq getFileInfoSeq(std::int32_t, const Ice::Current&) const;

        LargeFileInfoSeq getLargeFileInfoSeq(std::int32_t, const Ice::Current&) const final;

        ByteSeqSeq getChecksumSeq(const Ice::Current&) const final;

        Ice::ByteSeq getChecksum(const Ice::Current&) const final;

        void getFileCompressedAsync(
            std::string,
            std::int32_t,
            std::int32_t,
            std::function<void(std::pair<const std::byte*, const std::byte*> returnValue)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const;

        void getLargeFileCompressedAsync(
            std::string,
            std::int64_t,
            std::int32_t,
            std::function<void(std::pair<const std::byte*, const std::byte*> returnValue)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const final;

    private:
        void getFileCompressedInternal(std::string, std::int64_t, std::int32_t, std::vector<std::byte>&, bool) const;

        const std::string _dataDir;
        const IcePatch2Internal::FileTree0 _tree0;
    };
}

#endif
