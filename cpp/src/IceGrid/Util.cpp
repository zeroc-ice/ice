// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "../Ice/FileUtil.h"
#include "Ice/Ice.h"
#include "Ice/StringUtil.h"
#include "IceGrid/Admin.h"
#include "Internal.h"

#ifdef _WIN32
#    include <direct.h>
#    include <io.h>
#else
#    include <dirent.h>
#    include <unistd.h>
#endif

#include <cassert>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace
{
    bool isRoot(const string& pa)
    {
        string path = simplify(pa);
#ifdef _WIN32
        return path == "/" || path.size() == 3 && IceInternal::isAlpha(path[0]) && path[1] == ':' && path[2] == '/';
#else
        return path == "/";
#endif
    }
}

string
IceGrid::toString(const vector<string>& v, const string& sep)
{
    ostringstream os;
    auto p = v.begin();
    while (p != v.end())
    {
        os << *p;
        ++p;
        if (p != v.end())
        {
            os << sep;
        }
    }
    return os.str();
}

string
IceGrid::toString(exception_ptr exception)
{
    std::ostringstream os;
    try
    {
        rethrow_exception(exception);
    }
    catch (const NodeUnreachableException& ex)
    {
        os << ex << ":";
        os << "\nnode: " << ex.name;
        os << "\nreason: " << ex.reason;
    }
    catch (const DeploymentException& ex)
    {
        os << ex << ":";
        os << "\nreason: " << ex.reason;
    }
    catch (const Ice::Exception& ex)
    {
        os << ex;
    }
    return os.str();
}

string
IceGrid::getProperty(const PropertyDescriptorSeq& properties, const string& name, const string& def)
{
    string result;
    for (const auto& prop : properties)
    {
        if (prop.name == name)
        {
            result = prop.value;
        }
    }
    if (result.empty())
    {
        return def;
    }
    return result;
}

int
IceGrid::getPropertyAsInt(const PropertyDescriptorSeq& properties, const string& name, int def)
{
    string strVal;
    for (const auto& prop : properties)
    {
        if (prop.name == name)
        {
            strVal = prop.value;
        }
    }

    int result = def;

    if (!strVal.empty())
    {
        istringstream v(strVal);
        if (!(v >> result) || !v.eof())
        {
            result = def;
        }
    }
    return result;
}

bool
IceGrid::hasProperty(const PropertyDescriptorSeq& properties, const string& name)
{
    for (const auto& prop : properties)
    {
        if (prop.name == name)
        {
            return true;
        }
    }
    return false;
}

PropertyDescriptor
IceGrid::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

string
IceGrid::escapeProperty(const string& s, bool escapeEqual)
{
    size_t firstChar = s.find_first_not_of(' ');
    size_t lastChar = s.find_last_not_of(' ');
    string result;
    bool previousCharIsEscape = false;

    for (unsigned int i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        switch (c)
        {
            case ' ':
            {
                //
                // We only escape the space character when it's at the beginning
                // or at the end of the string
                //
                if (i < firstChar || i > lastChar)
                {
                    if (previousCharIsEscape)
                    {
                        result.push_back('\\'); // escape the previous char, by adding another escape.
                    }

                    result.push_back('\\');
                }
                result.push_back(c);
                previousCharIsEscape = false;
                break;
            }

            case '\\':
            case '#':
            case '=':
            {
                if (c == '=' && !escapeEqual)
                {
                    previousCharIsEscape = false;
                }
                else
                {
                    //
                    // We only escape the \ character when it is followed by a
                    // character that we escape, e.g. \# is encoded as \\\#, not \#
                    // and \\server is encoded as \\\server.
                    //
                    if (previousCharIsEscape)
                    {
                        result.push_back('\\'); // escape the previous char, by adding another escape.
                    }
                    if (c == '\\')
                    {
                        previousCharIsEscape = true; // deferring the potential escaping to the next loop
                    }
                    else
                    {
                        result.push_back('\\');
                        previousCharIsEscape = false;
                    }
                }
                result.push_back(c);
                break;
            }

            default:
            {
                result.push_back(c);
                previousCharIsEscape = false;
                break;
            }
        }
    }
    return result;
}

