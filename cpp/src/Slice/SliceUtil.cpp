// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "FileTracker.h"
#include "Ice/StringUtil.h"
#include "Util.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>

#ifndef _MSC_VER
#    include <unistd.h> // For readlink()
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    string normalizePath(const string& path)
    {
        string result = path;

        replace(result.begin(), result.end(), '\\', '/');

        string::size_type startReplace = 0;
#ifdef _WIN32
        //
        // For UNC paths we need to ensure they are in the format that is
        // returned by MCPP. IE. "//MACHINE/PATH"
        //
        if (result.find("//") == 0)
        {
            startReplace = 2;
        }
#endif
        string::size_type pos;
        while ((pos = result.find("//", startReplace)) != string::npos)
        {
            result.replace(pos, 2, "/");
        }
        pos = 0;
        while ((pos = result.find("/./", pos)) != string::npos)
        {
            result.erase(pos, 2);
        }
        pos = 0;
        while ((pos = result.find("/..", pos)) != string::npos)
        {
            string::size_type last = result.find_last_of('/', pos - 1);
            if (last != string::npos && result.substr(last, 4) != "/../")
            {
                result.erase(last, pos - last + 3);
                pos = last;
            }
            else
            {
                ++pos;
            }
        }

        if (result.size() > 1) // Remove trailing "/" or "/."
        {
            if (result[result.size() - 1] == '/')
            {
                result.erase(result.size() - 1);
            }
            else if (result[result.size() - 2] == '/' && result[result.size() - 1] == '.')
            {
                result.erase(result.size() - (result.size() == 2 ? 1 : 2));
            }
        }
        return result;
    }
}

string
Slice::fullPath(const string& path)
{
    //
    // This function returns the canonicalized absolute pathname of
    // the given path.
    //
    // It is used for instance used to normalize the paths provided
    // with -I options. Like mcpp, we need to follow the symbolic
    // links to ensure changeInclude works correctly. For example, it
    // must be possible to specify -I/opt/Ice-3.6 where Ice-3.6 is
    // symbolic link to Ice-3.6.0 (if we don't do the same as mcpp,
    // the generated headers will contain a full path...)
    //

    string result = path;
    if (!IceInternal::isAbsolutePath(result))
    {
        string cwd;
        if (IceInternal::getcwd(cwd) == 0)
        {
            result = string(cwd) + '/' + result;
        }
    }

    result = normalizePath(result);

#ifdef _WIN32
    return result;
#else

    string::size_type beg = 0;
    string::size_type next;
    do
    {
        string subpath;
        next = result.find('/', beg + 1);
        if (next == string::npos)
        {
            subpath = result;
        }
        else
        {
            subpath = result.substr(0, next);
        }

        char buf[PATH_MAX + 1];
        int len = static_cast<int>(readlink(subpath.c_str(), buf, sizeof(buf)));
        if (len > 0)
        {
            buf[len] = '\0';
            string linkpath = buf;
            if (!IceInternal::isAbsolutePath(linkpath)) // Path relative to the location of the link
            {
                string::size_type pos = subpath.rfind('/');
                assert(pos != string::npos);
                linkpath = subpath.substr(0, pos + 1) + linkpath;
            }
            result = normalizePath(linkpath) + (next != string::npos ? result.substr(next) : string());
            beg = 0;
            next = 0;
        }
        else
        {
            beg = next;
        }
    } while (next != string::npos);
    return result;
#endif
}

