// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IcePatch2/Util.h>
#include <openssl/sha.h>
#include <bzlib.h>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef _WIN32
#   include <direct.h>
#   include <io.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <unistd.h>
#   include <dirent.h>
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

    if((d = opendir(dir)) == 0)
    {
	return -1;
    }

    *namelist = 0;
    while((entry = readdir(d)) != 0)
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
using namespace IcePatch2;

string
IcePatch2::lastError()
{
#ifdef WIN32
    LPVOID lpMsgBuf = 0;
    DWORD ok = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			     FORMAT_MESSAGE_FROM_SYSTEM |
			     FORMAT_MESSAGE_IGNORE_INSERTS,
			     NULL,
			     GetLastError(),
			     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			     (LPTSTR)&lpMsgBuf,
			     0,
			     NULL);
    if(ok)
    {
	LPCTSTR msg = (LPCTSTR)lpMsgBuf;
	assert(msg && strlen((const char*)msg) > 0);
	string result = (const char*)msg;
	LocalFree(lpMsgBuf);

	while(result.size() >= 1 && (result[result.size() - 1] == '\n' || result[result.size() - 1] == '\r'))
	{
	    result.erase(result.size() - 1);
	}

	return result;
    }
    else
    {
	return "unknown error";
    }
#else
    return strerror(errno);
#endif
}

string
IcePatch2::bytesToString(const ByteSeq& bytes)
{
/*
    ostringstream s;

    for(ByteSeq::const_iterator p = bytes.begin(); p != bytes.end(); ++p)
    {
	s << setw(2) << setfill('0') << hex << static_cast<int>(*p);
    }

    return s.str();
*/

    static const char* toHex = "0123456789abcdef";

    string s;
    s.resize(bytes.size() * 2);

    for(unsigned int i = 0; i < bytes.size(); ++i)
    {
	s[i * 2] = toHex[(bytes[i] >> 4) & 0xf];
	s[i * 2 + 1] = toHex[bytes[i] & 0xf];
    }

    return s;
}

ByteSeq
IcePatch2::stringToBytes(const string& str)
{
    ByteSeq bytes;
    bytes.reserve((str.size() + 1) / 2);

    for(unsigned int i = 0; i + 1 < str.size(); i += 2)
    {
/*
	istringstream is(str.substr(i, 2));
	int byte;
	is >> hex >> byte;
*/

	int byte = 0;

	for(int j = 0; j < 2; ++j)
	{
	    char c = str[i + j];

	    if(c >= '0' && c <= '9')
	    {
		byte |= c - '0';
	    }
	    else if(c >= 'a' && c <= 'f')
	    {
		byte |= 10 + c - 'a';
	    }
	    else if(c >= 'A' && c <= 'F')
	    {
		byte |= 10 + c - 'A';
	    }

	    if(j == 0)
	    {
		byte <<= 4;
	    }
	}

	bytes.push_back(static_cast<Byte>(byte));
    }

    return bytes;
}

string
IcePatch2::normalize(const string& path)
{
    string result = path;

    string::size_type pos;

#ifdef _WIN32
    for(pos = 0; pos < result.size(); ++pos)
    {
	if(result[pos] == '\\')
	{
	    result[pos] = '/';
	}
    }
#endif

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

    if(result.size() >= 1 && result[result.size() - 1] == '/')
    {
	result.erase(result.size() - 1);
    }

    return result;
}

