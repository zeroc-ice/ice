// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/Util.h>
#include <IcePatch/Node.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <bzlib.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

const string IcePatch::tmpName = ".icepatchtemp";

static string
normalizePath(const string& path)
{
    string result = path;
    
    string::size_type pos;
    
    for (pos = 0; pos < result.size(); ++pos)
    {
	if (result[pos] == '\\')
	{
	    result[pos] = '/';
	}
    }
    
    pos = 0;
    while ((pos = result.find("//", pos)) != string::npos)
    {
	result.erase(pos, 1);
    }
    
    pos = 0;
    while ((pos = result.find("/./", pos)) != string::npos)
    {
	result.erase(pos, 2);
    }

    return result;
}

string
IcePatch::identityToPath(const Identity& identity)
{
    assert(identity.category == "IcePatch");
    return normalizePath(identity.name);
}

Identity
IcePatch::pathToIdentity(const string& path)
{
    Identity identity;
    identity.category = "IcePatch";
    identity.name = normalizePath(path);
    return identity;
}

string
IcePatch::pathToName(const string& path)
{
    string::size_type pos = path.rfind('/');
    if (pos == string::npos)
    {
	return path;
    }
    else
    {
	return path.substr(pos + 1);
    }
}

string
IcePatch::getSuffix(const string& path)
{
    string::size_type pos = path.rfind('.');
    if (pos == string::npos)
    {
	return string();
    }
    else
    {
	return path.substr(pos + 1);
    }
}

FileInfo
IcePatch::getFileInfo(const string& path)
{
    struct stat buf;
    if (::stat(path.c_str(), &buf) == -1)
    {
	if (errno == ENOENT)
	{
	    return FileInfoNotExist;
	}
	else
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	    throw ex;
	}
    }

    if (S_ISDIR(buf.st_mode))
    {
	return FileInfoDirectory;
    }

    if (S_ISREG(buf.st_mode))
    {
	return FileInfoRegular;
    }

    return FileInfoUnknown;
}

StringSeq
IcePatch::readDirectory(const string& path)
{
    struct dirent **namelist;
    int n = ::scandir(path.c_str(), &namelist, 0, alphasort);
    if (n < 0)
    {
	NodeAccessException ex;
	ex.reason = "cannot read directory `" + path + "': " + strerror(errno);
	throw ex;
    }

    StringSeq result;
    result.reserve(n - 2);

    for (int i = 0; i < n; ++i)
    {
	string name = namelist[i]->d_name;

	free(namelist[i]);
	
	if (name != ".." && name != ".")
	{
	    result.push_back(path + '/' + name);
	}
    }
    
    free(namelist);
    return result;
}

void
IcePatch::removeRecursive(const string& path)
{
    if (getFileInfo(path) == FileInfoDirectory)
    {
	StringSeq paths = readDirectory(path);
	StringSeq::const_iterator p;
	for (p = paths.begin(); p != paths.end(); ++p)
	{
	    removeRecursive(*p);
	}
    }
    
    if (::remove(path.c_str()) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot remove file `" + path + "': " + strerror(errno);
	throw ex;
    }
}

void
IcePatch::createDirectory(const string& path)
{
    if (::mkdir(path.c_str(), 00777) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot create directory `" + path + "': " + strerror(errno);
	throw ex;
    }
}

ByteSeq
IcePatch::getMD5(const string& path)
{
    string pathMD5 = path + ".md5";
    ifstream fileMD5(pathMD5.c_str());
    if (!fileMD5)
    {
	NodeAccessException ex;
	ex.reason = "cannot open `" + pathMD5 + "' for reading: " + strerror(errno);
	throw ex;
    }
    ByteSeq bytesMD5;
    bytesMD5.resize(16);
    fileMD5.read(&bytesMD5[0], 16);
    if (!fileMD5)
    {
	NodeAccessException ex;
	ex.reason = "cannot read `" + pathMD5 + "': " + strerror(errno);
	throw ex;
    }
    if (fileMD5.gcount() < 16)
    {
	NodeAccessException ex;
	ex.reason = "could not read 16 bytes from `" + pathMD5 + "'";
	throw ex;
    }
    return bytesMD5;
}