ObjectInfo
IceGrid::toObjectInfo(
    const shared_ptr<Ice::Communicator>& communicator,
    const ObjectDescriptor& obj,
    const string& adapterId)
{
    ObjectInfo info;
    info.type = obj.type;
    ostringstream proxyStr;
    proxyStr << "\"" << communicator->identityToString(obj.id) << "\"";
    if (!obj.proxyOptions.empty())
    {
        proxyStr << ' ' << obj.proxyOptions;
    }
    proxyStr << " @ \"" << adapterId << "\"";
    try
    {
        info.proxy = ObjectPrx{communicator, proxyStr.str()};
    }
    catch (const Ice::ParseException&)
    {
        ostringstream fallbackProxyStr;
        fallbackProxyStr << "\"" << communicator->identityToString(obj.id) << "\""
                         << " @ \"" << adapterId << "\"";
        info.proxy = ObjectPrx{communicator, fallbackProxyStr.str()};
    }
    return info;
}

void
IceGrid::setupThreadPool(
    const Ice::PropertiesPtr& properties,
    const string& name,
    int size,
    int sizeMax,
    bool serialize)
{
    if (properties->getPropertyAsIntWithDefault(name + ".Size", 0) < size)
    {
        ostringstream os;
        os << size;
        properties->setProperty(name + ".Size", os.str());
    }
    else
    {
        size = properties->getPropertyAsInt(name + ".Size");
    }

    if (sizeMax > 0 && properties->getPropertyAsIntWithDefault(name + ".SizeMax", 0) < sizeMax)
    {
        if (size >= sizeMax)
        {
            sizeMax = size * 10;
        }

        ostringstream os;
        os << sizeMax;
        properties->setProperty(name + ".SizeMax", os.str());
    }

    if (serialize)
    {
        properties->setProperty(name + ".Serialize", "1");
    }
}

int
IceGrid::getMMVersion(const string& o)
{
    //
    // Strip the version
    //
    string::size_type beg = o.find_first_not_of(' ');
    string::size_type end = o.find_last_not_of(' ');
    string version = o.substr(beg == string::npos ? 0 : beg, end == string::npos ? o.length() - 1 : end - beg + 1);

    string::size_type minorPos = version.find('.');
    string::size_type patchPos = version.find('.', minorPos + 1);

    if (minorPos != 1 && minorPos != 2)
    {
        return -1;
    }

    if (patchPos != string::npos)
    {
        if ((minorPos == 1 && patchPos != 3 && patchPos != 4) || (minorPos == 2 && patchPos != 4 && patchPos != 5))
        {
            return -1;
        }
        else if ((version.size() - patchPos - 1) > 2)
        {
            return -1;
        }
    }
    else if ((version.size() - minorPos - 1) > 2)
    {
        return -1;
    }

    int v, ver;

    istringstream major(version.substr(0, minorPos));
    major >> v;
    if (major.fail() || v > 99 || v < 1)
    {
        return -1;
    }
    ver = v;
    ver *= 100;

    istringstream minor(version.substr(minorPos + 1, patchPos != string::npos ? patchPos : version.size()));
    minor >> v;
    if (minor.fail() || v > 99 || v < 0)
    {
        return -1;
    }
    ver += v;
    ver *= 100;

    //
    // No need to get the patch number, we're only interested in
    // MAJOR.MINOR
    //
    //     if(patchPos != string::npos)
    //     {
    //      istringstream patch(version.substr(patchPos + 1));
    //      patch >> v;
    //      if(patch.fail() || v > 99 || v < 0)
    //      {
    //          return -1;
    //      }
    //      ver += v;
    //     }

    return ver;
}

int
IceGrid::secondsToInt(const std::chrono::seconds& sec)
{
    return static_cast<int>(sec.count());
}

void
IceGrid::createDirectory(const string& pa)
{
    const string path = simplify(pa);

    if (IceInternal::mkdir(path, 0777) == -1)
    {
        if (errno != EEXIST)
        {
            throw runtime_error("cannot create directory '" + path + "':\n" + IceInternal::lastErrorToString());
        }
    }
}

