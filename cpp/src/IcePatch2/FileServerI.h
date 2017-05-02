// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    void getFileCompressed_async(const AMD_FileServer_getFileCompressedPtr&,
                                 const std::string&,
                                 Ice::Int,
                                 Ice::Int,
                                 const Ice::Current&) const;

    void getLargeFileCompressed_async(const AMD_FileServer_getLargeFileCompressedPtr&,
                                      const std::string&,
                                      Ice::Long,
                                      Ice::Int,
                                      const Ice::Current&) const;

private:

    void
    getFileCompressedInternal(const std::string&,
                              Ice::Long,
                              Ice::Int,
                              std::vector<Ice::Byte>&,
                              bool) const;

    const std::string _dataDir;
    const IcePatch2Internal::FileTree0 _tree0;
};

}

#endif