void
IcePatch::createMD5(const string& path)
{
    if (pathToName(path) == tmpName)
    {
	return;
    }

    string suffix = getSuffix(path);
    if (suffix == "md5" || suffix == "bz2")
    {
	return;
    }

    //
    // Stat the file to get a MD5 hash value for.
    //
    struct stat buf;
    if (::stat(path.c_str(), &buf) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	throw ex;
    }
    else
    {
	if (!S_ISREG(buf.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' is not a regular file";
	    throw ex;
	}
    }

    //
    // Stat the MD5 file. If it doesn't exist, or if it's outdated,
    // set a flag to create a new MD5 hash value.
    //
    struct stat bufMD5;
    string pathMD5 = path + ".md5";
    bool makeMD5 = false;
    if (::stat(pathMD5.c_str(), &bufMD5) == -1)
    {
	if (errno == ENOENT)
	{
	    makeMD5 = true;
	}
	else
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	    throw ex;
	}
    }
    else
    {
	if (!S_ISREG(bufMD5.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + pathMD5 + "' is not a regular file";
	    throw ex;
	}
	
	if (bufMD5.st_size != 16)
	{
	    NodeAccessException ex;
	    ex.reason = "`" + pathMD5 + "' isn't 16 bytes in size";
	    throw ex;
	}
	
	if (bufMD5.st_mtime <= buf.st_mtime)
	{
	    makeMD5 = true;
	}
    }

    if (makeMD5)
    {
	ByteSeq bytes;
	bytes.resize(buf.st_size);

	//
	// Read the original file.
	//
	{
	    ifstream file(path.c_str());
	    if (!file)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
		throw ex;
	    }
	    file.read(&bytes[0], bytes.size());
	    if (!file)
	    {
		NodeAccessException ex;
		ex.reason = "cannot read `" + path + "': " + strerror(errno);
		throw ex;
	    }
	    if (file.gcount() < static_cast<int>(bytes.size()))
	    {
		NodeAccessException ex;
		ex.reason = "could not read all bytes from `" + path + "'";
		throw ex;
	    }
	}
	
	//
	// Create the MD5 hash value.
	//
	ByteSeq bytesMD5;
	bytesMD5.resize(16);
	MD5(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(), reinterpret_cast<unsigned char*>(&bytesMD5[0]));
	
	//
	// Save the MD5 hash value to the MD5 file.
	//
	{
	    ofstream fileMD5(tmpName.c_str());
	    if (!fileMD5)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + tmpName + "' for writing: " + strerror(errno);
		throw ex;
	    }
	    fileMD5.write(&bytesMD5[0], 16);
	    if (!fileMD5)
	    {
		NodeAccessException ex;
		ex.reason = "cannot write `" + tmpName + "': " + strerror(errno);
		throw ex;
	    }
	}

	//
	// Rename the temporary MD5 file to the "real" MD5 file. This
	// is done so that there can be no partial MD5 files after an
	// abortive application termination.
	//
	if (rename(tmpName.c_str(), pathMD5.c_str()) == -1)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot rename `" + tmpName + "' to  `" + pathMD5 + "': " + strerror(errno);
	    throw ex;
	}
    }
}

void
IcePatch::createMD5Recursive(const string& path)
{
    FileInfo info = getFileInfo(path);

    if (info == FileInfoDirectory)
    {
	StringSeq paths = readDirectory(path);
	StringSeq::const_iterator p;
	for (p = paths.begin(); p != paths.end(); ++p)
	{
	    createMD5Recursive(*p);
	}
    }
    else if (info == FileInfoRegular)
    {
	createMD5(path);
    }
}

Int
IcePatch::getSizeBZ2(const string& path)
{
    struct stat bufBZ2;
    string pathBZ2 = path + ".bz2";
    if (::stat(pathBZ2.c_str(), &bufBZ2) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	throw ex;
    }
    return bufBZ2.st_size;
}

