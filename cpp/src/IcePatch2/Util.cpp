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

#ifdef _WIN32
#   include <direct.h>
#   include <io.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
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
IcePatch2::bytesToString(const ByteSeq& bytes)
{
    ostringstream s;
    for(ByteSeq::const_iterator p = bytes.begin(); p != bytes.end(); ++p)
    {
	s << setw(2) << setfill('0') << hex << static_cast<int>(*p);
    }
    return s.str();
}

ByteSeq
IcePatch2::stringToBytes(const string& str)
{
    ByteSeq bytes;
    bytes.reserve((str.size() + 1) / 2);

    for(unsigned int i = 0; i + 1 < str.size(); i += 2)
    {
	istringstream is(str.substr(i, 2));

	int byte;
	is >> hex >> byte;

	bytes.push_back(static_cast<Byte>(byte));
    }

    return bytes;
}

string
IcePatch2::normalize(const string& path)
{
    string result = path;
    
    string::size_type pos;
    
#ifdef WIN32
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

    if(::remove(path.c_str()) == -1)
    {
	throw "cannot remove file `" + path + "': " + strerror(errno);
    }
}

void
IcePatch2::removeRecursive(const string& pa)
{
    const string path = normalize(pa);

    struct stat buf;
    if(stat(path.c_str(), &buf) == -1)
    {
	throw "cannot stat `" + path + "': " + strerror(errno);
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
	    throw "cannot remove directory `" + path + "': " + strerror(errno);
	}
    }
    else
    {
	remove(path);
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
	throw "cannot read directory `" + path + "': " + strerror(errno);
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

            string ex = "cannot read directory `" + path + "': " + strerror(errno);
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
	throw "cannot read directory `" + path + "': " + strerror(errno);
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
IcePatch2::createDirectoryRecursive(const string& pa)
{
    const string path = normalize(pa);

    string dir = getDirname(path);
    if(!dir.empty())
    {
	createDirectoryRecursive(dir);
    }

    if(mkdir(path.c_str(), 0777) == -1)
    {
	if(errno != EEXIST)
	{
	    throw "cannot create directory `" + path + "': " + strerror(errno);
	}
    }
}

void
IcePatch2::compressBytesToFile(const string& pa, const ByteSeq& bytes, Int pos)
{
    const string path = normalize(pa);

    FILE* stdioFile = fopen(path.c_str(), "wb");
    if(!stdioFile)
    {
	throw "cannot open `" + path + "' for writing: " + strerror(errno);
    }
    
    int bzError;
    BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, stdioFile, 5, 0, 0);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzWriteOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + strerror(errno);
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
	    ex += string(": ") + strerror(errno);
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
	    ex += string(": ") + strerror(errno);
	}
	fclose(stdioFile);
	throw ex;
    }
    
    fclose(stdioFile);
}

void
IcePatch2::uncompressFile(const string& pa)
{
    const string path = normalize(pa);
    const string pathBZ2 = path + ".bz2";

    ofstream file(path.c_str(), ios::binary);
    if(!file)
    {
	throw "cannot open `" + path + "' for writing: " + strerror(errno);
    }
    
    FILE* stdioFileBZ2 = fopen(pathBZ2.c_str(), "rb");
    if(!stdioFileBZ2)
    {
	throw "cannot open `" + pathBZ2 + "' for reading: " + strerror(errno);
    }
    
    int bzError;
    BZFILE* bzFile = BZ2_bzReadOpen(&bzError, stdioFileBZ2, 0, 0, 0, 0);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzReadOpen failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + strerror(errno);
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
		ex += string(": ") + strerror(errno);
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
		throw "cannot get read position for `" + pathBZ2 + "': " + strerror(errno);
	    }
	    
	    file.write(reinterpret_cast<char*>(bytesBZ2), sz);
	    if(!file)
	    {
		BZ2_bzReadClose(&bzError, bzFile);
		fclose(stdioFileBZ2);
		throw "cannot write `" + path + "': " + strerror(errno);
	    }
	}
    }

    BZ2_bzReadClose(&bzError, bzFile);
    if(bzError != BZ_OK)
    {
	string ex = "BZ2_bzReadClose failed";
	if(bzError == BZ_IO_ERROR)
	{
	    ex += string(": ") + strerror(errno);
	}
	fclose(stdioFileBZ2);
	throw ex;
    }

    fclose(stdioFileBZ2);
}