string
Slice::changeInclude(const string& p, const vector<string>& includePaths)
{
    string path = normalizePath(p);
    //
    // Compare each include path against the included file and select
    // the path that produces the shortest relative filename.
    //
    string result = path;
    vector<string> paths;
    paths.push_back(path);
    //
    // if path is not a canonical path we also test with its canonical
    // path
    //
    string canonicalPath = fullPath(path);
    if (canonicalPath != path)
    {
        paths.push_back(canonicalPath);
    }

    for (const auto& i : paths)
    {
        for (const auto& includePath : includePaths)
        {
            if (i.compare(0, includePath.length(), includePath) == 0)
            {
                string s = i.substr(includePath.length() + 1); // + 1 for the '/'
                if (s.size() < result.size())
                {
                    result = s;
                }
            }
        }

        //
        // If the path has been already shortened no need to test
        // with canonical path.
        //
        if (result != path)
        {
            break;
        }
    }

    return removeExtension(normalizePath(result));
}

string
Slice::removeExtension(const string& path)
{
    string result = path;
    string::size_type pos;
    if ((pos = result.rfind('.')) != string::npos)
    {
        result.erase(pos);
    }
    return result;
}

void
Slice::emitError(string_view file, int line, string_view message)
{
    if (!file.empty())
    {
        consoleErr << file;
        if (line != -1)
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
    consoleErr << message << endl;
}

void
Slice::emitWarning(string_view file, int line, string_view message)
{
    if (!file.empty())
    {
        consoleErr << file;
        if (line != -1)
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
    consoleErr << "warning: " << message << endl;
}

void
Slice::emitRaw(const char* message)
{
    consoleErr << message << flush;
}

vector<string>
Slice::filterMcppWarnings(const string& message)
{
    static const char* messages[] = {"Converted [CR+LF] to [LF]", "no newline, supplemented newline", nullptr};

    constexpr string_view warningPrefix = "warning:";
    constexpr string_view fromPrefix = "from";
    constexpr string_view separators = "\n\t ";

    vector<string> in;
    string::size_type start = 0;
    string::size_type end;
    while ((end = message.find('\n', start)) != string::npos)
    {
        in.push_back(message.substr(start, end - start));
        start = end + 1;
    }
    vector<string> out;
    bool skipped;
    for (auto i = in.begin(); i != in.end(); i++)
    {
        skipped = false;

        if (i->find(warningPrefix) != string::npos)
        {
            for (int j = 0; messages[j] != nullptr; ++j)
            {
                if (i->find(messages[j]) != string::npos)
                {
                    // This line should be skipped it contains the unwanted mcpp warning
                    // next line should also be skipped it contains the slice line that
                    // produces the skipped warning
                    i++;
                    skipped = true;
                    //
                    // Check if next lines are still the same warning
                    //
                    i++;
                    while (i != in.end())
                    {
                        string token = *i;
                        string::size_type index = token.find_first_not_of(separators);
                        if (index != string::npos)
                        {
                            token = token.substr(index);
                        }
                        if (token.find(fromPrefix) != 0)
                        {
                            //
                            // First line not of this warning
                            //
                            i--;
                            break;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    break;
                }
            }
            if (i == in.end())
            {
                break;
            }
        }
        if (!skipped)
        {
            out.push_back(*i + "\n");
        }
    }
    return out;
}

void
Slice::printGeneratedHeader(IceInternal::Output& out, string_view path, string_view commentStyle)
{
    // Get only the file name part of the given path.
    size_t pos = path.find_last_of("/\\");
    if (string::npos != pos)
    {
        path = path.substr(pos + 1);
    }

    out << nl << commentStyle << " <auto-generated>Generated from Slice file '" << path << "'.</auto-generated>";
}

void
Slice::writeDependencies(const string& dependencies, const string& dependFile)
{
    if (dependFile.empty())
    {
        consoleOut << dependencies << flush;
    }
    else
    {
        ofstream of(IceInternal::streamFilename(dependFile).c_str()); // dependFile is a UTF-8 string
        if (!of)
        {
            ostringstream os;
            os << "cannot open file '" << dependFile << "': " << IceInternal::errorToString(errno);
            throw Slice::FileException(os.str());
        }
        of << dependencies;
        of.close();
    }
}

#ifdef _WIN32
vector<string>
Slice::argvToArgs(int argc, wchar_t* argv[])
{
    vector<string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(Ice::wstringToString(argv[i]));
    }
    return args;
}
#else
vector<string>
Slice::argvToArgs(int argc, char* argv[])
{
    vector<string> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; i++)
    {
        args.emplace_back(argv[i]);
    }
    return args;
}
#endif

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
vector<string>
Slice::splitScopedName(const string& scoped, bool allowEmpty)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while ((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if (pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if (endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if (next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else if (allowEmpty)
    {
        ids.emplace_back("");
    }

    return ids;
}

string
Slice::getArticleFor(const string& s)
{
    static const string vowels = "aAeEiIoOuU";
    return (vowels.find_first_of(s[0]) != string::npos) ? "an" : "a";
}

std::string
Slice::pluralKindOf(const ContainedPtr& p)
{
    string kindOf = p->kindOf();
    if (kindOf == "class")
    {
        return "classes";
    }
    else if (kindOf == "dictionary")
    {
        return "dictionaries";
    }
    else
    {
        return kindOf + 's';
    }
}

bool
Slice::checkIdentifier(const string& identifier)
{
    // check whether the identifier is scoped
    size_t scopeIndex = identifier.rfind("::");
    bool isScoped = scopeIndex != string::npos;
    string name;
    if (isScoped)
    {
        name = identifier.substr(scopeIndex + 2); // Only check the unscoped identifier for syntax
    }
    else
    {
        name = identifier;
    }

    assert(!name.empty());
    bool isValid = true;

    // check the identifier for reserved suffixes
    static const string suffixBlacklist[] = {"Helper", "Holder", "Prx", "Ptr"};
    for (const auto& i : suffixBlacklist)
    {
        if (name.find(i, name.size() - i.size()) != string::npos)
        {
            currentUnit->error("illegal identifier '" + name + "': '" + i + "' suffix is reserved");
            isValid = false;
            break;
        }
    }

    // check the identifier for illegal underscores
    size_t index = name.find('_');
    if (index == 0)
    {
        currentUnit->error("illegal leading underscore in identifier '" + name + "'");
        isValid = false;
    }
    else if (name.rfind('_') == (name.size() - 1))
    {
        currentUnit->error("illegal trailing underscore in identifier '" + name + "'");
        isValid = false;
    }
    else if (name.find("__") != string::npos)
    {
        currentUnit->error("illegal double underscore in identifier '" + name + "'");
        isValid = false;
    }

    return isValid;
}

bool
Slice::isProxyType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin && builtin->kind() == Builtin::KindObjectProxy)
    {
        return true;
    }

    if (dynamic_pointer_cast<InterfaceDecl>(type))
    {
        return true;
    }
    return false;
}

bool
Slice::isFirstElement(const ContainedPtr& p)
{
    ContainerPtr container = dynamic_pointer_cast<Container>(p->container());
    return p == container->contents().front();
}

string
Slice::getFirstSentence(const StringList& lines)
{
    // Extract the first sentence.
    ostringstream ostr;
    for (auto i = lines.begin(); i != lines.end(); ++i)
    {
        const string ws = " \t";

        if (i->empty())
        {
            break;
        }
        if (i != lines.begin() && i->find_first_not_of(ws) == 0)
        {
            ostr << " ";
        }
        string::size_type pos = i->find('.');
        if (pos == string::npos)
        {
            ostr << *i;
        }
        else if (pos == i->size() - 1)
        {
            ostr << *i;
            break;
        }
        else
        {
            // Assume a period followed by whitespace indicates the end of the sentence.
            while (pos != string::npos)
            {
                if (ws.find((*i)[pos + 1]) != string::npos)
                {
                    break;
                }
                pos = i->find('.', pos + 1);
            }
            if (pos != string::npos)
            {
                ostr << i->substr(0, pos + 1);
                break;
            }
            else
            {
                ostr << *i;
            }
        }
    }

    return ostr.str();
}
