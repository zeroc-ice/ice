// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Slice/Util.h>
#include <Slice/FileTracker.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/ConsoleUtil.h>
#include <climits>
#include <cstring>

#ifndef _MSC_VER
#  include <unistd.h> // For readlink()
#endif

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

string
normalizePath(const string& path)
{
    string result = path;

    replace(result.begin(), result.end(), '\\', '/');

    string::size_type startReplace = 0;
#ifdef _WIN32
    //
    // For UNC paths we need to ensure they are in the format that is
    // returned by MCPP. IE. "//MACHINE/PATH"
    //
    if(result.find("//") == 0)
    {
        startReplace = 2;
    }
#endif
    string::size_type pos;
    while((pos = result.find("//", startReplace)) != string::npos)
    {
        result.replace(pos, 2, "/");
    }
    pos = 0;
    while((pos = result.find("/./", pos)) != string::npos)
    {
        result.erase(pos, 2);
    }
    pos = 0;
    while((pos = result.find("/..", pos)) != string::npos)
    {
        string::size_type last = result.find_last_of("/", pos - 1);
        if(last != string::npos && result.substr(last, 4) != "/../")
        {
            result.erase(last, pos - last + 3);
            pos = last;
        }
        else
        {
            ++pos;
        }
    }

    if(result.size() > 1) // Remove trailing "/" or "/."
    {
        if(result[result.size() - 1] == '/')
        {
            result.erase(result.size() - 1);
        }
        else if(result[result.size() - 2] == '/' && result[result.size() - 1] == '.')
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
    // links to ensure changeIncludes works correctly. For example, it
    // must be possible to specify -I/opt/Ice-3.6 where Ice-3.6 is
    // symbolic link to Ice-3.6.0 (if we don't do the same as mcpp,
    // the generated headers will contain a full path...)
    //

    string result = path;
    if(!IceUtilInternal::isAbsolutePath(result))
    {
        string cwd;
        if(IceUtilInternal::getcwd(cwd) == 0)
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
        if(next == string::npos)
        {
            subpath = result;
        }
        else
        {
            subpath = result.substr(0, next);
        }

        char buf[PATH_MAX + 1];
        int len = static_cast<int>(readlink(subpath.c_str(), buf, sizeof(buf)));
        if(len > 0)
        {
            buf[len] = '\0';
            string linkpath = buf;
            if(!IceUtilInternal::isAbsolutePath(linkpath)) // Path relative to the location of the link
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
    }
    while(next != string::npos);
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
    if(canonicalPath != path)
    {
        paths.push_back(canonicalPath);
    }
    
    for(vector<string>::const_iterator i = paths.begin(); i != paths.end(); ++i)
    {
        for(vector<string>::const_iterator j = includePaths.begin(); j != includePaths.end(); ++j)
        {            
            if(i->compare(0, j->length(), *j) == 0)
            {
                string s = i->substr(j->length() + 1); // + 1 for the '/'
                if(s.size() < result.size())
                {
                    result = s;
                }
            }
        }
        
        //
        // If the path has been already shortened no need to test
        // with canonical path.
        //
        if(result != path)
        {
            break;
        }
    }

    result = normalizePath(result); // Normalize the result.

    string::size_type pos;
    if((pos = result.rfind('.')) != string::npos)
    {
        result.erase(pos);
    }

    return result;
}

void
Slice::emitError(const string& file, int line, const string& message)
{
    if(!file.empty())
    {
        consoleErr << file;
        if(line != -1)
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
    consoleErr << message << endl;
}

void
Slice::emitWarning(const string& file, int line, const string& message)
{
    if(!file.empty())
    {
        consoleErr << file;
        if(line != -1)
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
    consoleErr << "warning: " << message << endl;
}

void
Slice::emitError(const string& file, const std::string& line, const string& message)
{
    if(!file.empty())
    {
        consoleErr << file;
        if(!line.empty())
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
    consoleErr << message << endl;
}

void
Slice::emitWarning(const string& file, const std::string& line, const string& message)
{
    if(!file.empty())
    {
        consoleErr << file;
        if(!line.empty())
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
    static const char* messages[] =
    {
        "Converted [CR+LF] to [LF]",
        "no newline, supplemented newline",
        0
    };

    static const string warningPrefix = "warning:";
    static const string fromPrefix = "from";
    static const string separators = "\n\t ";

    vector<string> in;
    string::size_type start = 0;
    string::size_type end;
    while((end = message.find('\n', start)) != string::npos)
    {
        in.push_back(message.substr(start, end - start));
        start = end + 1;
    }
    vector<string> out;
    bool skipped;
    for(vector<string>::const_iterator i = in.begin(); i != in.end(); i++)
    {
        skipped = false;

        if(i->find(warningPrefix) != string::npos)
        {
            for(int j = 0; messages[j] != 0; ++j)
            {
                if(i->find(messages[j]) != string::npos)
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
                    while(i != in.end())
                    {
                        string token = *i;
                        string::size_type index = token.find_first_not_of(separators);
                        if(index != string::npos)
                        {
                            token = token.substr(index);
                        }
                        if(token.find(fromPrefix) != 0)
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
            if(i == in.end())
            {
                break;
            }
        }
        if(!skipped)
        {
            out.push_back(*i + "\n");
        }
    }
    return out;
}

void
Slice::printGeneratedHeader(IceUtilInternal::Output& out, const string& path, const string& comment)
{
    //
    // Get only the file name part of the given path.
    //
    string file = path;
    size_t pos = file.find_last_of("/\\");
    if(string::npos != pos)
    {
        file = file.substr(pos + 1);
    }

    out << comment << " <auto-generated>\n";
    out << comment << "\n";
    out << comment << " Generated from file `" << file << "'" << "\n";
    out << comment << "\n";
    out << comment << " Warning: do not edit this file." << "\n";
    out << comment << "\n";
    out << comment << " </auto-generated>\n";
    out << comment << "\n";
}

void
Slice::writeDependencies(const string& dependencies, const string& dependFile)
{
    if(dependFile.empty())
    {
        consoleOut << dependencies << flush;
    }
    else
    {
        ofstream of(IceUtilInternal::streamFilename(dependFile).c_str()); // dependFile is a UTF-8 string
        if(!of)
        {
            ostringstream os;
            os << "cannot open file `" << dependFile << "': " << strerror(errno);
            throw Slice::FileException(__FILE__, __LINE__, os.str());
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
    for(int i = 0; i < argc; i++)
    {
        args.push_back(IceUtil::wstringToString(argv[i]));
    }
    return args;
}
#else
vector<string>
Slice::argvToArgs(int argc, char* argv[])
{
    vector<string> args;
    for(int i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return args;
}
#endif
