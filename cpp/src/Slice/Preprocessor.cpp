// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/Preprocessor.h>
#include <IceUtil/StringUtil.h>
#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32

#   include <sys/wait.h>
#endif

using namespace std;
using namespace Slice;

Slice::Preprocessor::Preprocessor(const string& path, const string& fileName, const string& args) :
    _path(path),
    _fileName(fileName),
    _args(args),
    _cppHandle(0)
{
}

Slice::Preprocessor::~Preprocessor()
{
    if(_cppHandle)
    {
        close();
    }
}

string
Slice::Preprocessor::getBaseName()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if(pos != string::npos)
    {
        base.erase(pos);
    }
    return base;
}

string
Slice::Preprocessor::addQuotes(const string& arg)
{
    //
    // Add quotes around the given argument to ensure that arguments
    // with spaces will be preserved as a single argument. We also
    // escape the "\" character to ensure that we don't end up with a
    // \" at the end of the string.
    //
    return "\"" + IceUtil::escapeString(arg, "\\") + "\"";
}

string
Slice::Preprocessor::normalizeIncludePath(const string& path)
{
    string result = path;

    replace(result.begin(), result.end(), '\\', '/');

    string::size_type pos;
    while((pos = result.find("//")) != string::npos)
    {
        result.replace(pos, 2, "/");
    }

    if(result == "/" || result.size() == 3 && isalpha(result[0]) && result[1] == ':' && result[2] == '/')
    {
	return result;
    }
    
    if(result.size() > 1 && result[result.size() - 1] == '/')
    {
	result.erase(result.size() - 1);
    }

    return "\"" + result + "\"";
}

FILE*
Slice::Preprocessor::preprocess(bool keepComments)
{
    if(!checkInputFile())
    {
        return 0;
    }

    string cmd = searchIceCpp();

    if(cmd.empty())
    {
        return 0;
    }

    if(keepComments)
    {
        cmd += " -C";
    }
    
    cmd += " " + _args + " \"" + _fileName + "\"";

    //
    // Open a pipe for reading to redirect icecpp output to _cppHandle.
    //
#ifdef _WIN32
    _cppHandle = _popen(cmd.c_str(), "r");
#else
    _cppHandle = popen(cmd.c_str(), "r");
#endif
    return _cppHandle;
}

