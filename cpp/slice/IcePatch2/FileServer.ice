// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_FILE_SERVER_ICE
#define ICE_PATCH2_FILE_SERVER_ICE

#include <IcePatch2/FileInfo.ice>

module IcePatch2
{

sequence<Ice::ByteSeq> ByteSeqSeq;

exception NodeOutOfRangeException
{
};

exception FileAccessException
{
    string reason;
};

interface FileServer
{
    nonmutating FileInfoSeq getFileInfoSeq(int node0, int node1)
	throws NodeOutOfRangeException;

    nonmutating ByteSeqSeq getChecksum2Seq(int node0)
	throws NodeOutOfRangeException;

    nonmutating ByteSeqSeq getChecksum1Seq();

    nonmutating Ice::ByteSeq getChecksum0();

    nonmutating Ice::ByteSeq getFileCompressed(string path, int pos, int num)
	throws FileAccessException;
};

};

#endif