string
IcePatch2::getSuffix(const string& pa)
{
    const string path = normalize(pa);

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

string
IcePatch2::getWithoutSuffix(const string& pa)
{
    const string path = normalize(pa);

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

bool
IcePatch2::ignoreSuffix(const string& path)
{
    string suffix = getSuffix(path);
    return suffix == "md5" // For legacy IcePatch
	|| suffix == "tot" // For legacy IcePatch
	|| suffix == "bz2";
}

string
IcePatch2::getBasename(const string& pa)
{
    const string path = normalize(pa);

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

string
IcePatch2::getDirname(const string& pa)
{
    const string path = normalize(pa);

    string::size_type pos = path.rfind('/');
    if(pos == string::npos)
    {
	return string();
    }
    else
    {
	return path.substr(0, pos);
    }
}

void
IcePatch2::remove(const string& pa)
{
    const string path = normalize(pa);

    struct stat buf;
    if(stat(path.c_str(), &buf) == -1)
    {
	throw "cannot stat `" + path + "':\n" + lastError();
    }

    if(S_ISDIR(buf.st_mode))
    {
#ifdef _WIN32
	if(_rmdir(path.c_str()) == -1)
#else
	if(rmdir(path.c_str()) == -1)
#endif
	{
	    throw "cannot remove directory `" + path + "':\n" + lastError();
	}
    }
    else
    {
	if(::remove(path.c_str()) == -1)
	{
	    throw "cannot remove file `" + path + "':\n" + lastError();
	}
    }
}

void
IcePatch2::removeRecursive(const string& pa)
{
    const string path = normalize(pa);

    struct stat buf;
    if(stat(path.c_str(), &buf) == -1)
    {
	throw "cannot stat `" + path + "':\n" + lastError();
    }

    if(S_ISDIR(buf.st_mode))
    {
	StringSeq paths = readDirectory(path);
	for(StringSeq::const_iterator p = paths.begin(); p != paths.end(); ++p)
	{
	    removeRecursive(path + '/' + *p);
	}

#ifdef _WIN32
	if(_rmdir(path.c_str()) == -1)
#else
	if(rmdir(path.c_str()) == -1)
#endif
	{
	    throw "cannot remove directory `" + path + "':\n" + lastError();
	}
    }
    else
    {
	if(::remove(path.c_str()) == -1)
	{
	    throw "cannot remove file `" + path + "':\n" + lastError();
	}
    }
}

StringSeq
IcePatch2::readDirectory(const string& pa)
{
    const string path = normalize(pa);

#ifdef _WIN32

    struct _finddata_t data;
    long h = _findfirst((path + "/*").c_str(), &data);
    if(h == -1)
    {
	throw "cannot read directory `" + path + "':\n" + lastError();
    }

    StringSeq result;

    while(true)
    {
	string name = data.name;
	assert(!name.empty());

	if(name != ".." && name != ".")
	{
	    result.push_back(name);
	}

	if(_findnext(h, &data) == -1)
	{
	    if(errno == ENOENT)
	    {
		break;
	    }

	    string ex = "cannot read directory `" + path + "':\n" + lastError();
	    _findclose(h);
	    throw ex;
	}
    }

    _findclose(h);

    sort(result.begin(), result.end());

    return result;

#else

    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
    if(n < 0)
    {
	throw "cannot read directory `" + path + "':\n" + lastError();
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
	    result.push_back(name);
	}
    }

    free(namelist);
    return result;

#endif
}

void
IcePatch2::createDirectory(const string& pa)
{
    const string path = normalize(pa);

#ifdef _WIN32
    if(_mkdir(path.c_str()) == -1)
#else
    if(mkdir(path.c_str(), 0777) == -1)
#endif
    {
	if(errno != EEXIST)
	{
	    throw "cannot create directory `" + path + "':\n" + lastError();
	}
    }
}

void
IcePatch2::createDirectoryRecursive(const string& pa)
{
    const string path = normalize(pa);

    string dir = getDirname(path);
    if(!dir.empty())
    {
	createDirectoryRecursive(dir);
    }

#ifdef _WIN32
    if(_mkdir(path.c_str()) == -1)
#else
    if(mkdir(path.c_str(), 0777) == -1)
#endif
    {
	if(errno != EEXIST)
	{
	    throw "cannot create directory `" + path + "':\n" + lastError();
	}
    }
}

int
IcePatch2::compressBytesToFile(const string& pa, const ByteSeq& bytes, Int pos, int compress)
{
    const string path = normalize(pa);

    FILE* stdioFile = fopen(path.c_str(), "wb");
    if(!stdioFile)
    {
	throw "cannot open `" + path + "' for writing:\n" + lastError();
    }

    int bzError;
    BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, stdioFile, 5, 0, 0);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzWriteOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + lastError();
	}
	fclose(stdioFile);
	throw ex;
    }

    BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[pos]), bytes.size() - pos);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzWrite failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + lastError();
	}
	BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
	fclose(stdioFile);
	throw ex;
    }

    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzWriteClose failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + lastError();
	}
	fclose(stdioFile);
	throw ex;
    }

    fclose(stdioFile);

    struct stat buf;
    if(stat(path.c_str(), &buf) == -1)
    {
	throw "cannot stat `" + path + "':\n" + lastError();
    }
    if(compress == 1) // Use compressed file only if it is smaller.
    {
	if(static_cast<ByteSeq::size_type>(buf.st_size) >= bytes.size() - pos)
	{
	    remove(path);
	    return 0;
	}
    }
    return buf.st_size;
}