void
Slice::Preprocessor::printMakefileDependencies(Language lang)
{
    if(!checkInputFile())
    {
        return;
    }

    string cmd = searchIceCpp();
    
    if(cmd.empty())
    {
        return;
    }
    
    cmd += " -M " + _args + " \"" + _fileName + "\"";

#ifdef _WIN32
    FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
    FILE* cppHandle = popen(cmd.c_str(), "r");
#endif

    /*
     * icecpp emits dependencies in any of the following formats, depending on the
     * length of the filenames:
     *
     * x.cpp: /path/x.ice /path/y.ice
     *
     * x.cpp: /path/x.ice \ 
     *  /path/y.ice
     *
     * x.cpp: /path/x.ice /path/y.ice \ 
     *  /path/z.ice
     *
     * x.cpp: \ 
     *  /path/x.ice
     *
     * x.cpp: \ 
     *  /path/x.ice \ 
     *  /path/y.ice
     *
     * Spaces embedded within filenames are escaped with a backslash. Note that
     * Windows filenames may contain colons.
     *
     */
    switch(lang)
    {
        case CPlusPlus:
        {
            char buf[1024];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                fputs(buf, stdout);
            }
            break;
        }
        case Java:
        {
            //
            // We want to shift the files left one position, so that
            // "x.cpp: x.ice y.ice" becomes "x.ice: y.ice".
            //
            // Since the pipe input can be returned a line at a time, we collect
            // all of the output into one string before manipulating it.
            //
            string deps;
            char buf[1024];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                deps.append(buf, strlen(buf));
            }

            //
            // Remove the first file.
            //
            string::size_type start = deps.find(".cpp:");
            assert(start != string::npos);
            start = deps.find_first_not_of(" \t\r\n\\", start + 5); // Skip to beginning of next file.
            assert(start != string::npos);
            deps.erase(0, start);

            //
            // Find end of next file.
            //
            string::size_type pos = 0;
            while((pos = deps.find_first_of(" :\t\r\n\\", pos + 1)) != string::npos)
            {
                if(deps[pos] == ':')
                {
                    deps.insert(pos, 1, '\\'); // Escape colons.
                    ++pos;
                }
                else if(deps[pos] == '\\') // Ignore escaped characters.
                {
                    ++pos;
                }
                else
                {
                    break;
                }
            }

            if(pos == string::npos)
            {
                deps.append(":");
            }
            else
            {
                deps.insert(pos, 1, ':');
            }

            fputs(deps.c_str(), stdout);
            break;
        }
        case CSharp:
        {
            //
            // Change .cpp suffix to .cs suffix.
            //
            char buf[1024];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                char* dot;
                char* colon = strchr(buf, ':');
                if(colon != NULL)
                {
                    *colon = '\0';
                    dot = strrchr(buf, '.');
                    *colon = ':';
                    if(dot != NULL)
                    {
                        if(strncmp(dot, ".cpp:", 5) == 0)
                        {
                            *dot = '\0';
                            fputs(buf, stdout);
                            fputs(".cs", stdout);
                            fputs(colon, stdout);
                            continue;
                        }
                    }
                }
                fputs(buf, stdout);
            }
            break;
        }
        case VisualBasic:
        {
            //
            // Change .cpp suffix to .vb suffix.
            //
            char buf[1024];
            while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
            {
                char* dot;
                char* colon = strchr(buf, ':');
                if(colon != NULL)
                {
                    *colon = '\0';
                    dot = strrchr(buf, '.');
                    *colon = ':';
                    if(dot != NULL)
                    {
                        if(strncmp(dot, ".cpp:", 5) == 0)
                        {
                            *dot = '\0';
                            fputs(buf, stdout);
                            fputs(".vb", stdout);
                            fputs(colon, stdout);
                            continue;
                        }
                    }
                }
                fputs(buf, stdout);
            }
            break;
        }
        default:
        {
            abort();
            break;
        }
    }
}

bool
Slice::Preprocessor::close()
{
    assert(_cppHandle);

#ifndef _WIN32
    int status = pclose(_cppHandle);
    _cppHandle = 0;

    if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        return false;
    }
#else
    int status = _pclose(_cppHandle);
    _cppHandle = 0;

    if(status != 0)
    {
        return false;
    }
#endif
    
    return true;
}

bool
Slice::Preprocessor::checkInputFile()
{
    string base(_fileName);
    string suffix;
    string::size_type pos = base.rfind('.');
    if(pos != string::npos)
    {
        suffix = base.substr(pos);
        transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
    }
    if(suffix != ".ice")
    {
        cerr << _path << ": input files must end with `.ice'" << endl;
        return false;
    }
    
    ifstream test(_fileName.c_str());
    if(!test)
    {
        cerr << _path << ": can't open `" << _fileName << "' for reading" << endl;
        return false;
    }
    test.close();

    return true;
}

string
Slice::Preprocessor::searchIceCpp()
{
#ifndef _WIN32
    const char* icecpp = "icecpp";
#else
    const char* icecpp = "icecpp.exe";
#endif

    string::size_type pos = _path.find_last_of("/\\");
    if(pos != string::npos)
    {
        string path = _path.substr(0, pos + 1);
        path += icecpp;

        struct stat st;
        if(stat(path.c_str(), &st) == 0)
        {
#ifndef _WIN32
            if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
#else
            if(st.st_mode & (S_IEXEC))
#endif
            {
                return path;
            }
        }
    }

    return icecpp;
}
