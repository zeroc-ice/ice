// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IcePatch/Util.h>
#include <IcePatch/ClientUtil.h>
#include <fstream>
#include <bzlib.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

string
IcePatch::pathToName(const string& path)
{
    string::size_type pos = path.rfind('/');
    if(pos == string::npos)
    {
	return path;
    }
    else
    {
	return path.substr(pos + 1);
    }
}

void
IcePatch::getRegular(const RegularPrx& regular, ProgressCB& progressCB)
{
    string path = identityToPath(regular->ice_getIdentity());
    string pathBZ2 = path + ".bz2";
    Int totalBZ2 = regular->getBZ2Size();
    Int posBZ2 = 0;

    //
    // Check for partial BZ2 file.
    //
    FileInfo infoBZ2 = getFileInfo(pathBZ2, false);
    if(infoBZ2.type == FileTypeRegular)
    {
	ByteSeq remoteBZ2MD5 = regular->getBZ2MD5(infoBZ2.size);
	ByteSeq localBZ2MD5 = calcPartialMD5(pathBZ2, infoBZ2.size);

	if(remoteBZ2MD5 == localBZ2MD5)
	{
	    posBZ2 = infoBZ2.size;
	}
    }

    //
    // Get the BZ2 file.
    //
    progressCB.startDownload(totalBZ2, posBZ2);

    ofstream fileBZ2(pathBZ2.c_str(), ios::binary | (posBZ2 ? ios::app : ios::openmode(0)));
    if(!fileBZ2)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for writing: " + strerror(errno);
	throw ex;
    }

    while(posBZ2 < totalBZ2)
    {
	const Int numBZ2 = 64 * 1024;

	ByteSeq bytesBZ2 = regular->getBZ2(posBZ2, numBZ2);
	if(bytesBZ2.empty())
	{
	    break;
	}
	
	posBZ2 += bytesBZ2.size();
	
	fileBZ2.write(&bytesBZ2[0], bytesBZ2.size());
	if(!fileBZ2)
	{
	    FileAccessException ex;
	    ex.reason = "cannot write `" + pathBZ2 + "': " + strerror(errno);
	    throw ex;
	}
	
	if(static_cast<Int>(bytesBZ2.size()) < numBZ2)
	{
	    break;
	}
	
	progressCB.updateDownload(totalBZ2, posBZ2);
    }

    progressCB.finishedDownload(totalBZ2);

    fileBZ2.close();
    
    //
    // Read the BZ2 file in blocks and write the original file.
    //
    ofstream file(path.c_str(), ios::binary);
    if(!file)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + path + "' for writing: " + strerror(errno);
	throw ex;
    }
    
    FILE* stdioFileBZ2 = fopen(pathBZ2.c_str(), "rb");
    if(!stdioFileBZ2)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for reading: " + strerror(errno);
	throw ex;
    }
    
    int bzError;
    BZFILE* bzFile = BZ2_bzReadOpen(&bzError, stdioFileBZ2, 0, 0, 0, 0);
    if(bzError != BZ_OK)
    {
	FileAccessException ex;
	ex.reason = "BZ2_bzReadOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex.reason += string(": ") + strerror(errno);
	}
	fclose(stdioFileBZ2);
	throw ex;
    }
    
    const Int numBZ2 = 64 * 1024;
    Byte bytesBZ2[numBZ2];

    try
    {
        progressCB.startUncompress(totalBZ2, 0);
    
        while(bzError != BZ_STREAM_END)
        {
	    int sz = BZ2_bzRead(&bzError, bzFile, bytesBZ2, numBZ2);
	    if(bzError != BZ_OK && bzError != BZ_STREAM_END)
	    {
	        FileAccessException ex;
	        ex.reason = "BZ2_bzRead failed";
	        if(bzError == BZ_IO_ERROR)
	        {
		    ex.reason += string(": ") + strerror(errno);
	        }
	        throw ex;
	    }
	    
	    if(sz > 0)
	    {
	        long pos = ftell(stdioFileBZ2);
	        if(pos == -1)
	        {
		    FileAccessException ex;
		    ex.reason = "cannot get read position for `" + pathBZ2 + "': " + strerror(errno);
		    throw ex;
	        }

	        progressCB.updateUncompress(totalBZ2, pos);
	        
	        file.write(bytesBZ2, sz);
	        if(!file)
	        {
		    FileAccessException ex;
		    ex.reason = "cannot write `" + path + "': " + strerror(errno);
		    throw ex;
	        }
	    }
        }
    
        progressCB.finishedUncompress(totalBZ2);
    }
    catch(...)
    {
        BZ2_bzReadClose(&bzError, bzFile);
        fclose(stdioFileBZ2);
        file.close();
        throw;
    }

    BZ2_bzReadClose(&bzError, bzFile);
    if(bzError != BZ_OK)
    {
	FileAccessException ex;
	ex.reason = "BZ2_bzReadClose failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex.reason += string(": ") + strerror(errno);
	}
	fclose(stdioFileBZ2);
	throw ex;
    }

    fclose(stdioFileBZ2);
    file.close();
    
    //
    // Remove the BZ2 file, it is not needed anymore.
    //
    if(::remove(pathBZ2.c_str()) == -1)
    {
	FileAccessException ex;
	ex.reason = "cannot remove file `" + pathBZ2 + "': " + strerror(errno);
	throw ex;
    }

    //
    // Create a MD5 file for the original file.
    //
    createMD5(path);
}
