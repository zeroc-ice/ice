// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// We need to include io.h first to get the proper signature for
// _wfindfirst
//
#ifdef _WIN32
#   include <io.h>
#endif

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/IceUtil.h>
#include <IcePatch2/Util.h>
#include <openssl/sha.h>
#include <bzlib.h>
#include <iomanip>
#include <OS.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

#ifdef __BCPLUSPLUS__
#   include <iterator>
#endif

const char* IcePatch2::checksumFile = "IcePatch2.sum";
const char* IcePatch2::logFile = "IcePatch2.log";

//
// Solaris 9 and before doesn't have scandir() or alphasort().
//
#ifdef __sun

extern "C" static int
ice_scandir(const char* dir, struct dirent*** namelist,
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
                closedir(d);
                return -1;
            }

            entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
            (*namelist)[i] = (struct dirent*)malloc(entrysize);
            if((*namelist)[i] == 0)
            {
                closedir(d);
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

extern "C" static int
ice_alphasort(const void* v1, const void* v2)
{
    const struct dirent **a = (const struct dirent **)v1;
    const struct dirent **b = (const struct dirent **)v2;
    return(strcmp((*a)->d_name, (*b)->d_name));
}

#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

bool
IcePatch2::writeFileInfo(FILE* fp, const FileInfo& info)
{
    int rc = fprintf(fp, "%s\t%s\t%d\t%d\n", 
                     IceUtil::escapeString(info.path, "").c_str(),
                     bytesToString(info.checksum).c_str(),
                     info.size,
                     static_cast<int>(info.executable));
    return rc > 0;
}

bool
IcePatch2::readFileInfo(FILE* fp, FileInfo& info)
{
    string data;
    char buf[BUFSIZ];
    while(fgets(buf, static_cast<int>(sizeof(buf)), fp) != 0)
    {
        data += buf;

        size_t len = strlen(buf);
        if(buf[len - 1] == '\n')
        {
            break;
        }
    }
    if(data.empty())
    {
        return false;
    }

    istringstream is(data);

    string s;
    getline(is, s, '\t');
    IceUtil::unescapeString(s, 0, s.size(), info.path);

    getline(is, s, '\t');
    info.checksum = stringToBytes(s);

    is >> info.size;
    is >> info.executable;

    return true;
}

string
IcePatch2::lastError()
{
#ifdef _WIN32
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
IcePatch2::simplify(const string& path)
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

    while(result.substr(0, 4) == "/../")
    {
        result.erase(0, 3);
    }

    if(result.substr(0, 2) == "./")
    {
        result.erase(0, 2);
    }

    if(result == "/." ||
       result.size() == 4 && isalpha(result[0]) && result[1] == ':' && result[2] == '/' && result[3] == '.')
    {
       return result.substr(0, result.size() - 1);
    }

    if(result.size() >= 2 && result.substr(result.size() - 2, 2) == "/.")
    {
        result.erase(result.size() - 2, 2);
    }

    if(result == "/" || result.size() == 3 && isalpha(result[0]) && result[1] == ':' && result[2] == '/')
    {
        return result;
    }

    if(result.size() >= 1 && result[result.size() - 1] == '/')
    {
        result.erase(result.size() - 1);
    }

    if(result == "/..")
    {
        result = "/";
    }

    return result;
}

bool
IcePatch2::isAbsolute(const string& pa)
{
    if(pa.empty())
    {
        return false;
    }

    unsigned i = 0;
    while(isspace(pa[i]))
    {
        ++i;
    }
#ifdef _WIN32
    return pa[i] == '\\' || pa[i] == '/' || pa.size() > i + 1 && isalpha(pa[i]) && pa[i + 1] == ':';
#else
    return pa[i] == '/';
#endif
}

bool
IcePatch2::isRoot(const string& pa)
{
    string path = simplify(pa);
#ifdef _WIN32
    return path == "/" || path.size() == 3 && isalpha(path[0]) && path[1] == ':' && path[2] == '/';
#else
    return path == "/";
#endif
}

string
IcePatch2::getSuffix(const string& pa)
{
    const string path = simplify(pa);

    string::size_type dotPos = path.rfind('.');
    string::size_type slashPos = path.rfind('/');

    if(dotPos == string::npos || slashPos != string::npos && slashPos > dotPos)
    {
        return string();
    }

    return path.substr(dotPos + 1);
}

string
IcePatch2::getWithoutSuffix(const string& pa)
{
    const string path = simplify(pa);

    string::size_type dotPos = path.rfind('.');
    string::size_type slashPos = path.rfind('/');

    if(dotPos == string::npos || slashPos != string::npos && slashPos > dotPos)
    {
        return path;
    }

    return path.substr(0, dotPos);
}

bool
IcePatch2::ignoreSuffix(const string& path)
{
    string suffix = getSuffix(path);
    return suffix == "md5" // For legacy IcePatch.
        || suffix == "tot" // For legacy IcePatch.
        || suffix == "bz2"
        || suffix == "bz2temp";
}

string
IcePatch2::getBasename(const string& pa)
{
    const string path = simplify(pa);

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
    const string path = simplify(pa);

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
IcePatch2::rename(const string& fromPa, const string& toPa)
{

    const string fromPath = simplify(fromPa);
    const string toPath = simplify(toPa);

    OS::remove(toPath); // We ignore errors, as the file we are renaming to might not exist.

    if(OS::rename(fromPath ,toPath) == -1)
    {
        throw "cannot rename `" + fromPath + "' to  `" + toPath + "': " + lastError();
    }
}

void
IcePatch2::remove(const string& pa)
{
    const string path = simplify(pa);

    OS::structstat buf;
    if(OS::osstat(path, &buf) == -1)
    {
        throw "cannot stat `" + path + "':\n" + lastError();
    }

    if(S_ISDIR(buf.st_mode))
    {
        if(OS::rmdir(path) == -1)
        {
            if(errno == EACCES)
            {
                assert(false);
            }
            throw "cannot remove directory `" + path + "':\n" + lastError();
        }
    }
    else
    {
        if(OS::remove(path) == -1)
        {
            throw "cannot remove file `" + path + "':\n" + lastError();
        }
    }
}

void
IcePatch2::removeRecursive(const string& pa)
{
    const string path = simplify(pa);

    OS::structstat buf;
    if(OS::osstat(path, &buf) == -1)
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

        if(!isRoot(path))
        {
            if(OS::rmdir(path) == -1)
            {
                throw "cannot remove directory `" + path + "':\n" + lastError();
            }
        }
    }
    else
    {
        if(OS::remove(path) == -1)
        {
            throw "cannot remove file `" + path + "':\n" + lastError();
        }
    }
}

StringSeq
IcePatch2::readDirectory(const string& pa)
{
    const string path = simplify(pa);

#ifdef _WIN32

    StringSeq result;
    const wstring fs = IceUtil::stringToWstring(simplify(path + "/*"));

#  ifdef __BCPLUSPLUS__
    struct _wffblk data;
    int h = _wfindfirst(fs.c_str(), &data, FA_DIREC);
    if(h == -1)
    {
        if(_doserrno == ENMFILE)
        {
            return result;
        }
        throw "cannot read directory `" + path + "':\n" + lastError();
    }


    while(true)
    {
        string name = IceUtil::wstringToString(data.ff_name);
        assert(!name.empty());

        if(name != ".." && name != ".")
        {
            result.push_back(name);
        }

        if(_wfindnext(&data) == -1)
        {
            if(errno == ENOENT)
            {
                break;
            }

            string ex = "cannot read directory `" + path + "':\n" + lastError();
            _wfindclose(&data);
            throw ex;
        }
    }

    _wfindclose(&data);
#  else
    struct _wfinddata_t data;

#    if defined(_MSC_VER) && (_MSC_VER < 1300)
    long h = _wfindfirst(fs.c_str(), &data);
#    else
    intptr_t h = _wfindfirst(fs.c_str(), &data);
#    endif
    if(h == -1)
    {
        throw "cannot read directory `" + path + "':\n" + lastError();
    }

    while(true)
    {
        string name = IceUtil::wstringToString(data.name);
        assert(!name.empty());

        if(name != ".." && name != ".")
        {
            result.push_back(name);
        }

        if(_wfindnext(h, &data) == -1)
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
#  endif

    sort(result.begin(), result.end());
    return result;

#else

    struct dirent **namelist;
#ifdef __sun
    int n = ice_scandir(path.c_str(), &namelist, 0, ice_alphasort);
#else
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
#endif
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
    const string path = simplify(pa);

    if(OS::mkdir(path, 0777) == -1)
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
    const string path = simplify(pa);

    string dir = getDirname(path);
    if(!dir.empty())
    {
        createDirectoryRecursive(dir);
    }

    if(!isRoot(path + "/"))
    {
        OS::structstat buf;
        if(OS::osstat(path, &buf) != -1)
        {
            if(S_ISDIR(buf.st_mode))
            {
                return;
            }
        }

        if(OS::mkdir(path, 0777) == -1)
        {
            if(errno != EEXIST)
            {
                throw "cannot create directory `" + path + "':\n" + lastError();
            }
        }
    }
}

void
IcePatch2::compressBytesToFile(const string& pa, const ByteSeq& bytes, Int pos)
{
    const string path = simplify(pa);

    FILE* stdioFile = OS::fopen(path, "wb");
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

    BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[pos]), static_cast<int>(bytes.size() - pos));
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
}