ByteSeq
IcePatch::getBytesBZ2(const string& path, Int pos, Int num)
{
    string pathBZ2 = path + ".bz2";
    ifstream fileBZ2(pathBZ2.c_str());
    if (!fileBZ2)
    {
	NodeAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for reading: " + strerror(errno);
	throw ex;
    }
    fileBZ2.seekg(pos);
    if (!fileBZ2)
    {
	NodeAccessException ex;
	ostringstream out;
	out << "cannot seek position " << pos << " in file `" << path << "':" << strerror(errno);
	ex.reason = out.str();
	throw ex;
    }
    ByteSeq bytesBZ2;
    bytesBZ2.resize(num);
    fileBZ2.read(&bytesBZ2[0], bytesBZ2.size());
    if (!fileBZ2 && !fileBZ2.eof())
    {
	NodeAccessException ex;
	ex.reason = "cannot read `" + pathBZ2 + "': " + strerror(errno);
	throw ex;
    }
    bytesBZ2.resize(fileBZ2.gcount());
    return bytesBZ2;
}

void
IcePatch::createBZ2(const string& path)
{
    if (pathToName(path) == tmpName)
    {
	return;
    }

    string suffix = getSuffix(path);
    if (suffix == "md5" || suffix == "bz2")
    {
	return;
    }

    //
    // Stat the file to get a BZ2 file for.
    //
    struct stat buf;
    if (::stat(path.c_str(), &buf) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	throw ex;
    }
    else
    {
	if (!S_ISREG(buf.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' is not a regular file";
	    throw ex;
	}
    }

    //
    // Stat the BZ2 file. If it doesn't exist, or if it's outdated,
    // set a flag to create a new BZ2 file.
    //
    struct stat bufBZ2;
    string pathBZ2 = path + ".bz2";
    bool makeBZ2 = false;
    if (::stat(pathBZ2.c_str(), &bufBZ2) == -1)
    {
	if (errno == ENOENT)
	{
	    makeBZ2 = true;
	}
	else
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "': " + strerror(errno);
	    throw ex;
	}
    }
    else
    {
	if (!S_ISREG(bufBZ2.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + pathBZ2 + "' is not a regular file";
	    throw ex;
	}

	if (bufBZ2.st_mtime <= buf.st_mtime)
	{
	    makeBZ2 = true;
	}
    }

    if (makeBZ2)
    {
	//
	// Read the original file in blocks and write the BZ2 file.
	//
	{
	    ifstream file(path.c_str());
	    if (!file)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
		throw ex;
	    }
	    
	    FILE* fileBZ2 = fopen(tmpName.c_str(), "wb");
	    if (!fileBZ2)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + tmpName + "' for writing: " + strerror(errno);
		throw ex;
	    }
	    
	    int bzError;
	    BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, fileBZ2, 5, 0, 0);
	    if (bzError != BZ_OK)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzWriteOpen failed";
		if (bzError == BZ_IO_ERROR)
		{
		    ex.reason += string(": ") + strerror(errno);
		}
		fclose(fileBZ2);
		throw ex;
	    }
	    
	    static const Int num = 64 * 1024;
	    Byte bytes[num];
	    
	    while (!file.eof())
	    {
		file.read(bytes, num);
		if (!file && !file.eof())
		{
		    NodeAccessException ex;
		    ex.reason = "cannot read `" + path + "': " + strerror(errno);
		    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
		    fclose(fileBZ2);
		    throw ex;
		}
		
		if (file.gcount() > 0)
		{
		    BZ2_bzWrite(&bzError, bzFile, bytes, file.gcount());
		    if (bzError != BZ_OK)
		    {
			NodeAccessException ex;
			ex.reason = "BZ2_bzWrite failed";
			if (bzError == BZ_IO_ERROR)
			{
			    ex.reason += string(": ") + strerror(errno);
			}
			BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
			fclose(fileBZ2);
			throw ex;
		    }
		}
	    }
	    
	    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
	    if (bzError != BZ_OK)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzWriteClose failed";
		if (bzError == BZ_IO_ERROR)
		{
		    ex.reason += string(": ") + strerror(errno);
		}
		fclose(fileBZ2);
		throw ex;
	    }
	    
	    fclose(fileBZ2);
	}
	
	//
	// Rename the temporary BZ2 file to the "real" BZ2 file. This
	// is done so that there can be no partial BZ2 files after an
	// abortive application termination.
	//
	if (rename(tmpName.c_str(), pathBZ2.c_str()) == -1)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot rename `" + tmpName + "' to  `" + pathBZ2 + "': " + strerror(errno);
	    throw ex;
	}
    }
}

