// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <IcePatch/Util.h>
#include <IceUtil/InputUtil.h>
#include <fstream>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <bzlib.h>

#ifdef _WIN32
#   include <direct.h>
#   include <io.h>
#   include <sys/utime.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <unistd.h>
#   include <dirent.h>
#   include <utime.h>
#endif

//
// Stupid Visual C++ defines min and max as macros :-(
//
#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

//
// Sun-OS doesn't have scandir() or alphasort().
//
#ifdef __sun

static int
scandir(const char* dir, struct dirent*** namelist,
	int (*select)(const struct dirent*),
	int (*compar)(const void*, const void*))
{
    DIR* d;
    struct dirent* entry;
    register int i = 0;
    size_t entrysize;

    if((d=opendir(dir)) == 0)
    {
	return -1;
    }
    
    *namelist = 0;
    while((entry=readdir(d)) != 0)
    {
	if(select == 0 || (select != 0 && (*select)(entry)))
	{
	    *namelist = (struct dirent**)realloc((void*)(*namelist), (size_t)((i + 1) * sizeof(struct dirent*)));
	    if(*namelist == 0)
	    {
		return -1;
	    }
	    
	    entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
	    (*namelist)[i] = (struct dirent*)malloc(entrysize);
	    if((*namelist)[i] == 0)
	    {
		return -1;
	    }
	    memcpy((*namelist)[i], entry, entrysize);
	    ++i;
	}
    }

    if(closedir(d))
    {
	return -1;
    }
    
    if(i == 0)
    {
	return -1;
    }

    if(compar != 0)
    {
	qsort((void *)(*namelist), (size_t)i, sizeof(struct dirent *), compar);
    }
    
    return i;
}

static int
alphasort(const void* v1, const void* v2)
{
    const struct dirent **a = (const struct dirent **)v1;
    const struct dirent **b = (const struct dirent **)v2;
    return(strcmp((*a)->d_name, (*b)->d_name));
}

#endif

using namespace std;
using namespace Ice;
using namespace IcePatch;

string
normalizePath(const string& path)
{
    string result = path;
    
    string::size_type pos;
    
    for(pos = 0; pos < result.size(); ++pos)
    {
	if(result[pos] == '\\')
	{
	    result[pos] = '/';
	}
    }
    
    pos = 0;
    while((pos = result.find("//", pos)) != string::npos)
    {
	result.erase(pos, 1);
    }
    
    pos = 0;
    while((pos = result.find("/./", pos)) != string::npos)
    {
	result.erase(pos, 2);
    }

    if(result.substr(0, 2) == "./")
    {
        result.erase(0, 2);
    }

    if(result.size() >= 2 && result.substr(result.size() - 2, 2) == "/.")
    {
        result.erase(result.size() - 2, 2);
    }

    return result;
}

string
IcePatch::identityToPath(const Identity& ident)
{
    assert(ident.category == "IcePatch");
    return normalizePath(ident.name);
}

Identity
IcePatch::pathToIdentity(const string& path)
{
    Identity ident;
    ident.category = "IcePatch";
    ident.name = normalizePath(path);
    return ident;
}

string
IcePatch::getSuffix(const string& path)
{
    string::size_type pos = path.rfind('.');
    if(pos == string::npos)
    {
	return string();
    }
    else
    {
	return path.substr(pos + 1);
    }
}

bool
IcePatch::ignoreSuffix(const string& path)
{
    string suffix = getSuffix(path);
    return suffix == "md5" || suffix == "md5temp" || suffix == "bz2" || suffix == "bz2temp" ||
           suffix == "tot" || suffix == "tottemp";
}

string
IcePatch::removeSuffix(const string& path)
{
    string::size_type pos = path.rfind('.');
    if(pos == string::npos)
    {
	return path;
    }
    else
    {
	return path.substr(0, pos);
    }
}