void
IcePatch2::getFileInfoSeq(const string& pa, FileInfoSeq& infoSeq, bool size, bool compress, bool verbose)
{
    const string path = normalize(pa);

    if(ignoreSuffix(path))
    {
	string pathWithoutSuffix = getWithoutSuffix(path);

	if(ignoreSuffix(pathWithoutSuffix))
	{
	    if(verbose)
	    {
		cout << path << ": removing " << getSuffix(path) << " file for "
		     << getSuffix(pathWithoutSuffix) << " file" << endl;
	    }

	    remove(path);
	}
	else
	{
	    struct stat buf;
	    if(stat(getWithoutSuffix(path).c_str(), &buf) == -1)
	    {
		if(errno == ENOENT)
		{
		    if(verbose)
		    {
			cout << path << ": removing orphaned " << getSuffix(path) << " file" << endl;
		    }
		    remove(path);
		}
		else
		{
		    throw "cannot stat `" + path + "': " + strerror(errno);
		}
	    }
	    else if(buf.st_size == 0)
	    {
		if(verbose)
		{
		    cout << path << ": removing " << getSuffix(path) << " file for empty file" << endl;
		}
		remove(path);
	    }
	}

	return;
    }

    struct stat buf;
    if(stat(path.c_str(), &buf) == -1)
    {
	throw "cannot stat `" + path + "': " + strerror(errno);
    }
    
    if(S_ISDIR(buf.st_mode))
    {
	FileInfo info;
	info.path = path;
	info.size = -1;

	ByteSeq bytes(path.size());
	copy(path.begin(), path.end(), bytes.begin());

	ByteSeq bytesSHA(20);
	SHA1(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(),
	     reinterpret_cast<unsigned char*>(&bytesSHA[0]));
	info.checksum.swap(bytesSHA);

	infoSeq.push_back(info);
	
	StringSeq content = readDirectory(path);
	for(StringSeq::const_iterator p = content.begin(); p != content.end() ; ++p)
	{
	    getFileInfoSeq(path + '/' + *p, infoSeq, size, compress, verbose);
	}
    }
    else if(S_ISREG(buf.st_mode))
    {
	FileInfo info;
	info.path = path;
	info.size = 0;

	ByteSeq bytes(path.size() + buf.st_size);
	copy(path.begin(), path.end(), bytes.begin());

	if(buf.st_size != 0)
	{
	    if(verbose)
	    {
		if(compress)
		{
		    cout << path << ": calculating checksum and compressing file" << endl;
		}
		else
		{
		    cout << path << ": calculating checksum" << endl;
		}
	    }
	    
	    int fd = open(path.c_str(), O_RDONLY);
	    if(fd == -1)
	    {
		throw "cannot open `" + path + "' for reading: " + strerror(errno);
	    }

	    if(read(fd, &bytes[path.size()], buf.st_size) == -1)
	    {
		close(fd);
		throw "cannot read `" + path + "': " + strerror(errno);
	    }
	    
	    close(fd);
	    
	    if(compress)
	    {
		string pathBZ2 = path + ".bz2";
		compressBytesToFile(pathBZ2, bytes, path.size());
	    }

	    if(size)
	    {
		string pathBZ2 = path + ".bz2";

		struct stat bufBZ2;
		if(stat(pathBZ2.c_str(), &bufBZ2) == -1)
		{
		    throw "cannot stat `" + pathBZ2 + "': " + strerror(errno);
		}

		info.size = bufBZ2.st_size;
	    }
	}
	else
	{
	    if(verbose)
	    {
		cout << path << ": calculating checksum for empty file" << endl;
	    }
	}
	
	ByteSeq bytesSHA(20);
	SHA1(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(),
	     reinterpret_cast<unsigned char*>(&bytesSHA[0]));
	info.checksum.swap(bytesSHA);

	infoSeq.push_back(info);
    }
    else
    {
	if(verbose)
	{
	    cout << path << ": ignoring unknown file type" << endl;
	}
    }
}

void
IcePatch2::saveFileInfoSeq(const string& path, const FileInfoSeq& infoSeq)
{
    ofstream os(path.c_str());
    if(!os)
    {
	throw "cannot open `" + path + "' for writing: " + strerror(errno);
    }
    
    for(FileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
    {
	os << *p << '\n';
    }
}

void
IcePatch2::loadFileInfoSeq(const string& path, FileInfoSeq& infoSeq)
{
    ifstream is(path.c_str());
    if(!is)
    {
	throw "cannot open `" + path + "' for reading: " + strerror(errno);
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
}

void
IcePatch2::getFileTree1(const FileInfoSeq& infoSeq, FileTree1& tree1)
{
    if(infoSeq.empty())
    {
	tree1.files.clear();
	tree1.checksum.resize(20, 0);
    }
    else
    {
	tree1.files.reserve(infoSeq.size());
	
	FileInfoSeq::const_iterator p = infoSeq.begin();
	
	ByteSeq allChecksums;
	allChecksums.resize(infoSeq.size() * 20);
	ByteSeq::iterator q = allChecksums.begin();
	
	while(p < infoSeq.end())
	{
	    tree1.files.push_back(*p);
	    
	    assert(p->checksum.size() == 20);
	    copy(p->checksum.begin(), p->checksum.end(), q);
	    
	    ++p;
	    q += 20;
	}

	sort(tree1.files.begin(), tree1.files.end(), FileInfoCompare());
	
	tree1.checksum.resize(20);
	SHA1(reinterpret_cast<unsigned char*>(&allChecksums[0]), allChecksums.size(),
	     reinterpret_cast<unsigned char*>(&tree1.checksum[0]));
    }
}

void
IcePatch2::getFileTree0(const FileInfoSeq& infoSeq, FileTree0& tree0)
{
    tree0.nodes.resize(256);

    ByteSeq allChecksums;
    allChecksums.resize(256 * 20);
    ByteSeq::iterator q = allChecksums.begin();

    for(int i = 0; i < 256; ++i)
    {
	FileInfoSeq infoSeq1;

	for(FileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
	{
	    if(i == static_cast<int>(p->checksum[20 - 2]))
	    {
		infoSeq1.push_back(*p);
	    }
	}

	getFileTree1(infoSeq1, tree0.nodes[i]);

	assert(tree0.nodes[i].checksum.size() == 20);
	copy(tree0.nodes[i].checksum.begin(), tree0.nodes[i].checksum.end(), q);

	q += 20;
    }

    tree0.checksum.resize(20);
    SHA1(reinterpret_cast<unsigned char*>(&allChecksums[0]), allChecksums.size(),
	 reinterpret_cast<unsigned char*>(&tree0.checksum[0]));
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
