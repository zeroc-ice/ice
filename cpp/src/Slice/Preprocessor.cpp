// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
Slice::Preprocessor::printMakefileDependencies()
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
    
    cmd += " -M " + _args + " " +_fileName;

    //
    // Open a pipe for writing to redirect icecpp output to the standard output.
    //
#ifndef _WIN32
    FILE* cppHandle = popen(cmd.c_str(), "w");
    pclose(cppHandle);
#else
    FILE* cppHandle = _popen(cmd.c_str(), "w");
    _pclose(cppHandle);
#endif
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
	cerr << _path << ": can't open `" << _fileName << "' for reading: " << strerror(errno) << endl;
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
