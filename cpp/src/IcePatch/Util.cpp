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
	    ex.reason = "cannot stat `" + path + "':" + strerror(errno);
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
	ex.reason = "cannot read directory `" + path + "':" + strerror(errno);
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
	ex.reason = "cannot remove file `" + path + "':" + strerror(errno);
	throw ex;
    }
}

void
IcePatch::createDirectory(const string& path)
{
    if (::mkdir(path.c_str(), 00777) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot create directory `" + path + "':" + strerror(errno);
	throw ex;
    }
}

ByteSeq
IcePatch::getMD5(const string& path)
{
    //
    // Stat the file to get a MD5 hash value for.
    //
    struct stat buf;
    if (::stat(path.c_str(), &buf) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot stat `" + path + "':" + strerror(errno);
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
    bool createMD5 = false;
    if (::stat(pathMD5.c_str(), &bufMD5) == -1)
    {
	if (errno == ENOENT)
	{
	    createMD5 = true;
	}
	else
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "':" + strerror(errno);
	    throw ex;
	}
    }
    else
    {
	if (!S_ISREG(bufMD5.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' is not a regular file";
	    throw ex;
	}
	
	if (bufMD5.st_size != 16)
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' isn't 16 bytes in size";
	    throw ex;
	}
	
	if (bufMD5.st_mtime <= buf.st_mtime)
	{
	    createMD5 = true;
	}
    }

    ByteSeq bytesMD5;
    bytesMD5.resize(16);

    if (createMD5)
    {
	//
	// Read the original file.
	//
	ifstream file(path.c_str());
	if (!file)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + path + "' for reading:" + strerror(errno);
	    throw ex;
	}
	ByteSeq bytes;
	bytes.resize(buf.st_size);
	file.read(&bytes[0], bytes.size());
	if (!file)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot read `" + path + "':" + strerror(errno);
	    throw ex;
	}
	if (file.gcount() < static_cast<int>(bytes.size()))
	{
	    NodeAccessException ex;
	    ex.reason = "could not read all bytes from `" + path + "'";
	    throw ex;
	}
	file.close();
	
	//
	// Create the MD5 hash value.
	//
	MD5(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(), reinterpret_cast<unsigned char*>(&bytesMD5[0]));
	
	//
	// Save the MD5 hash value to the MD5 file.
	//
	ofstream fileMD5(pathMD5.c_str());
	if (!fileMD5)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + pathMD5 + "' for writing:" + strerror(errno);
	    throw ex;
	}
	fileMD5.write(&bytesMD5[0], 16);
	if (!fileMD5)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot write `" + pathMD5 + "':" + strerror(errno);
	    throw ex;
	}
	fileMD5.close();
    }
    else
    {
	//
	// Read the MD5 hash value from the MD5 file.
	//
	ifstream fileMD5(pathMD5.c_str());
	if (!fileMD5)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + pathMD5 + "' for reading:" + strerror(errno);
	    throw ex;
	}
	fileMD5.read(&bytesMD5[0], 16);
	if (!fileMD5)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot read `" + pathMD5 + "':" + strerror(errno);
	    throw ex;
	}
	if (fileMD5.gcount() < 16)
	{
	    NodeAccessException ex;
	    ex.reason = "could not read 16 bytes from `" + pathMD5 + "'";
	    throw ex;
	}
	fileMD5.close();
    }

    return bytesMD5;
}

string
IcePatch::MD5ToString(const ByteSeq& bytesMD5)
{
    if (bytesMD5.size() != 16)
    {
	return "illegal MD5 hash code";
    }

    ostringstream out;

    for (int i = 0; i < 16; ++i)
    {
	int b = static_cast<int>(bytesMD5[i]);
	if (b < 0)
	{
	    b += 256;
	}
	out << hex << b;
    }

    return out.str();
}