void
IcePatch2::decompressFile(const string& pa)
{
    const string path = simplify(pa);
    const string pathBZ2 = path + ".bz2";

    FILE* fp = 0;
    FILE* stdioFileBZ2 = 0;
    int bzError;
    BZFILE* bzFile = 0;

    try
    {
        fp = OS::fopen(path, "wb");
        if(!fp)
        {
            throw "cannot open `" + path + "' for writing:\n" + lastError();
        }
        
        stdioFileBZ2 = OS::fopen(pathBZ2, "rb");
        if(!stdioFileBZ2)
        {
            throw "cannot open `" + pathBZ2 + "' for reading:\n" + lastError();
        }

#ifdef __BCPLUSPLUS__
        //
        // The BZ2_bzReadOpen/BZ2_bzRead/BZ2_bzReadClose functions fail with BCC
        //
        OS::structstat buf;
        if(OS::osstat(pathBZ2, &buf) == -1)
        {
            throw "cannot stat `" + pathBZ2 + "':\n" + lastError();
        }

        ByteSeq compressedBytes(buf.st_size);
        if(fread(&compressedBytes[0], buf.st_size, 1, stdioFileBZ2) == -1)
        {
             throw "cannot read from `" + pathBZ2 + "':\n" + lastError();
        }

        ByteSeq uncompressedBytes;
        unsigned int uncompressedLen = buf.st_size * 2;
        while(true)
        {
            uncompressedBytes.resize(uncompressedLen);
            int bzError = BZ2_bzBuffToBuffDecompress(&uncompressedBytes[0], &uncompressedLen, &compressedBytes[0], 
                                                     buf.st_size, 0, 0);
            if(bzError == BZ_OK)
            {
                break;
            }
            else if(bzError == BZ_OUTBUFF_FULL)
            {
                uncompressedLen *= 2;
                continue;
            }
            else
            {
                string ex = "BZ2_bzBuffToBuffDecompress failed";
                if(bzError == BZ_IO_ERROR)
                {
                    ex += string(": ") + lastError();
                }
                throw ex;
            }
        }

        if(fwrite(&uncompressedBytes[0], uncompressedLen, 1, fp) != 1)
        {
            throw "cannot write to `" + path + "':\n" + lastError();
        }
#else
        bzFile = BZ2_bzReadOpen(&bzError, stdioFileBZ2, 0, 0, 0, 0);
        if(bzError != BZ_OK)
        {
            string ex = "BZ2_bzReadOpen failed";
            if(bzError == BZ_IO_ERROR)
            {
                ex += string(": ") + lastError();
            }
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
                throw ex;
            }
            
            if(sz > 0)
            {
                long pos = ftell(stdioFileBZ2);
                if(pos == -1)
                {
                    throw "cannot get read position for `" + pathBZ2 + "':\n" + lastError();
                }
                
                if(fwrite(bytesBZ2, sz, 1, fp) != 1)
                {
                    throw "cannot write to `" + path + "':\n" + lastError();
                }
            }
        }
        
        BZ2_bzReadClose(&bzError, bzFile);
        bzFile = 0;
        if(bzError != BZ_OK)
        {
            string ex = "BZ2_bzReadClose failed";
            if(bzError == BZ_IO_ERROR)
            {
                ex += string(": ") + lastError();
            }
            throw ex;
        }
#endif
    }
    catch(...)
    {
        if(bzFile != 0)
        {
            BZ2_bzReadClose(&bzError, bzFile);
        }
        if(stdioFileBZ2 != 0)
        {
            fclose(stdioFileBZ2);
        }
        if(fp != 0)
        {
            fclose(fp);
        }
        throw;
    }

    fclose(stdioFileBZ2);
    fclose(fp);
}

