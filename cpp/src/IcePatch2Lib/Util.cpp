// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

//
// We need to include io.h first to get the proper signature for
// _wfindfirst
//
#ifdef _WIN32
#   include <io.h>
#endif

#include <IceUtil/IceUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/SHA1.h>
#include <IceUtil/Exception.h>
#include <IcePatch2Lib/Util.h>
#include <IcePatch2/FileServer.h>
#include <bzlib.h>
#include <iomanip>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

#include <iterator>

// Ignore macOS OpenSSL deprecation warnings
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

const char* IcePatch2Internal::checksumFile = "IcePatch2.sum";
const char* IcePatch2Internal::logFile = "IcePatch2.log";

using namespace std;
using namespace Ice;
using namespace IcePatch2;
using namespace IcePatch2Internal;

FileInfo
IcePatch2Internal::toFileInfo(const LargeFileInfo& largeInfo)
{
    if(largeInfo.size > 0x7FFFFFFF)
    {
        ostringstream os;
        os << "cannot encode size `" << largeInfo.size << "' for file `" << largeInfo.path << "' as Ice::Int" << endl;
        throw FileSizeRangeException(os.str());
    }

    FileInfo info;
    info.path = largeInfo.path;
    info.checksum = largeInfo.checksum;
    info.size = static_cast<Ice::Int>(largeInfo.size),
    info.executable = largeInfo.executable;

    return info;
}

LargeFileInfo
IcePatch2Internal::toLargeFileInfo(const FileInfo& info)
{
    LargeFileInfo largeInfo;
    largeInfo.path = info.path;
    largeInfo.checksum = info.checksum;
    largeInfo.size = info.size;
    largeInfo.executable = info.executable;
    return largeInfo;
}

bool
IcePatch2Internal::writeFileInfo(FILE* fp, const LargeFileInfo& info)
{
    int rc = fprintf(fp, "%s\t%s\t" ICE_INT64_FORMAT "\t%d\n",
                     escapeString(info.path, "", IceUtilInternal::Compat).c_str(),
                     bytesToString(info.checksum).c_str(),
                     info.size,
                     static_cast<int>(info.executable));
    return rc > 0;
}

bool
IcePatch2Internal::readFileInfo(FILE* fp, LargeFileInfo& info)
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
    try
    {
        info.path = IceUtilInternal::unescapeString(s, 0, s.size(), "");
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        throw invalid_argument(ex.reason());
    }

    getline(is, s, '\t');
    info.checksum = stringToBytes(s);

    is >> info.size;
    is >> info.executable;

    return true;
}

string
IcePatch2Internal::bytesToString(const ByteSeq& bytes)
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
IcePatch2Internal::stringToBytes(const string& str)
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
IcePatch2Internal::simplify(const string& path)
{
    string result = path;

    string::size_type pos;

#ifdef _WIN32
    pos = 0;
    if(result.find("\\\\") == 0)
    {
        pos = 2;
    }

    for(; pos < result.size(); ++pos)
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
       (result.size() == 4 && IceUtilInternal::isAlpha(result[0]) && result[1] == ':' &&
        result[2] == '/' && result[3] == '.'))
    {
       return result.substr(0, result.size() - 1);
    }

    if(result.size() >= 2 && result.substr(result.size() - 2, 2) == "/.")
    {
        result.erase(result.size() - 2, 2);
    }

    if(result == "/" || (result.size() == 3 && IceUtilInternal::isAlpha(result[0]) && result[1] == ':' &&
                         result[2] == '/'))
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
IcePatch2Internal::isRoot(const string& pa)
{
    string path = simplify(pa);
#ifdef _WIN32
    return path == "/" || path.size() == 3 && IceUtilInternal::isAlpha(path[0]) && path[1] == ':' &&
           path[2] == '/';
#else
    return path == "/";
#endif
}

string
IcePatch2Internal::getSuffix(const string& pa)
{
    const string path = simplify(pa);

    string::size_type dotPos = path.rfind('.');
    string::size_type slashPos = path.rfind('/');

    if(dotPos == string::npos || (slashPos != string::npos && slashPos > dotPos))
    {
        return string();
    }

    return path.substr(dotPos + 1);
}