void
IcePatch2::decompressFile(const string& pa)
{
    const string path = normalize(pa);
    const string pathBZ2 = path + ".bz2";

    ofstream file(path.c_str(), ios::binary);
    if(!file)
    {
	throw "cannot open `" + path + "' for writing:\n" + lastError();
    }

    FILE* stdioFileBZ2 = fopen(pathBZ2.c_str(), "rb");
    if(!stdioFileBZ2)
    {
	throw "cannot open `" + pathBZ2 + "' for reading:\n" + lastError();
    }

    int bzError;
    BZFILE* bzFile = BZ2_bzReadOpen(&bzError, stdioFileBZ2, 0, 0, 0, 0);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzReadOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + lastError();
	}
	fclose(stdioFileBZ2);
	throw ex;
    }

    const Int numBZ2 = 64 * 1024;
    Byte bytesBZ2[numBZ2];

    while(bzError != BZ_STREAM_END)
    {
	int sz = BZ2_bzRead(&bzError, bzFile, bytesBZ2, numBZ2);
	if(bzError != BZ_OK && bzError != BZ_STREAM_END)
	{
	    string ex = "BZ2_bzRead failed";
	    if(bzError == BZ_IO_ERROR)
	    {
		ex += string(": ") + lastError();
	    }
	    BZ2_bzReadClose(&bzError, bzFile);
	    fclose(stdioFileBZ2);
	    throw ex;
	}

	if(sz > 0)
	{
	    long pos = ftell(stdioFileBZ2);
	    if(pos == -1)
	    {
		BZ2_bzReadClose(&bzError, bzFile);
		fclose(stdioFileBZ2);
		throw "cannot get read position for `" + pathBZ2 + "':\n" + lastError();
	    }

	    file.write(reinterpret_cast<char*>(bytesBZ2), sz);
	    if(!file)
	    {
		BZ2_bzReadClose(&bzError, bzFile);
		fclose(stdioFileBZ2);
		throw "cannot write to `" + path + "':\n" + lastError();
	    }
	}
    }

    BZ2_bzReadClose(&bzError, bzFile);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzReadClose failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + lastError();
	}
	fclose(stdioFileBZ2);
	throw ex;
    }

    fclose(stdioFileBZ2);
}

