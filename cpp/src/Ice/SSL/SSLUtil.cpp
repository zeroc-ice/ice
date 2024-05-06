//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceUtil/Config.h"
#if defined(_WIN32)
#    include <winsock2.h>
#endif

#include "../Base64.h"
#include "../Network.h"
#include "Ice/LocalException.h"
#include "Ice/SSL/Certificate.h"
#include "Ice/StringConverter.h"
#include "Ice/UniqueRef.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "SSLUtil.h"

#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceUtil;
using namespace Ice::SSL;

#if defined(__APPLE__)

std::string
Ice::SSL::fromCFString(CFStringRef v)
{
    string s;
    if (v)
    {
        CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(v), kCFStringEncodingUTF8);
        vector<char> buffer;
        buffer.resize(static_cast<size_t>(size + 1));
        CFStringGetCString(v, &buffer[0], static_cast<CFIndex>(buffer.size()), kCFStringEncodingUTF8);
        s.assign(&buffer[0]);
    }
    return s;
}

#endif

bool
Ice::SSL::parseBytes(const string& arg, vector<unsigned char>& buffer)
{
    string v = IceUtilInternal::toUpper(arg);

    // Check for any invalid characters.
    size_t pos = v.find_first_not_of(" :0123456789ABCDEF");
    if (pos != string::npos)
    {
        return false;
    }

    // Remove any separator characters.
    ostringstream s;
    for (string::const_iterator i = v.begin(); i != v.end(); ++i)
    {
        if (*i == ' ' || *i == ':')
        {
            continue;
        }
        s << *i;
    }
    v = s.str();

    // Convert the bytes.
    for (size_t i = 0, length = v.size(); i + 2 <= length;)
    {
        buffer.push_back(static_cast<unsigned char>(strtol(v.substr(i, 2).c_str(), 0, 16)));
        i += 2;
    }
    return true;
}

void
Ice::SSL::readFile(const string& file, vector<char>& buffer)
{
    ifstream is(IceUtilInternal::streamFilename(file).c_str(), ios::in | ios::binary);
    if (!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }

    is.seekg(0, is.end);
    buffer.resize(static_cast<size_t>(is.tellg()));
    is.seekg(0, is.beg);

    if (!buffer.empty())
    {
        is.read(&buffer[0], static_cast<streamsize>(buffer.size()));
        if (!is.good())
        {
            throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
        }
    }
}

bool
Ice::SSL::checkPath(const string& path, const string& defaultDir, bool dir, string& resolved)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS) || defined(ICE_SWIFT)
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle)
    {
        UniqueRef<CFStringRef> resourceName(toCFString(path));
        UniqueRef<CFStringRef> subDirName(toCFString(defaultDir));
        UniqueRef<CFURLRef> url(CFBundleCopyResourceURL(bundle, resourceName.get(), 0, subDirName.get()));

        UInt8 filePath[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(url.get(), true, filePath, sizeof(filePath)))
        {
            string tmp = string(reinterpret_cast<char*>(filePath));
            if ((dir && IceUtilInternal::directoryExists(tmp)) || (!dir && IceUtilInternal::fileExists(tmp)))
            {
                resolved = tmp;
                return true;
            }
        }
    }
#endif
    if (IceUtilInternal::isAbsolutePath(path))
    {
        if ((dir && IceUtilInternal::directoryExists(path)) || (!dir && IceUtilInternal::fileExists(path)))
        {
            resolved = path;
            return true;
        }
        return false;
    }

    //
    // If a default directory is provided, the given path is relative to the default directory.
    //
    string tmp;
    if (!defaultDir.empty())
    {
        tmp = defaultDir + IceUtilInternal::separator + path;
    }
    else
    {
        tmp = path;
    }

    if ((dir && IceUtilInternal::directoryExists(tmp)) || (!dir && IceUtilInternal::fileExists(tmp)))
    {
        resolved = tmp;
        return true;
    }
    return false;
}