string
IcePatch2Internal::getWithoutSuffix(const string& pa)
{
    const string path = simplify(pa);

    string::size_type dotPos = path.rfind('.');
    string::size_type slashPos = path.rfind('/');

    if(dotPos == string::npos || (slashPos != string::npos && slashPos > dotPos))
    {
        return path;
    }

    return path.substr(0, dotPos);
}

bool
IcePatch2Internal::ignoreSuffix(const string& path)
{
    string suffix = getSuffix(path);
    return suffix == "md5" // For legacy IcePatch.
        || suffix == "tot" // For legacy IcePatch.
        || suffix == "bz2"
        || suffix == "bz2temp";
}

string
IcePatch2Internal::getBasename(const string& pa)
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
IcePatch2Internal::getDirname(const string& pa)
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
IcePatch2Internal::rename(const string& fromPa, const string& toPa)
{

    const string fromPath = simplify(fromPa);
    const string toPath = simplify(toPa);

    IceUtilInternal::remove(toPath); // We ignore errors, as the file we are renaming to might not exist.
    if(IceUtilInternal::rename(fromPath, toPath) == -1)
    {
        throw runtime_error("cannot rename `" + fromPath + "' to  `" + toPath + "': " + IceUtilInternal::lastErrorToString());
    }
}

void
IcePatch2Internal::remove(const string& pa)
{
    const string path = simplify(pa);

    IceUtilInternal::structstat buf;
    if(IceUtilInternal::stat(path, &buf) == -1)
    {
        throw runtime_error("cannot stat `" + path + "':\n" + IceUtilInternal::lastErrorToString());
    }

    if(S_ISDIR(buf.st_mode))
    {
        if(IceUtilInternal::rmdir(path) == -1)
        {
            if(errno == EACCES)
            {
                assert(false);
            }
            throw runtime_error("cannot remove directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
        }
    }
    else
    {
        if(IceUtilInternal::remove(path) == -1)
        {
            throw runtime_error("cannot remove file `" + path + "':\n" + IceUtilInternal::lastErrorToString());
        }
    }
}

void
IcePatch2Internal::removeRecursive(const string& pa)
{
    const string path = simplify(pa);

    IceUtilInternal::structstat buf;
    if(IceUtilInternal::stat(path, &buf) == -1)
    {
        throw runtime_error("cannot stat `" + path + "':\n" + IceUtilInternal::lastErrorToString());
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
            if(IceUtilInternal::rmdir(path) == -1)
            {
                throw runtime_error("cannot remove directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
            }
        }
    }
    else
    {
        if(IceUtilInternal::remove(path) == -1)
        {
            throw runtime_error("cannot remove file `" + path + "':\n" + IceUtilInternal::lastErrorToString());
        }
    }
}

StringSeq
IcePatch2Internal::readDirectory(const string& pa)
{
    const string path = simplify(pa);

#ifdef _WIN32

    //
    // IcePatch2 doesn't support to use string converters, so don't need to use
    // any string converter in stringToWstring or wstringToString conversions.
    //
    StringSeq result;
    const wstring fs = IceUtil::stringToWstring(simplify(path + "/*"));

    struct _wfinddata_t data;
    intptr_t h = _wfindfirst(fs.c_str(), &data);
    if(h == -1)
    {
        throw runtime_error("cannot read directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
    }

    while(true)
    {
        string name = wstringToString(data.name);
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
            string reason = "cannot read directory `" + path + "':\n" + IceUtilInternal::lastErrorToString();
            _findclose(h);
            throw runtime_error(reason);
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
        throw runtime_error("cannot read directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
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
IcePatch2Internal::createDirectory(const string& pa)
{
    const string path = simplify(pa);

    if(IceUtilInternal::mkdir(path, 0777) == -1)
    {
        if(errno != EEXIST)
        {
            throw runtime_error("cannot create directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
        }
    }
}

void
IcePatch2Internal::createDirectoryRecursive(const string& pa)
{
    const string path = simplify(pa);

    string dir = getDirname(path);
    if(!dir.empty())
    {
        createDirectoryRecursive(dir);
    }

    if(!isRoot(path + "/"))
    {
        IceUtilInternal::structstat buf;
        if(IceUtilInternal::stat(path, &buf) != -1)
        {
            if(S_ISDIR(buf.st_mode))
            {
                return;
            }
        }

        if(IceUtilInternal::mkdir(path, 0777) == -1)
        {
            if(errno != EEXIST)
            {
                throw runtime_error("cannot create directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
            }
        }
    }
}

void
IcePatch2Internal::compressBytesToFile(const string& pa, const ByteSeq& bytes, Int pos)
{
    const string path = simplify(pa);

    FILE* stdioFile = IceUtilInternal::fopen(path, "wb");
    if(!stdioFile)
    {
        throw runtime_error("cannot open `" + path + "' for writing:\n" + IceUtilInternal::lastErrorToString());
    }

    int bzError;
    BZFILE* bzFile = BZ2_bzWriteOpen(&bzError, stdioFile, 5, 0, 0);
    if(bzError != BZ_OK)
    {
        string reason = "BZ2_bzWriteOpen failed";
        if(bzError == BZ_IO_ERROR)
        {
            reason += ": " + IceUtilInternal::lastErrorToString();
        }
        fclose(stdioFile);
        throw runtime_error(reason);
    }

    BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[pos]), static_cast<int>(bytes.size() - pos));
    if(bzError != BZ_OK)
    {
        string reason = "BZ2_bzWrite failed";
        if(bzError == BZ_IO_ERROR)
        {
            reason += ": " + IceUtilInternal::lastErrorToString();
        }
        BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
        fclose(stdioFile);
        throw runtime_error(reason);
    }

    BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
    if(bzError != BZ_OK)
    {
        string reason = "BZ2_bzWriteClose failed";
        if(bzError == BZ_IO_ERROR)
        {
            reason += ": " + IceUtilInternal::lastErrorToString();
        }
        fclose(stdioFile);
        throw runtime_error(reason);
    }

    fclose(stdioFile);
}

void
IcePatch2Internal::decompressFile(const string& pa)
{
    const string path = simplify(pa);
    const string pathBZ2 = path + ".bz2";

    FILE* fp = 0;
    FILE* stdioFileBZ2 = 0;
    int bzError;
    BZFILE* bzFile = 0;

    try
    {
        fp = IceUtilInternal::fopen(path, "wb");
        if(!fp)
        {
            throw runtime_error("cannot open `" + path + "' for writing:\n" + IceUtilInternal::lastErrorToString());
        }

        stdioFileBZ2 = IceUtilInternal::fopen(pathBZ2, "rb");
        if(!stdioFileBZ2)
        {
            throw runtime_error("cannot open `" + pathBZ2 + "' for reading:\n" + IceUtilInternal::lastErrorToString());
        }

        bzFile = BZ2_bzReadOpen(&bzError, stdioFileBZ2, 0, 0, 0, 0);
        if(bzError != BZ_OK)
        {
            string reason = "BZ2_bzReadOpen failed";
            if(bzError == BZ_IO_ERROR)
            {
                reason += ": " + IceUtilInternal::lastErrorToString();
            }
            throw runtime_error(reason);
        }

        const Int numBZ2 = 64 * 1024;
        Byte bytesBZ2[numBZ2];

        while(bzError != BZ_STREAM_END)
        {
            int sz = BZ2_bzRead(&bzError, bzFile, bytesBZ2, numBZ2);
            if(bzError != BZ_OK && bzError != BZ_STREAM_END)
            {
                string reason = "BZ2_bzRead failed";
                if(bzError == BZ_IO_ERROR)
                {
                    reason += ": " + IceUtilInternal::lastErrorToString();
                }
                throw runtime_error(reason);
            }

            if(sz > 0)
            {
                long pos = ftell(stdioFileBZ2);
                if(pos == -1)
                {
                    throw runtime_error("cannot get read position for `" + pathBZ2 + "':\n" + IceUtilInternal::lastErrorToString());
                }

                if(fwrite(bytesBZ2, sz, 1, fp) != 1)
                {
                    throw runtime_error("cannot write to `" + path + "':\n" + IceUtilInternal::lastErrorToString());
                }
            }
        }

        BZ2_bzReadClose(&bzError, bzFile);
        bzFile = 0;
        if(bzError != BZ_OK)
        {
            string reason = "BZ2_bzReadClose failed";
            if(bzError == BZ_IO_ERROR)
            {
                reason += ": " + IceUtilInternal::lastErrorToString();
            }
            throw runtime_error(reason);
        }
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

#ifndef _WIN32
void
IcePatch2Internal::setFileFlags(const string& pa, const LargeFileInfo& info)
{
    const string path = simplify(pa);
    IceUtilInternal::structstat buf;
    if(IceUtilInternal::stat(path, &buf) == -1)
    {
        throw runtime_error("cannot stat `" + path + "':\n" + IceUtilInternal::lastErrorToString());
    }
    chmod(path.c_str(), info.executable ? buf.st_mode | S_IXUSR : buf.st_mode & ~S_IXUSR);
}
#else // Windows doesn't support the executable flag
void
IcePatch2Internal::setFileFlags(const string&, const LargeFileInfo&)
{
}
#endif

namespace
{

static bool
getFileInfoSeqInternal(const string& basePath, const string& relPath, int compress, GetFileInfoSeqCB* cb,
                       LargeFileInfoSeq& infoSeq)
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
            IceUtilInternal::structstat buf;
            if(IceUtilInternal::stat(getWithoutSuffix(path), &buf) == -1)
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
                    throw runtime_error("cannot stat `" + path + "':\n" + IceUtilInternal::lastErrorToString());
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

        IceUtilInternal::structstat buf;
        if(IceUtilInternal::stat(path, &buf) == -1)
        {
            throw runtime_error("cannot stat `" + path + "':\n" + IceUtilInternal::lastErrorToString());
        }

        if(S_ISDIR(buf.st_mode))
        {
            LargeFileInfo info;
            info.path = relPath;
            info.size = -1;
            info.executable = false;

            ByteSeq bytes(relPath.size());
            copy(relPath.begin(), relPath.end(), bytes.begin());

            ByteSeq bytesSHA(20);
            if(!bytes.empty())
            {
                IceInternal::sha1(reinterpret_cast<unsigned char*>(&bytes[0]), bytes.size(), bytesSHA);
            }
            else
            {
                fill(bytesSHA.begin(), bytesSHA.end(), Byte(0));
            }
            info.checksum.swap(bytesSHA);

            infoSeq.push_back(info);

            StringSeq content = readDirectory(path);
            for(StringSeq::const_iterator p = content.begin(); p != content.end() ; ++p)
            {
                if(!getFileInfoSeqInternal(basePath, simplify(relPath + '/' + *p), compress, cb, infoSeq))
                {
                    return false;
                }
            }
        }
        else if(S_ISREG(buf.st_mode))
        {
            LargeFileInfo info;
            info.path = relPath;
            info.size = 0;
#ifdef _WIN32
            info.executable = false; // Windows doesn't support the executable flag
#else
            info.executable = buf.st_mode & S_IXUSR;
#endif

            IceUtilInternal::structstat bufBZ2;
            const string pathBZ2 = path + ".bz2";
            bool doCompress = false;
            if(buf.st_size != 0 && compress > 0)
            {
                //
                // compress == 0: Never compress.
                // compress == 1: Compress if necessary.
                // compress >= 2: Always compress.
                //
                if(compress >= 2 || IceUtilInternal::stat(pathBZ2, &bufBZ2) == -1 || buf.st_mtime >= bufBZ2.st_mtime)
                {
                    if(cb && !cb->compress(relPath))
                    {
                        return false;
                    }

                    doCompress = true;
                }
                else
                {
                    info.size = bufBZ2.st_size;
                }
            }

            if(cb && !cb->checksum(relPath))
            {
                return false;
            }

            ByteSeq bytesSHA;

            if(relPath.size() + buf.st_size == 0)
            {
                bytesSHA.resize(20);
                fill(bytesSHA.begin(), bytesSHA.end(), Byte(0));
            }
            else
            {
                IceInternal::SHA1 hasher;
                if(relPath.size() != 0)
                {
                    hasher.update(reinterpret_cast<const IceUtil::Byte*>(relPath.c_str()), relPath.size());
                }

                if(buf.st_size != 0)
                {
                    int fd = IceUtilInternal::open(path.c_str(), O_BINARY|O_RDONLY);
                    if(fd == -1)
                    {
                        throw runtime_error("cannot open `" + path + "' for reading:\n" + IceUtilInternal::lastErrorToString());
                    }

                    const string pathBZ2Temp = path + ".bz2temp";
                    FILE* stdioFile = 0;
                    int bzError = 0;
                    BZFILE* bzFile = 0;
                    if(doCompress)
                    {
                        stdioFile = IceUtilInternal::fopen(simplify(pathBZ2Temp), "wb");
                        if(!stdioFile)
                        {
                            IceUtilInternal::close(fd);
                            throw runtime_error("cannot open `" + pathBZ2Temp + "' for writing:\n" + IceUtilInternal::lastErrorToString());
                        }

                        bzFile = BZ2_bzWriteOpen(&bzError, stdioFile, 5, 0, 0);
                        if(bzError != BZ_OK)
                        {
                            string reason = "BZ2_bzWriteOpen failed";
                            if(bzError == BZ_IO_ERROR)
                            {
                                reason += ": " + IceUtilInternal::lastErrorToString();
                            }
                            fclose(stdioFile);
                            IceUtilInternal::close(fd);
                            throw runtime_error(reason);
                        }
                    }

                    long bytesLeft = buf.st_size;
                    while(bytesLeft > 0)
                    {
                        ByteSeq bytes(min(bytesLeft, 1024l*1024));
                        if(
#if defined(_MSC_VER)
                            _read(fd, &bytes[0], static_cast<unsigned int>(bytes.size()))
#else
                            read(fd, &bytes[0], static_cast<unsigned int>(bytes.size()))
#endif
                            == -1)
                        {
                            if(doCompress)
                            {
                                fclose(stdioFile);
                            }

                            IceUtilInternal::close(fd);
                            throw runtime_error("cannot read from `" + path + "':\n" + IceUtilInternal::lastErrorToString());
                        }
                        bytesLeft -= static_cast<unsigned int>(bytes.size());
                        if(doCompress)
                        {
                            BZ2_bzWrite(&bzError, bzFile, const_cast<Byte*>(&bytes[0]), static_cast<int>(bytes.size()));
                            if(bzError != BZ_OK)
                            {
                                string reason = "BZ2_bzWrite failed";
                                if(bzError == BZ_IO_ERROR)
                                {
                                    reason += ": " + IceUtilInternal::lastErrorToString();
                                }
                                BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
                                fclose(stdioFile);
                                IceUtilInternal::close(fd);
                                throw runtime_error(reason);
                            }
                        }

                        hasher.update(reinterpret_cast<IceUtil::Byte*>(&bytes[0]), bytes.size());
                    }

                    IceUtilInternal::close(fd);

                    if(doCompress)
                    {
                        BZ2_bzWriteClose(&bzError, bzFile, 0, 0, 0);
                        if(bzError != BZ_OK)
                        {
                            string reason = "BZ2_bzWriteClose failed";
                            if(bzError == BZ_IO_ERROR)
                            {
                                reason += ": " + IceUtilInternal::lastErrorToString();
                            }
                            fclose(stdioFile);
                            throw runtime_error(reason);
                        }

                        fclose(stdioFile);

                        rename(pathBZ2Temp, pathBZ2);

                        if(IceUtilInternal::stat(pathBZ2, &bufBZ2) == -1)
                        {
                            throw runtime_error("cannot stat `" + pathBZ2 + "':\n" + IceUtilInternal::lastErrorToString());
                        }

                        info.size = bufBZ2.st_size;
                    }
                }
                hasher.finalize(bytesSHA);
            }

            info.checksum.swap(bytesSHA);

            infoSeq.push_back(info);
        }
    }

    return true;
}

}

IcePatch2Internal::GetFileInfoSeqCB::~GetFileInfoSeqCB()
{
    // Out of line to avoid weak vtable
}

bool
IcePatch2Internal::getFileInfoSeq(const string& basePath, int compress, GetFileInfoSeqCB* cb,
                                  LargeFileInfoSeq& infoSeq)
{
    return getFileInfoSeqSubDir(basePath, ".", compress, cb, infoSeq);
}

bool
IcePatch2Internal::getFileInfoSeqSubDir(const string& basePa, const string& relPa, int compress, GetFileInfoSeqCB* cb,
                                        LargeFileInfoSeq& infoSeq)
{
    const string basePath = simplify(basePa);
    const string relPath = simplify(relPa);

    if(!getFileInfoSeqInternal(basePath, relPath, compress, cb, infoSeq))
    {
        return false;
    }

    sort(infoSeq.begin(), infoSeq.end(), FileInfoLess());
    infoSeq.erase(unique(infoSeq.begin(), infoSeq.end(), FileInfoEqual()), infoSeq.end());

    return true;
}

void
IcePatch2Internal::saveFileInfoSeq(const string& pa, const LargeFileInfoSeq& infoSeq)
{
    {
        const string path = simplify(pa + '/' + checksumFile);
        FILE* fp = IceUtilInternal::fopen(path, "w");
        if(!fp)
        {
            throw runtime_error("cannot open `" + path + "' for writing:\n" + IceUtilInternal::lastErrorToString());
        }
        try
        {
            for(LargeFileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
            {
                if(!writeFileInfo(fp, *p))
                {
                    throw runtime_error("error writing `" + path + "':\n" + IceUtilInternal::lastErrorToString());
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
IcePatch2Internal::loadFileInfoSeq(const string& pa, LargeFileInfoSeq& infoSeq)
{
    {
        const string path = simplify(pa + '/' + checksumFile);

        FILE* fp = IceUtilInternal::fopen(path, "r");
        if(!fp)
        {
            throw runtime_error("cannot open `" + path + "' for reading:\n" + IceUtilInternal::lastErrorToString());
        }

        while(true)
        {
            LargeFileInfo info;
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

        FILE* fp = IceUtilInternal::fopen(pathLog, "r");
        if(fp != 0)
        {
            LargeFileInfoSeq remove;
            LargeFileInfoSeq update;

            while(true)
            {
                int c = fgetc(fp);
                if(c == EOF)
                {
                    break;
                }

                LargeFileInfo info;
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

            LargeFileInfoSeq newInfoSeq;
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
IcePatch2Internal::getFileTree0(const LargeFileInfoSeq& infoSeq, FileTree0& tree0)
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

        for(LargeFileInfoSeq::const_iterator p = infoSeq.begin(); p != infoSeq.end(); ++p)
        {
            if(i == static_cast<int>(p->checksum[0]))
            {
                tree1.files.push_back(*p);
            }
        }

        ByteSeq allChecksums1;
        allChecksums1.resize(tree1.files.size() * 21); // 20 bytes for the checksum + 1 byte for the flag
        ByteSeq::iterator c1 = allChecksums1.begin();

        for(LargeFileInfoSeq::const_iterator p = tree1.files.begin(); p != tree1.files.end(); ++p, c1 += 21)
        {
            copy(p->checksum.begin(), p->checksum.end(), c1);
            *(c1 + 20) = p->executable;
        }

        if(!allChecksums1.empty())
        {
            IceInternal::sha1(reinterpret_cast<unsigned char*>(&allChecksums1[0]), allChecksums1.size(), tree1.checksum);
        }
        else
        {
            fill(tree1.checksum.begin(), tree1.checksum.end(), Byte(0));
        }

        copy(tree1.checksum.begin(), tree1.checksum.end(), c0);
    }

    if(!allChecksums0.empty())
    {
        IceInternal::sha1(reinterpret_cast<unsigned char*>(&allChecksums0[0]), allChecksums0.size(), tree0.checksum);
    }
    else
    {
        fill(tree0.checksum.begin(), tree0.checksum.end(), Byte(0));
    }
}
