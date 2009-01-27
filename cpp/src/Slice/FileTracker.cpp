// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/FileTracker.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;

Slice::FileException::FileException(const char* file, int line, const string& r) :
    IceUtil::Exception(file, line),
    _reason(r)
{
}

Slice::FileException::~FileException() throw()
{
}

const char* Slice::FileException::_name = "Slice::FileException";

string
Slice::FileException::ice_name() const
{
    return _name;
}

void
Slice::FileException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": " << _reason;
}

IceUtil::Exception*
Slice::FileException::ice_clone() const
{
    return new FileException(*this);
}

void
Slice::FileException::ice_throw() const
{
    throw *this;
}

string
Slice::FileException::reason() const
{
    return _reason;
}


static Slice::FileTrackerPtr Instance;

Slice::FileTracker::FileTracker()
{
}

Slice::FileTracker::~FileTracker()
{
}

// The file tracker is not supposed to be thread safe.
Slice::FileTrackerPtr
Slice::FileTracker::instance()
{
    if(!Instance)
    {
        Instance = new FileTracker();
    }
    return Instance;
}

void
Slice::FileTracker::addFile(const string& file)
{
    _files.push_front(make_pair(file, false));
}

void
Slice::FileTracker::addDirectory(const string& dir)
{
    _files.push_front(make_pair(dir, true));
}

void
Slice::FileTracker::cleanup()
{
    for(list<pair<string, bool> >::const_iterator p = _files.begin(); p != _files.end(); ++p)
    {
        if(!p->second)
        {
#ifdef _WIN32
            _unlink(p->first.c_str());
#else       
            unlink(p->first.c_str());
#endif
        }
        else
        {
#ifdef _WIN32
            _rmdir(p->first.c_str());
#else       
            rmdir(p->first.c_str());
#endif
        }
    }
}