FileInfo
IcePatch::getFileInfo(const string& path, bool exceptionIfNotExist, const Ice::LoggerPtr& logger)
{
    FileInfo result;
    struct stat buf;

    while(true)
    {
	if(::stat(path.c_str(), &buf) == -1)
	{
	    if(!exceptionIfNotExist && errno == ENOENT)
	    {
		result.size = 0;
		result.time = 0;
		result.type = FileTypeNotExist;
		return result;
	    }
	    else
	    {
		FileAccessException ex;
		ex.reason = "cannot stat `" + path + "': " + strerror(errno);
		throw ex;
	    }
	}
	
	result.size = buf.st_size;
	result.time = buf.st_mtime;
	
	if(IceUtil::Time::seconds(result.time) <= IceUtil::Time::now())
	{
	    break;
	}

	//
	// Modification time is in the future.
	//
	
	if(logger)
	{
	    Trace out(logger, "IcePatch");
	    out << "modification time for `" << path << "' is in the future\n";
	    out << "setting modification time to the current time.";
	}

	//
	// First we remove .md5 and .bz2 files, which might be wrong
	// if the original file is in the future.
	//
	if(!ignoreSuffix(path))
	{
	    try
	    {
		removeRecursive(path + ".md5", logger);
	    }
	    catch(const FileAccessException&)
	    {
	    }
	    
	    try
	    {
		removeRecursive(path + ".bz2", logger);
	    }
	    catch(const FileAccessException&)
	    {
	    }
	}

	//
	// Now we reset the time of the file to the current time.
	//
	if(::utime(path.c_str(), 0) == -1)
	{
	    if(errno != ENOENT)
	    {
		FileAccessException ex;
		ex.reason = "cannot utime `" + path + "': " + strerror(errno);
		throw ex;
	    }
	}

	//
	// Now we do the loop again.
	//
    }

    if(S_ISDIR(buf.st_mode))
    {
	result.type = FileTypeDirectory;
    }
    else if(S_ISREG(buf.st_mode))
    {
	result.type = FileTypeRegular;
    }
    else
    {
	FileAccessException ex;
	ex.reason = "file type of `" + path + "' is not supported";
	throw ex;
    }
    
    return result;
}

void
IcePatch::removeRecursive(const string& path, const Ice::LoggerPtr& logger)
{
    if(getFileInfo(path, true, logger).type == FileTypeDirectory)
    {
	StringSeq paths = readDirectory(path);
        for(StringSeq::const_iterator p = paths.begin(); p != paths.end(); ++p)
	{
	    removeRecursive(*p, logger);
	}

#ifdef _WIN32
	if(_rmdir(path.c_str()) == -1)
#else
        if(rmdir(path.c_str()) == -1)
#endif
	{
	    FileAccessException ex;
	    ex.reason = "cannot remove directory `" + path + "': " + strerror(errno);
	    throw ex;
	}
    }
    else
    {
	if(::remove(path.c_str()) == -1)
	{
	    FileAccessException ex;
	    ex.reason = "cannot remove file `" + path + "': " + strerror(errno);
	    throw ex;
	}
    }

    if(logger)
    {
	Trace out(logger, "IcePatch");
	out << "removed file `" << path << "'";
    }
}

StringSeq
IcePatch::readDirectory(const string& path)
{
    return readDirectory(path, path);
}