StringSeq
IceGrid::readDirectory(const string& pa)
{
    const string path = simplify(pa);

#ifdef _WIN32

    //
    // IceGrid doesn't support to use string converters, so don't need to use
    // any string converter in stringToWstring or wstringToString conversions.
    //
    StringSeq result;
    const wstring fs = Ice::stringToWstring(simplify(path + "/*"));

    struct _wfinddata_t data;
    intptr_t h = _wfindfirst(fs.c_str(), &data);
    if (h == -1)
    {
        throw runtime_error("cannot read directory '" + path + "':\n" + IceInternal::lastErrorToString());
    }

    while (true)
    {
        string name = wstringToString(data.name);
        assert(!name.empty());

        if (name != ".." && name != ".")
        {
            result.push_back(name);
        }

        if (_wfindnext(h, &data) == -1)
        {
            if (errno == ENOENT)
            {
                break;
            }
            string reason = "cannot read directory '" + path + "':\n" + IceInternal::lastErrorToString();
            _findclose(h);
            throw runtime_error(reason);
        }
    }

    _findclose(h);

    sort(result.begin(), result.end());
    return result;

#else

    struct dirent** namelist;
    int n = scandir(path.c_str(), &namelist, nullptr, alphasort);

    if (n < 0)
    {
        throw runtime_error("cannot read directory '" + path + "':\n" + IceInternal::lastErrorToString());
    }

    StringSeq result;
    assert(n >= 2);
    result.reserve(static_cast<size_t>(n - 2));

    for (int i = 0; i < n; ++i)
    {
        string name = namelist[i]->d_name;
        assert(!name.empty());

        free(namelist[i]);

        if (name != ".." && name != ".")
        {
            result.push_back(name);
        }
    }

    free(namelist);
    return result;

#endif
}

void
IceGrid::remove(const string& pa)
{
    const string path = simplify(pa);

    IceInternal::structstat buf;
    if (IceInternal::stat(path, &buf) == -1)
    {
        throw runtime_error("cannot stat '" + path + "':\n" + IceInternal::lastErrorToString());
    }

    if (S_ISDIR(buf.st_mode))
    {
        if (IceInternal::rmdir(path) == -1)
        {
            if (errno == EACCES)
            {
                assert(false);
            }
            throw runtime_error("cannot remove directory '" + path + "':\n" + IceInternal::lastErrorToString());
        }
    }
    else
    {
        if (IceInternal::remove(path) == -1)
        {
            throw runtime_error("cannot remove file '" + path + "':\n" + IceInternal::lastErrorToString());
        }
    }
}

void
IceGrid::removeRecursive(const string& pa)
{
    const string path = simplify(pa);

    IceInternal::structstat buf;
    if (IceInternal::stat(path, &buf) == -1)
    {
        throw runtime_error("cannot stat '" + path + "':\n" + IceInternal::lastErrorToString());
    }

    if (S_ISDIR(buf.st_mode))
    {
        StringSeq paths = readDirectory(path);
        for (const auto& p : paths)
        {
            removeRecursive(path + '/' + p);
        }

        if (!isRoot(path))
        {
            if (IceInternal::rmdir(path) == -1)
            {
                throw runtime_error("cannot remove directory '" + path + "':\n" + IceInternal::lastErrorToString());
            }
        }
    }
    else
    {
        if (IceInternal::remove(path) == -1)
        {
            throw runtime_error("cannot remove file '" + path + "':\n" + IceInternal::lastErrorToString());
        }
    }
}

string
IceGrid::simplify(const string& path)
{
    string result = path;

    string::size_type pos;

#ifdef _WIN32
    pos = 0;
    if (result.find("\\\\") == 0)
    {
        pos = 2;
    }

    for (; pos < result.size(); ++pos)
    {
        if (result[pos] == '\\')
        {
            result[pos] = '/';
        }
    }
#endif

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

    while (result.substr(0, 4) == "/../")
    {
        result.erase(0, 3);
    }

    if (result.substr(0, 2) == "./")
    {
        result.erase(0, 2);
    }

    if (result == "/." || (result.size() == 4 && IceInternal::isAlpha(result[0]) && result[1] == ':' &&
                           result[2] == '/' && result[3] == '.'))
    {
        return result.substr(0, result.size() - 1);
    }

    if (result.size() >= 2 && result.substr(result.size() - 2, 2) == "/.")
    {
        result.erase(result.size() - 2, 2);
    }

    if (result == "/" ||
        (result.size() == 3 && IceInternal::isAlpha(result[0]) && result[1] == ':' && result[2] == '/'))
    {
        return result;
    }

    if (result.size() >= 1 && result[result.size() - 1] == '/')
    {
        result.erase(result.size() - 1);
    }

    if (result == "/..")
    {
        result = "/";
    }

    return result;
}