void
IcePatch2::setFileFlags(const string& pa, const FileInfo& info)
{
#ifndef _WIN32 // Windows doesn't support the executable flag
    const string path = simplify(pa);
    OS::structstat buf;
    if(OS::osstat(path, &buf) == -1)
    {
        throw "cannot stat `" + path + "':\n" + lastError();
    }
    chmod(path.c_str(), info.executable ? buf.st_mode | S_IXUSR : buf.st_mode & ~S_IXUSR);
#endif
}

static bool
getFileInfoSeqInt(const string& basePath, const string& relPath, int compress, GetFileInfoSeqCB* cb,
                  FileInfoSeq& infoSeq)
{
    if(relPath == checksumFile || relPath == logFile)
    {
        return true;
    }

    const string path = simplify(basePath + '/' + relPath);

    if(ignoreSuffix(path))
    {
        const string pathWithoutSuffix = getWithoutSuffix(path);

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
            OS::structstat buf;
            if(OS::osstat(getWithoutSuffix(path), &buf) == -1)
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

        OS::structstat buf;
        if(OS::osstat(path, &buf) == -1)
        {
            throw "cannot stat `" + path + "':\n" + lastError();
        }

        if(S_ISDIR(buf.st_mode))
        {
            FileInfo info;
            info.path = relPath;
            info.size = -1;
            info.executable = false;

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
                if(!getFileInfoSeqInt(basePath, simplify(relPath + '/' + *p), compress, cb, infoSeq))
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
#ifdef _WIN32
            info.executable = false; // Windows doesn't support the executable flag
#else
            info.executable = buf.st_mode & S_IXUSR;
#endif

            OS::structstat bufBZ2;
            const string pathBZ2 = path + ".bz2";
            bool doCompress = false;
            if(buf.st_size != 0 && compress > 0)
            {
                //
                // compress == 0: Never compress.
                // compress == 1: Compress if necessary.
                // compress >= 2: Always compress.
                //
                if(compress >= 2 || OS::osstat(pathBZ2, &bufBZ2) == -1 || buf.st_mtime >= bufBZ2.st_mtime)
                {
                    if(cb && !cb->compress(relPath))
                    {
                        return false;
                    }

                    doCompress = true;
                }
                else
                {
                    info.size = static_cast<Int>(bufBZ2.st_size);
                }
            }

            if(cb && !cb->checksum(relPath))
            {
                return false;
            }

            ByteSeq bytesSHA(20);

            if(relPath.size() + buf.st_size == 0)
            {
                fill(bytesSHA.begin(), bytesSHA.end(), 0);
            }
            else
            {
                SHA_CTX ctx;
                SHA1_Init(&ctx);
                if(relPath.size() != 0)
                {
                    SHA1_Update(&ctx, reinterpret_cast<const void*>(relPath.c_str()), relPath.size());
                }

                if(buf.st_size != 0)
                {
#ifdef __BCPLUSPLUS__
                    //
                    // The BZ2_bzWriteOpen/BZ2_bzWrite/BZ2_bzWriteClose functions fail with BCC
                    //
                    if(doCompress)
                    {
                        int fd = OS::open(path.c_str(), O_BINARY|O_RDONLY);
                        if(fd == -1)
                        {
                            throw "cannot open `" + path + "' for reading:\n" + lastError();
                        }

                        ByteSeq uncompressedBytes(buf.st_size);
                    
                        if(read(fd, &uncompressedBytes[0], buf.st_size) == -1)
                        {
                            close(fd);
                            throw "cannot read from `" + path + "':\n" + lastError();
                        }

                        unsigned int compressedLen = buf.st_size * 1.01 + 600;
                        ByteSeq compressedBytes(compressedLen);

                        int bzError = BZ2_bzBuffToBuffCompress(&compressedBytes[0], &compressedLen, 
                                                               &uncompressedBytes[0], buf.st_size, 5, 0, 0);
                        if(bzError != BZ_OK)
                        {
                            string ex = "BZ2_bzBuffToBuffCompress failed";
                            if(bzError == BZ_IO_ERROR)
                            {
                                ex += string(": ") + lastError();
                            }
                            close(fd);
                            throw ex;
                        }
                        close(fd);

                        const string pathBZ2Temp = path + ".bz2temp";
                        FILE* stdioFile = OS::fopen(pathBZ2Temp, "wb");
                        if(fwrite(&compressedBytes[0], compressedLen, 1, stdioFile) != 1)
                        {
                            fclose(stdioFile);
                            throw "cannot write to `" + pathBZ2Temp + "':\n" + lastError();
                        }
                        fclose(stdioFile);

                        rename(pathBZ2Temp, pathBZ2);

                        info.size = compressedLen;
                    }
#endif

                    int fd = OS::open(path.c_str(), O_BINARY|O_RDONLY);
                    if(fd == -1)
                    {
                        throw "cannot open `" + path + "' for reading:\n" + lastError();
                    }

#ifndef __BCPLUSPLUS__
                    const string pathBZ2Temp = path + ".bz2temp";
                    FILE* stdioFile = 0;
                    int bzError = 0;
                    BZFILE* bzFile = 0;
                    if(doCompress)
                    {
                        stdioFile = OS::fopen(simplify(pathBZ2Temp), "wb");
                        if(!stdioFile)
                        {
                            close(fd);
                            throw "cannot open `" + pathBZ2Temp + "' for writing:\n" + lastError();
                        }

                        bzFile = BZ2_bzWriteOpen(&bzError, stdioFile, 5, 0, 0);
                        if(bzError != BZ_OK)
                        {
                            string ex = "BZ2_bzWriteOpen failed";
                            if(bzError == BZ_IO_ERROR)
                            {
                            ex += string(": ") + lastError();
                            }
                            fclose(stdioFile);
                            close(fd);
                            throw ex;
                        }
                    }
#endif

                    unsigned int bytesLeft = static_cast<unsigned int>(buf.st_size);
                    while(bytesLeft > 0)
                    {
                        ByteSeq bytes(min(bytesLeft, 1024u*1024));
                        if(read(fd, &bytes[0], static_cast<unsigned int>(bytes.size())) == -1)
                        {
#ifndef __BCPLUSPLUS__
                            if(doCompress)
                            {
                                fclose(stdioFile);
                            }
#endif
                            close(fd);
                            throw "cannot read from `" + path + "':\n" + lastError();
                        }
                        bytesLeft -= static_cast<unsigned int>(bytes.size());

#ifndef __BCPLUSPLUS__
                        if(doCompress)
                        {
                            BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[0]), static_cast<int>(bytes.size()));
                            if(bzError != BZ_OK)
                            {
                                string ex = "BZ2_bzWrite failed";
                                if(bzError == BZ_IO_ERROR)
                                {
                                    ex += string(": ") + lastError();
                                }
                                BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
                                fclose(stdioFile);
                                close(fd);
                                throw ex;
                            }
                        }
#endif

                        SHA1_Update(&ctx, reinterpret_cast<const void*>(&bytes[0]), bytes.size());
                    }

                    close(fd);

#ifndef __BCPLUSPLUS__
                    if(doCompress)
                    {
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

                        rename(pathBZ2Temp, pathBZ2);

                        if(OS::osstat(pathBZ2, &bufBZ2) == -1)
                        {
                            throw "cannot stat `" + pathBZ2 + "':\n" + lastError();
                        }

                        info.size = static_cast<Int>(bufBZ2.st_size);
                    }
#endif
                }

                SHA1_Final(reinterpret_cast<unsigned char*>(&bytesSHA[0]), &ctx);
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
    const string basePath = simplify(basePa);
    const string relPath = simplify(relPa);

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
        const string path = simplify(pa + '/' + checksumFile);
        
        FILE* fp = OS::fopen(path, "w");
        if(!fp)
        {
            throw "cannot open `" + path + "' for writing:\n" + lastError();
        }
        try
        {
            for(FileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
            {
                if(!writeFileInfo(fp, *p))
                {
                    throw "error writing `" + path + "':\n" + lastError();
                }
            }
        }
        catch(...)
        {
            fclose(fp);
            throw;
        }
        fclose(fp);
    }

    {
        const string pathLog = simplify(pa + '/' + logFile);

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
        const string path = simplify(pa + '/' + checksumFile);

        FILE* fp = OS::fopen(path, "r");
        if(!fp)
        {
            throw "cannot open `" + path + "' for reading:\n" + lastError();
        }

        while(true)
        {
            FileInfo info;
            if(readFileInfo(fp, info))
            {
                infoSeq.push_back(info);
            }
            else
            {
                break;
            }
        }
        fclose(fp);

        sort(infoSeq.begin(), infoSeq.end(), FileInfoLess());
        infoSeq.erase(unique(infoSeq.begin(), infoSeq.end(), FileInfoEqual()), infoSeq.end());
    }

    {
        const string pathLog = simplify(pa + '/' + logFile);

        FILE* fp = OS::fopen(pathLog, "r");
        if(fp != 0)
        {
            FileInfoSeq remove;
            FileInfoSeq update;
            
            while(true)
            {
                int c = fgetc(fp);
                if(c == EOF)
                {
                    break;
                }

                FileInfo info;
                if(!readFileInfo(fp, info))
                {
                    break;
                }

                if(c == '-')
                {
                    remove.push_back(info);
                }
                else if(c == '+')
                {
                    update.push_back(info);
                }
            }
            fclose(fp);

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
        allChecksums1.resize(tree1.files.size() * 21); // 20 bytes for the checksum + 1 byte for the flag
        ByteSeq::iterator c1 = allChecksums1.begin();

        for(p = tree1.files.begin(); p != tree1.files.end(); ++p, c1 += 21)
        {
            copy(p->checksum.begin(), p->checksum.end(), c1);
            *(c1 + 20) = p->executable;
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