StringSeq
IcePatch::readDirectory(const string& realPath, const string& prependPath)
{
#ifdef _WIN32

    struct _finddata_t data;
    long h = _findfirst((realPath + "/*").c_str(), &data);
    if(h == -1)
    {
	FileAccessException ex;
	ex.reason = "cannot read directory `" + realPath + "': " + strerror(errno);
	throw ex;
    }
    
    StringSeq result;

    while(true)
    {
	string name = data.name;
	assert(!name.empty());

	if(name != ".." && name != ".")
	{
	    result.push_back(normalizePath(prependPath + '/' + name));
	}

        if(_findnext(h, &data) == -1)
        {
            if(errno == ENOENT)
            {
                break;
            }

            FileAccessException ex;
            ex.reason = "cannot read directory `" + realPath + "': " + strerror(errno);
            _findclose(h);
            throw ex;
        }
    }

    _findclose(h);

    sort(result.begin(), result.end());

    return result;

#else

    struct dirent **namelist;
    int n = ::scandir(realPath.c_str(), &namelist, 0, alphasort);
    if(n < 0)
    {
	FileAccessException ex;
	ex.reason = "cannot read directory `" + realPath + "': " + strerror(errno);
	throw ex;
    }

    StringSeq result;
    result.reserve(n - 2);

    for(int i = 0; i < n; ++i)
    {
	string name = namelist[i]->d_name;
	assert(!name.empty());

	free(namelist[i]);

	if(name != ".." && name != ".")
	{
	    result.push_back(normalizePath(prependPath + '/' + name));
	}
    }
    
    free(namelist);
    return result;

#endif
}

void
IcePatch::createDirectory(const string& path)
{
#ifdef _WIN32
    if(::_mkdir(path.c_str()) == -1)
#else
    if(::mkdir(path.c_str(), 00777) == -1)
#endif
    {
	FileAccessException ex;
	ex.reason = "cannot create directory `" + path + "': " + strerror(errno);
	throw ex;
    }
}

ByteSeq
IcePatch::getMD5(const string& path)
{
    string pathMD5 = path + ".md5";
    ifstream fileMD5(pathMD5.c_str(), ios::binary);
    if(!fileMD5)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathMD5 + "' for reading: " + strerror(errno);
	throw ex;
    }

    ByteSeq bytesMD5;
    bytesMD5.resize(16);
    fileMD5.read(reinterpret_cast<char*>(&bytesMD5[0]), 16);
    if(!fileMD5)
    {
	FileAccessException ex;
	ex.reason = "cannot read `" + pathMD5 + "': " + strerror(errno);
	throw ex;
    }
    if(fileMD5.gcount() < 16)
    {
	FileAccessException ex;
	ex.reason = "could not read 16 bytes from `" + pathMD5 + "'";
	throw ex;
    }

    fileMD5.close();

    return bytesMD5;
}

void
IcePatch::putMD5(const string& path, const ByteSeq& bytesMD5)
{
    assert(bytesMD5.size() == 16);
    
    //
    // Save the MD5 hash value to a temporary MD5 file.
    //
    string pathMD5Temp = path + ".md5temp";
    ofstream fileMD5(pathMD5Temp.c_str(), ios::binary);
    if(!fileMD5)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathMD5Temp + "' for writing: " + strerror(errno);
	throw ex;
    }

    fileMD5.write(reinterpret_cast<const char*>(&bytesMD5[0]), 16);
    if(!fileMD5)
    {
	FileAccessException ex;
	ex.reason = "cannot write `" + pathMD5Temp + "': " + strerror(errno);
	throw ex;
    }

    fileMD5.close();
    
    //
    // Rename the temporary MD5 file to the final MD5 file. This is
    // done so that there can be no partial MD5 files after an
    // abortive application termination.
    //
    string pathMD5 = path + ".md5";
    ::remove(pathMD5.c_str());
    if(::rename(pathMD5Temp.c_str(), pathMD5.c_str()) == -1)
    {
	FileAccessException ex;
	ex.reason = "cannot rename `" + pathMD5Temp + "' to  `" + pathMD5 + "': " + strerror(errno);
	throw ex;
    }
}

void
IcePatch::createMD5(const string& path, const LoggerPtr& logger)
{
    //
    // The current directory is not permissible for MD5 value
    // creation.
    //
    assert(path != ".");

    //
    // Calculate and save the MD5 hash value.
    //
    ByteSeq bytesMD5 = calcMD5(path, logger);

    putMD5(path, bytesMD5);

    if(logger)
    {
	Trace out(logger, "IcePatch");
	out << "created MD5 file for `" << path << "'";
    }
}

