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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <openssl/md5.h>

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

StringSeq
IcePatch::readDirectory(const string& path)
{
    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
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

ByteSeq
IcePatch::getMD5(const std::string& path)
{
    //
    // Stat the file to get a MD5 hash value for.
    //
    struct stat buf;
    if (stat(path.c_str(), &buf) == -1)
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
    // Stat the .md5 file. If it doesn't exist, or if it's outdated,
    // set a flag to create a new MD5 hash value.
    //
    struct stat bufmd5;
    string pathmd5 = path + ".md5";
    unsigned char md5[16];
    bool createmd5 = false;
    if (stat(pathmd5.c_str(), &bufmd5) == -1)
    {
	if (errno == ENOENT)
	{
	    createmd5 = true;
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
	if (!S_ISREG(bufmd5.st_mode))
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' is not a regular file";
	    throw ex;
	}
	
	if (bufmd5.st_size != 16)
	{
	    NodeAccessException ex;
	    ex.reason = "`" + path + "' isn't 16 bytes in size";
	    throw ex;
	}
	
	if (bufmd5.st_mtime <= buf.st_mtime)
	{
	    createmd5 = true;
	}
    }

    if (createmd5)
    {
	//
	// Open the original file and create a MD5 hash value
	//
	{
	    int fd = open(path.c_str(), O_RDONLY);
	    
	    if (fd == -1)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + path + "' for reading:" + strerror(errno);
		throw ex;
	    }
	    
	    unsigned char* fileBuf = new unsigned char[buf.st_size];
	    
	    try
	    {
		int sz = read(fd, fileBuf, buf.st_size);
		
		if (sz == -1)
		{
		    NodeAccessException ex;
		    ex.reason = "cannot read `" + path + "':" + strerror(errno);
		    throw ex;
		}
		
		if (sz < buf.st_size)
		{
		    NodeAccessException ex;
		    ex.reason = "could not read all bytes from `" + path + "'";
		    throw ex;
		}
		
		MD5(fileBuf, sz, md5);
		
		close(fd);
		delete [] fileBuf;
	    }
	    catch (...)
	    {
		close(fd);
		delete [] fileBuf;
		throw;
	    }
	}

	//
	// Write the MD5 hash value to the corresponding .md5 file.
	//
	{
	    int fd = open(pathmd5.c_str(), O_WRONLY | O_CREAT, 00644);
	    
	    if (fd == -1)
	    {
		NodeAccessException ex;
		ex.reason = "cannot open `" + pathmd5 + "' for writing:" + strerror(errno);
		throw ex;
	    }
	    
	    try
	    {
		int sz = write(fd, md5, 16);
		
		if (sz == -1)
		{
		    NodeAccessException ex;
		    ex.reason = "cannot read `" + path + "':" + strerror(errno);
		    throw ex;
		}
		
		if (sz < 16)
		{
		    NodeAccessException ex;
		    ex.reason = "could not write 16 bytes to `" + path + "'";
		    throw ex;
		}
		
		close(fd);
	    }
	    catch (...)
	    {
		close(fd);
		throw;
	    }
	}
    }
    else
    {
	//
	// Read the MD5 hash value from the .md5 file.
	//
	int fd = open(pathmd5.c_str(), O_RDONLY);
	
	if (fd == -1)
	{
	    NodeAccessException ex;
	    ex.reason = "cannot open `" + path + "' for reading:" + strerror(errno);
	    throw ex;
	}
	
	try
	{
	    int sz = read(fd, md5, 16);
	    
	    if (sz == -1)
	    {
		NodeAccessException ex;
		ex.reason = "cannot read `" + path + "':" + strerror(errno);
		throw ex;
	    }
	    
	    if (sz < 16)
	    {
		NodeAccessException ex;
		ex.reason = "could not read 16 bytes from `" + path + "'";
		throw ex;
	    }
	    
	    close(fd);
	}
	catch (...)
	{
	    close(fd);
	    throw;
	}
    }

    //
    // Convert array to byte sequence.
    //
    ByteSeq result;
    result.resize(16);
    for (int i = 0; i < 16; ++i)
    {
	result[i] = md5[i];
    }
    return result;
}

std::string
IcePatch::MD5ToString(const Ice::ByteSeq& md5)
{
    if (md5.size() != 16)
    {
	return "illegal MD5 hash code";
    }

    ostringstream out;

    for (int i = 0; i < 16; ++i)
    {
	int b = static_cast<int>(md5[i]);
	if (b < 0)
	{
	    b += 256;
	}
	out << hex << b;
    }

    return out.str();
}
