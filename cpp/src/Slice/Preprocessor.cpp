// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/Preprocessor.h>

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
    
    cmd += " " + _args + " " + _fileName;

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
    
    cmd += " -M " + _args + " " + _fileName;

#ifdef _WIN32
    FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
    FILE* cppHandle = popen(cmd.c_str(), "r");
#endif

    switch(lang)
    {
        case CPlusPlus:
	{
	    char buf[1024];
	    while(fgets(buf, sizeof(buf), cppHandle) != NULL)
	    {
	        fputs(buf, stdout);
	    }
	    break;
	}
	case Java:
	{
	    //
	    // Remove leading "<file>.cpp:" and make the first following
	    // file the dependent. For example, "x.cpp: x.ice y.ice"
	    // is rewritten as "x.ice: y.ice".
	    //
	    char buf[1024];
	    while(fgets(buf, sizeof(buf), cppHandle) != NULL)
	    {
		bool needNewline = false;
		char *p = buf;
		char* pos = strchr(buf, ':');			// Find first colon.
		if(pos != NULL)
		{
		    *pos = '\0';				// Find last dot preceding colon.
		    char* pos2 = strrchr(buf, '.');
		    *pos = ':';
		    if(pos2 != NULL)
		    {
			if(strncmp(pos2, ".cpp:", 5) == 0)	// Check for ".cpp:".
			{
			    while(isspace(*++pos))		// Move pos to first non-space char following colon.
			    {
			        ;
			    }
			    char* pos2 = pos;
			    while(!isspace(*++pos2))		// Move pos2 to first space char following pos.
			    {
			        ;
			    }
			    *pos2 = '\0';
			    while(isspace(*++pos2))		// Move pos2 to next non-space char.
			    {
			        ;
			    }
			    if(*pos2 == '\0')
			    {
			        continue;
			    }
			    needNewline = *pos2 == '\0';	// Don't lose trailing newline.
			    fputs(pos, stdout);			// Write first file name following colon.
			    fputs(": ", stdout);
			    p = pos2;
			}
		    }
		}
		fputs(p, stdout);
		if(needNewline)
		{
		    fputs("\n", stdout);
		}
	    }
	    break;
	}
	case CSharp:
	{
	    //
	    // Change .cpp suffix to .cs suffix.
	    //
	    char buf[1024];
	    while(fgets(buf, sizeof(buf), cppHandle) != NULL)
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

#ifndef WIN32
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
#ifndef WIN32
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
#ifndef WIN32
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