static bool
getFileInfoSeqInt(const string& basePath, const string& relPath, int compress, GetFileInfoSeqCB* cb,
		  FileInfoSeq& infoSeq)
{
    const string path = basePath + '/' + relPath;

    if(ignoreSuffix(path))
    {
	string pathWithoutSuffix = getWithoutSuffix(path);

	if(ignoreSuffix(pathWithoutSuffix))
	{
	    if(cb && !cb->remove(relPath))
	    {
		return false;
	    }

	    remove(path); // Removing file with suffix for another file that already has a suffix.
	}
	else
	{
	    struct stat buf;
	    if(stat(getWithoutSuffix(path).c_str(), &buf) == -1)
	    {
		if(errno == ENOENT)
		{
		    if(cb && !cb->remove(relPath))
		    {
			return false;
		    }

		    remove(path); // Removing orphaned file.
		}
		else
		{
		    throw "cannot stat `" + path + "':\n" + lastError();
		}
	    }
	    else if(buf.st_size == 0)
	    {
		if(cb && !cb->remove(relPath))
		{
		    return false;
		}

		remove(path); // Removing file with suffix for empty file.
	    }
	}
    }
    else
    {
	struct stat buf;
	if(stat(path.c_str(), &buf) == -1)
	{
	    throw "cannot stat `" + path + "':\n" + lastError();
	}

	if(S_ISDIR(buf.st_mode))
	{
	    FileInfo info;
	    info.path = relPath;
	    info.size = -1;

	    ByteSeq bytes(relPath.size());
	    copy(relPath.begin(), relPath.end(), bytes.begin());

	    ByteSeq bytesSHA(20);
	    if(!bytes.empty())
	    {
		SHA1(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(),
		     reinterpret_cast<unsigned char*>(&bytesSHA[0]));
	    }
	    else
	    {
		fill(bytesSHA.begin(), bytesSHA.end(), 0);
	    }
	    info.checksum.swap(bytesSHA);

	    infoSeq.push_back(info);

	    StringSeq content = readDirectory(path);
	    for(StringSeq::const_iterator p = content.begin(); p != content.end() ; ++p)
	    {
		if(!getFileInfoSeqInt(basePath, normalize(relPath + '/' + *p), compress, cb, infoSeq))
		{
		    return false;
		}
	    }
	}
	else if(S_ISREG(buf.st_mode))
	{
	    FileInfo info;
	    info.path = relPath;
	    info.size = 0;

	    ByteSeq bytes(relPath.size() + buf.st_size);
	    copy(relPath.begin(), relPath.end(), bytes.begin());

	    if(buf.st_size != 0)
	    {
#ifdef _WIN32
		int fd = open(path.c_str(), _O_RDONLY | _O_BINARY);
#else
		int fd = open(path.c_str(), O_RDONLY);
#endif
		if(fd == -1)
		{
		    throw "cannot open `" + path + "' for reading:\n" + lastError();
		}

		if(read(fd, &bytes[relPath.size()], buf.st_size) == -1)
		{
		    close(fd);
		    throw "cannot read from `" + path + "':\n" + lastError();
		}

		close(fd);

		//
		// compress == 0: Never compress.
		// compress == 1: Compress if necessary.
		// compress >= 2: Always compress.
		//
		cerr << "compress = " << compress << endl;
		if(compress > 0)
		{
		    if(compress >= 2 && cb && !cb->compress(relPath))
		    {
			return false;
		    }

		    string pathBZ2 = path + ".bz2";
		    struct stat bufBZ2;
		    if(compress >= 2 || stat(pathBZ2.c_str(), &bufBZ2) == -1 || buf.st_mtime >= bufBZ2.st_mtime)
		    {
			info.size = compressBytesToFile(pathBZ2, bytes, relPath.size(), compress);
		    }
		}
	    }

	    if(cb && !cb->checksum(relPath))
	    {
		return false;
	    }

	    ByteSeq bytesSHA(20);
	    if(!bytes.empty())
	    {
		SHA1(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(),
		     reinterpret_cast<unsigned char*>(&bytesSHA[0]));
	    }
	    else
	    {
		fill(bytesSHA.begin(), bytesSHA.end(), 0);
	    }
	    info.checksum.swap(bytesSHA);

	    infoSeq.push_back(info);
	}
    }

    return true;
}

bool
IcePatch2::getFileInfoSeq(const string& basePath, int compress, GetFileInfoSeqCB* cb,
			  FileInfoSeq& infoSeq)
{
    return getFileInfoSeqSubDir(basePath, ".", compress, cb, infoSeq);
}

bool
IcePatch2::getFileInfoSeqSubDir(const string& basePa, const string& relPa, int compress, GetFileInfoSeqCB* cb,
				FileInfoSeq& infoSeq)
{
    const string basePath = normalize(basePa);
    const string relPath = normalize(relPa);

    if(!getFileInfoSeqInt(basePath, relPath, compress, cb, infoSeq))
    {
	return false;
    }

    sort(infoSeq.begin(), infoSeq.end(), FileInfoLess());
    infoSeq.erase(unique(infoSeq.begin(), infoSeq.end(), FileInfoEqual()), infoSeq.end());

    return true;
}