void
IcePatch::createBZ2Recursive(const string& path)
{
    FileInfo info = getFileInfo(path);

    if (info == FileInfoDirectory)
    {
	StringSeq paths = readDirectory(path);
	StringSeq::const_iterator p;
	for (p = paths.begin(); p != paths.end(); ++p)
	{
	    createBZ2Recursive(*p);
	}
    }
    else if (info == FileInfoRegular)
    {
	createBZ2(path);
    }
}

void
IcePatch::getFile(const FilePrx& file, ProgressCB& progressCB)
{
    string path = identityToPath(file->ice_getIdentity());
    string pathBZ2 = path + ".bz2";

    //
    // Get the BZ2 file.
    //
    {
	Int totalBZ2 = file->getSizeBZ2();
	progressCB.start(totalBZ2);
	
	ofstream fileBZ2(pathBZ2.c_str());
	if (!fileBZ2)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + pathBZ2 + "' for writing: " + strerror(errno);
	    throw ex;
	}
	ByteSeq bytesBZ2;
	Int pos = 0;
	while(pos < totalBZ2)
	{
	    static const Int num = 64 * 1024;
	    
	    bytesBZ2 = file->getBytesBZ2(pos, num);
	    if (bytesBZ2.empty())
	    {
		break;
	    }
	    
	    pos += bytesBZ2.size();
	    
	    fileBZ2.write(&bytesBZ2[0], bytesBZ2.size());
	    if (!fileBZ2)
	    {
		NodeAccessException ex;
		ex.reason = "cannot write `" + pathBZ2 + "': " + strerror(errno);
		throw ex;
	    }
	    
	    if (static_cast<Int>(bytesBZ2.size()) < num)
	    {
		break;
	    }
	    
	    progressCB.update(totalBZ2, pos);
	}
	
	progressCB.finished(totalBZ2);
    }
    
    //
    // Read the BZ2 file in blocks and write the original file.
    //
    {
	ofstream file(path.c_str());
	if (!file)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + path + "' for writing: " + strerror(errno);
	    throw ex;
	}
	
	FILE* fileBZ2 = fopen(pathBZ2.c_str(), "rb");
	if (!fileBZ2)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + pathBZ2 + "' for reading: " + strerror(errno);
	    throw ex;
	}
	
	int bzError;
	BZFILE* bzFile = BZ2_bzReadOpen(&bzError, fileBZ2, 0, 0, 0, 0);
	if (bzError != BZ_OK)
	{
	    NodeAccessException ex;
	    ex.reason = "BZ2_bzReadOpen failed";
	    if (bzError == BZ_IO_ERROR)
	    {
		ex.reason += string(": ") + strerror(errno);
	    }
	    fclose(fileBZ2);
	    throw ex;
	}
	
	static const Int num = 64 * 1024;
	Byte bytes[num];
	
	while (bzError != BZ_STREAM_END)
	{
	    int sz = BZ2_bzRead(&bzError, bzFile, bytes, num);
	    if (bzError != BZ_OK && bzError != BZ_STREAM_END)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzRead failed";
		if (bzError == BZ_IO_ERROR)
		{
		    ex.reason += string(": ") + strerror(errno);
		}
		BZ2_bzReadClose(&bzError, bzFile);
		fclose(fileBZ2);
		throw ex;
	    }
	    
	    if (sz > 0)
	    {
		file.write(bytes, sz);
		if (!file)
		{
		    NodeAccessException ex;
		    ex.reason = "cannot write `" + path + "': " + strerror(errno);
		    BZ2_bzReadClose(&bzError, bzFile);
		    fclose(fileBZ2);
		    throw ex;
		}
	    }
	}
	
	BZ2_bzReadClose(&bzError, bzFile);
	if (bzError != BZ_OK)
	{
	    NodeAccessException ex;
	    ex.reason = "BZ2_bzReadClose failed";
	    if (bzError == BZ_IO_ERROR)
	    {
		ex.reason += string(": ") + strerror(errno);
	    }
	    fclose(fileBZ2);
	    throw ex;
	}
	fclose(fileBZ2);
    }
    
    //
    // Remove the BZ2 file, it is not needed anymore.
    //
    if (::remove(pathBZ2.c_str()) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot remove file `" + pathBZ2 + "': " + strerror(errno);
	throw ex;
    }

    //
    // Create a MD5 file for the original file.
    //
    createMD5(path);
}