ByteSeq
IcePatch::calcMD5(const string& path, const LoggerPtr& logger)
{
    ByteSeq bytes;

    FileInfo info = getFileInfo(path, true, logger);
    if(info.type == FileTypeDirectory)
    {
	//
	// Create a summary of all MD5 files.
	//
	StringSeq paths = readDirectory(path);
	for(StringSeq::const_iterator p = paths.begin(); p < paths.end(); ++p)
	{
	    if(getSuffix(*p) == "md5")
	    {
		string plainPath = removeSuffix(*p);

		//
		// We must take the filename into account, so that
		// renaming a file will change the summary MD5.
		//
                string plainFile = plainPath.substr(plainPath.rfind('/') + 1);
		copy(plainFile.begin(), plainFile.end(), back_inserter(bytes));

		ByteSeq subBytesMD5 = getMD5(plainPath);
		copy(subBytesMD5.begin(), subBytesMD5.end(), back_inserter(bytes));
	    }
	}	
    }
    else
    {
	assert(info.type == FileTypeRegular);

	//
	// Read the original file.
	//
	ifstream file(path.c_str(), ios::binary);
	if(!file)
	{
	    FileAccessException ex;
	    ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
	    throw ex;
	}
	
	bytes.resize(info.size);
	if(bytes.size() > 0)
	{
	    file.read(reinterpret_cast<char*>(&bytes[0]), bytes.size());
	    if(!file)
	    {
		FileAccessException ex;
		ex.reason = "cannot read `" + path + "': " + strerror(errno);
		throw ex;
	    }
	    if(file.gcount() < static_cast<int>(bytes.size()))
	    {
		FileAccessException ex;
		ex.reason = "could not read all bytes from `" + path + "'";
		throw ex;
	    }
	}

	file.close();
    }
    
    //
    // Create the MD5 hash value.
    //
    ByteSeq bytesMD5;
    bytesMD5.resize(16);
    if(bytes.size() > 0)
    {
	MD5(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(), reinterpret_cast<unsigned char*>(&bytesMD5[0]));
    }
    else
    {
	fill(bytesMD5.begin(), bytesMD5.end(), 0);
    }
    
    return bytesMD5;
}

ByteSeq
IcePatch::calcPartialMD5(const string& path, Int size, const LoggerPtr& logger)
{
    if(size < 0)
    {
	FileAccessException ex;
	ex.reason = "negative file size is illegal";
	throw ex;
    }
    
    FileInfo info = getFileInfo(path, true, logger);
    if(info.type == FileTypeDirectory)
    {
	FileAccessException ex;
	ex.reason = "cannot calculate partial MD5 for `" + path + "' because this is a directory";
	throw ex;
    }

    //
    // Read the original file partially.
    //
    size = std::min(size, static_cast<Int>(info.size));
    ifstream file(path.c_str(), ios::binary);
    if(!file)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
	throw ex;
    }

    ByteSeq bytes;
    bytes.resize(size);
    if(bytes.size() > 0)
    {
	file.read(reinterpret_cast<char*>(&bytes[0]), bytes.size());
	if(!file)
	{
	    FileAccessException ex;
	    ex.reason = "cannot read `" + path + "': " + strerror(errno);
	    throw ex;
	}
	if(file.gcount() < static_cast<int>(bytes.size()))
	{
	    FileAccessException ex;
	    ex.reason = "could not read all bytes from `" + path + "'";
	    throw ex;
	}
    }

    file.close();
    
    //
    // Create the MD5 hash value.
    //
    ByteSeq bytesMD5;
    bytesMD5.resize(16);
    if(bytes.size() > 0)
    {
	MD5(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(), reinterpret_cast<unsigned char*>(&bytesMD5[0]));
    }
    else
    {
	fill(bytesMD5.begin(), bytesMD5.end(), 0);
    }

    return bytesMD5;
}