void
IcePatch2::saveFileInfoSeq(const string& pa, const FileInfoSeq& infoSeq)
{
    {
	const string path = normalize(pa + ".sum");
	
	ofstream os(path.c_str());
	if(!os)
	{
	    throw "cannot open `" + path + "' for writing:\n" + lastError();
	}
	
	for(FileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
	{
	    os << *p << '\n';
	}
    }

    {
	const string pathLog = normalize(pa + ".log");

	try
	{
	    remove(pathLog);
	}
	catch(...)
	{
	}
    }
}

void
IcePatch2::loadFileInfoSeq(const string& pa, FileInfoSeq& infoSeq)
{
    {
	const string path = normalize(pa + ".sum");

	ifstream is(path.c_str());
	if(!is)
	{
	    throw "cannot open `" + path + "' for reading:\n" + lastError();
	}

	while(is.good())
	{
	    FileInfo info;
	    is >> info;

	    if(is.good())
	    {
		infoSeq.push_back(info);
	    }
	}

	sort(infoSeq.begin(), infoSeq.end(), FileInfoLess());
	infoSeq.erase(unique(infoSeq.begin(), infoSeq.end(), FileInfoEqual()), infoSeq.end());
    }

    {
	const string pathLog = normalize(pa + ".log");

	ifstream is(pathLog.c_str());
	if(is)
	{
	    FileInfoSeq remove;
	    FileInfoSeq update;
	    
	    while(is.good())
	    {
		char c;
		is >> c;

		FileInfo info;
		is >> info;

		if(is.good())
		{
		    if(c == '-')
		    {
			remove.push_back(info);
		    }
		    else if(c == '+')
		    {
			update.push_back(info);
		    }
		}
	    }

	    sort(remove.begin(), remove.end(), FileInfoLess());
	    remove.erase(unique(remove.begin(), remove.end(), FileInfoEqual()), remove.end());

	    sort(update.begin(), update.end(), FileInfoLess());
	    update.erase(unique(update.begin(), update.end(), FileInfoEqual()), update.end());

	    FileInfoSeq newInfoSeq;
	    newInfoSeq.reserve(infoSeq.size());
	    
	    set_difference(infoSeq.begin(),
			   infoSeq.end(),
			   remove.begin(),
			   remove.end(),
			   back_inserter(newInfoSeq),
			   FileInfoLess());
	    
	    infoSeq.swap(newInfoSeq);
	    
	    newInfoSeq.clear();
	    newInfoSeq.reserve(infoSeq.size());

	    set_union(infoSeq.begin(),
		      infoSeq.end(),
		      update.begin(),
		      update.end(),
		      back_inserter(newInfoSeq),
		      FileInfoLess());
	    
	    infoSeq.swap(newInfoSeq);

	    saveFileInfoSeq(pa, infoSeq);
	}
    }
}

void
IcePatch2::getFileTree0(const FileInfoSeq& infoSeq, FileTree0& tree0)
{
    tree0.nodes.resize(256);
    tree0.checksum.resize(20);

    ByteSeq allChecksums0;
    allChecksums0.resize(256 * 20);
    ByteSeq::iterator c0 = allChecksums0.begin();

    for(int i = 0; i < 256; ++i, c0 += 20)
    {
	FileTree1& tree1 = tree0.nodes[i];

	tree1.files.clear();
	tree1.checksum.resize(20);
	
	FileInfoSeq::const_iterator p;
	
	for(p = infoSeq.begin(); p != infoSeq.end(); ++p)
	{
	    if(i == static_cast<int>(p->checksum[0]))
	    {
		tree1.files.push_back(*p);
	    }
	}

	ByteSeq allChecksums1;
	allChecksums1.resize(tree1.files.size() * 20);
	ByteSeq::iterator c1 = allChecksums1.begin();
	
	for(p = tree1.files.begin(); p != tree1.files.end(); ++p, c1 += 20)
	{
	    copy(p->checksum.begin(), p->checksum.end(), c1);
	}
	
	if(!allChecksums1.empty())
	{
	    SHA1(reinterpret_cast<unsigned char*>(&allChecksums1[0]), allChecksums1.size(),
		 reinterpret_cast<unsigned char*>(&tree1.checksum[0]));
	}
	else
	{
	    fill(tree1.checksum.begin(), tree1.checksum.end(), 0);
	}

	copy(tree1.checksum.begin(), tree1.checksum.end(), c0);
    }

    if(!allChecksums0.empty())
    {
	SHA1(reinterpret_cast<unsigned char*>(&allChecksums0[0]), allChecksums0.size(),
	     reinterpret_cast<unsigned char*>(&tree0.checksum[0]));
    }
    else
    {
	fill(tree0.checksum.begin(), tree0.checksum.end(), 0);
    }
}

ostream&
IcePatch2::operator<<(ostream& os, const FileInfo& info)
{
    os << IceUtil::escapeString(info.path, "") << '\t' << bytesToString(info.checksum) << '\t' << info.size;
    return os;
}

istream&
IcePatch2::operator>>(istream& is, FileInfo& info)
{
    string s;

    getline(is, s, '\t');
    IceUtil::unescapeString(s, 0, s.size(), info.path);

    getline(is, s, '\t');
    info.checksum = stringToBytes(s);

    getline(is, s, '\n');
    info.size = atoi(s.c_str());

    return is;
}