void
IcePatch::writeBZ2(const string& pathBZ2, const ByteSeq& bytes)
{
    FILE* file = fopen(pathBZ2.c_str(), "wb");
    if (!file)
    {
	NodeAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for writing:" + strerror(errno);
	throw ex;
    }
    
    try
    {
	int bzError;
	BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, file, 5, 0, 0);
	if (bzError != BZ_OK)
	{
	    NodeAccessException ex;
	    ex.reason = "BZ2_bzWriteOpen failed";
	    throw ex;
	}
	
	try
	{
	    BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[0]), bytes.size());
	    if (bzError != BZ_OK)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzWrite failed";
		throw ex;
	    }

	    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
	    if (bzError != BZ_OK)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzWriteClose failed";
		throw ex;
	    }
	}
	catch (...)
	{
	    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
	    if (bzError != BZ_OK)
	    {
		NodeAccessException ex;
		ex.reason = "BZ2_bzWriteClose failed";
		throw ex;
	    }
	    throw;
	}
	
	fclose(file);
    }
    catch (...)
    {
	fclose(file);
	throw;
    }
}

ByteSeq
IcePatch::getBytesBZ2(const string& path, Int pos, Int num)
{
    //
    // Stat the file to get a BZ2 file for.
    //
    struct stat buf;
    if (::stat(path.c_str(), &buf) == -1)
    {
	NodeAccessException ex;
	ex.reason = "cannot stat `" + path + "':" + strerror(errno);
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
    bool createBZ2 = false;
    if (::stat(pathBZ2.c_str(), &bufBZ2) == -1)
    {
	if (errno == ENOENT)
	{
	    createBZ2 = true;
	}
	else
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "':" + strerror(errno);
	    throw ex;
	}
    }
    else
    {
	if (!S_ISREG(bufBZ2.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' is not a regular file";
	    throw ex;
	}

	if (bufBZ2.st_mtime <= buf.st_mtime)
	{
	    createBZ2 = true;
	}
    }

    if (createBZ2)
    {
	//
	// Read the original file.
	//
	ifstream file(path.c_str());
	if (!file)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + path + "' for reading:" + strerror(errno);
	    throw ex;
	}
	ByteSeq bytes;
	bytes.resize(buf.st_size);
	file.read(&bytes[0], bytes.size());
	if (!file)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot read `" + path + "':" + strerror(errno);
	    throw ex;
	}
	if (file.gcount() < static_cast<int>(bytes.size()))
	{
	    NodeAccessException ex;
	    ex.reason = "could not read all bytes from `" + path + "'";
	    throw ex;
	}
	file.close();

	//
	// Create the BZ2 file.
	//
	writeBZ2(pathBZ2, bytes);

/*
	//
	// Stat the BZ2 file. This time, the BZ2 file must exist,
	// otherwise it's an error.
	//
	if (::stat(pathBZ2.c_str(), &bufBZ2) == -1)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot stat `" + path + "':" + strerror(errno);
	    throw ex;
	}
*/
    }

    //
    // Read the BZ2 file.
    //
    ifstream fileBZ2(pathBZ2.c_str());
    if (!fileBZ2)
    {
	NodeAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for reading:" + strerror(errno);
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
	ex.reason = "cannot read `" + pathBZ2 + "':" + strerror(errno);
	throw ex;
    }
    bytesBZ2.resize(fileBZ2.gcount());
    fileBZ2.close();

    return bytesBZ2;
}

void
IcePatch::getFile(const FilePrx& file)
{
    string path = identityToPath(file->ice_getIdentity());

    string pathBZ2 = path + ".bz2";
    ofstream fileBZ2(pathBZ2.c_str());
    if (!fileBZ2)
    {
	NodeAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for writing:" + strerror(errno);
	throw ex;
    }
    ByteSeq bytesBZ2;
    Int pos = 0;
    while(true)
    {
	bytesBZ2 = file->getBytesBZ2(pos, 256 * 1024);
	if (bytesBZ2.empty())
	{
	    break;
	}

	pos += bytesBZ2.size();

	fileBZ2.write(&bytesBZ2[0], bytesBZ2.size());
	if (!fileBZ2)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot write `" + pathBZ2 + "':" + strerror(errno);
	    throw ex;
	}
    }
    fileBZ2.close();
}