ByteSeq
IcePatch::getBZ2(const string& path, Int pos, Int num)
{
    if(pos < 0)
    {
	FileAccessException ex;
	ex.reason = "negative read offset is illegal";
	throw ex;
    }

    if(num < 0)
    {
	FileAccessException ex;
	ex.reason = "negative data segment size is illegal";
	throw ex;
    }

    if(num > 1024 * 1024)
    {
	FileAccessException ex;
	ex.reason = "maximum data segment size exceeded";
	throw ex;
    }
    
    string pathBZ2 = path + ".bz2";
    ifstream fileBZ2(pathBZ2.c_str(), ios::binary);
    if(!fileBZ2)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathBZ2 + "' for reading: " + strerror(errno);
	throw ex;
    }

    fileBZ2.seekg(pos);
    if(!fileBZ2)
    {
	FileAccessException ex;
	ostringstream out;
	out << "cannot seek position " << pos << " in file `" << path << "':" << strerror(errno);
	ex.reason = out.str();
	throw ex;
    }

    ByteSeq bytesBZ2;
    bytesBZ2.resize(num);
    fileBZ2.read(reinterpret_cast<char*>(&bytesBZ2[0]), bytesBZ2.size());
    if(!fileBZ2 && !fileBZ2.eof())
    {
	FileAccessException ex;
	ex.reason = "cannot read `" + pathBZ2 + "': " + strerror(errno);
	throw ex;
    }

    bytesBZ2.resize(fileBZ2.gcount());

    fileBZ2.close();

    return bytesBZ2;
}

void
IcePatch::createBZ2(const string& path, const Ice::LoggerPtr& logger)
{
    FileInfo info = getFileInfo(path, true, logger);
    if(info.type == FileTypeDirectory)
    {
	FileAccessException ex;
	ex.reason = "cannot create BZ2 file for `" + path + "' because this is a directory";
	throw ex;
    }

    //
    // Read the original file in blocks and write a temporary BZ2
    // file.
    //
    ifstream file(path.c_str(), ios::binary);
    if(!file)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
	throw ex;
    }

    string pathBZ2Temp = path + ".bz2temp";
    FILE* stdioFileBZ2 = fopen(pathBZ2Temp.c_str(), "wb");
    if(!stdioFileBZ2)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + pathBZ2Temp + "' for writing: " + strerror(errno);
	throw ex;
    }
    
    int bzError;
    BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, stdioFileBZ2, 5, 0, 0);
    if(bzError != BZ_OK)
    {
	FileAccessException ex;
	ex.reason = "BZ2_bzWriteOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex.reason += string(": ") + strerror(errno);
	}
	fclose(stdioFileBZ2);
	throw ex;
    }
    
    const Int num = 64 * 1024;
    Byte bytes[num];
    
    while(!file.eof())
    {
	file.read(reinterpret_cast<char*>(&bytes[0]), num);
	if(!file && !file.eof())
	{
	    FileAccessException ex;
	    ex.reason = "cannot read `" + path + "': " + strerror(errno);
	    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
	    fclose(stdioFileBZ2);
	    throw ex;
	}
	
	if(file.gcount() > 0)
	{
	    BZ2_bzWrite(&bzError, bzFile, bytes, static_cast<int>(file.gcount()));
	    if(bzError != BZ_OK)
	    {
		FileAccessException ex;
		ex.reason = "BZ2_bzWrite failed";
		if(bzError == BZ_IO_ERROR)
		{
		    ex.reason += string(": ") + strerror(errno);
		}
		BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
		fclose(stdioFileBZ2);
		throw ex;
	    }
	}
    }
    
    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
    if(bzError != BZ_OK)
    {
	FileAccessException ex;
	ex.reason = "BZ2_bzWriteClose failed";
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
    // Rename the temporary BZ2 file to the final BZ2 file. This is
    // done so that there can be no partial BZ2 files after an
    // abortive application termination.
    //
    string pathBZ2 = path + ".bz2";
    ::remove(pathBZ2.c_str());
    if(::rename(pathBZ2Temp.c_str(), pathBZ2.c_str()) == -1)
    {
	FileAccessException ex;
	ex.reason = "cannot rename `" + pathBZ2Temp + "' to  `" + pathBZ2 + "': " + strerror(errno);
	throw ex;
    }

    if(logger)
    {
	Trace out(logger, "IcePatch");
	out << "created BZ2 file for `" << path << "'";
    }
}

IcePatch::TotalMap
IcePatch::getTotalMap(const CommunicatorPtr& communicator, const string& path)
{
    TotalMap result;

    string pathTot = path + ".tot";

    FileInfo infoTot = getFileInfo(pathTot, false);
    if(infoTot.type == FileTypeNotExist)
    {
        return result;
    }

    ifstream fileTot(pathTot.c_str(), ios::binary);
    if(!fileTot)
    {
        FileAccessException ex;
        ex.reason = "cannot open `" + pathTot + "' for reading: " + strerror(errno);
        throw ex;
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream is(instance.get());
    is.b.resize(static_cast<ByteSeq::size_type>(infoTot.size));
    fileTot.read(reinterpret_cast<char*>(&is.b[0]), is.b.size());

    if(static_cast<vector<Byte>::size_type>(fileTot.gcount()) < is.b.size())
    {
        FileAccessException ex;
        ex.reason = "could not read all bytes from `" + pathTot + "'";
        throw ex;
    }

    fileTot.close();

    is.i = is.b.begin();
    try
    {
        Int i, sz;
        is.readSize(sz);
        for(i = 0; i < sz; ++i)
        {
            ByteSeq key;
            Long value;
            is.read(key);
            is.read(value);
            result.insert(TotalMap::value_type(key, value));
        }
    }
    catch(const Ice::MarshalException&)
    {
        FileAccessException ex;
        ex.reason = "could not unmarshal contents of `" + pathTot + "'";
        throw ex;
    }

    return result;
}

void
IcePatch::putTotalMap(const CommunicatorPtr& communicator, const string& path, const TotalMap& tot)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream os(instance.get());

    //
    // Marshal the map.
    //
    os.writeSize(static_cast<Int>(tot.size()));
    for(TotalMap::const_iterator p = tot.begin(); p != tot.end(); ++p)
    {
        os.write(p->first);
        os.write(p->second);
    }

    //
    // Save the map to a temporary file.
    //
    string pathTotTemp = path + ".tottemp";
    ofstream fileTot(pathTotTemp.c_str(), ios::binary);
    if(!fileTot)
    {
        FileAccessException ex;
        ex.reason = "cannot open `" + pathTotTemp + "' for writing: " + strerror(errno);
        throw ex;
    }

    os.i = os.b.begin();
    size_t sz = os.b.end() - os.i;
    fileTot.write(reinterpret_cast<const char*>(&os.b[0]), sz);
    if(!fileTot)
    {
        FileAccessException ex;
        ex.reason = "cannot write `" + pathTotTemp + "': " + strerror(errno);
        throw ex;
    }

    fileTot.close();

    //
    // Rename the temporary file to the final file. This is
    // done so that there can be no partial files after an
    // abortive application termination.
    //
    string pathTot = path + ".tot";
    ::remove(pathTot.c_str());
    if(::rename(pathTotTemp.c_str(), pathTot.c_str()) == -1)
    {
        FileAccessException ex;
        ex.reason = "cannot rename `" + pathTotTemp + "' to  `" + pathTot + "': " + strerror(errno);
        throw ex;
    }
}

#ifdef _WIN32

bool
IcePatch::CICompare::operator()(const string& s1, const string& s2) const
{
    string::const_iterator p1 = s1.begin();
    string::const_iterator p2 = s2.begin();
    while(p1 != s1.end() && p2 != s2.end() && ::tolower(*p1) == ::tolower(*p2))
    {
        ++p1;
        ++p2;
    }
    if(p1 == s1.end() && p2 == s2.end())
    {
        return false;
    }
    else if(p1 == s1.end())
    {
        return true;
    }
    else if(p2 == s2.end())
    {
        return false;
    }
    else
    {
        return ::tolower(*p1) < ::tolower(*p2);
    }
}

#endif
