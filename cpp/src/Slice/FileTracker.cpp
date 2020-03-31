//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/FileTracker.h>
#include <IceUtil/ConsoleUtil.h>
#include <IceUtil/FileUtil.h>

using namespace IceUtilInternal;
using namespace std;

Slice::FileException::FileException(const char* file, int line, const string& r) :
    IceUtil::Exception(file, line),
    _reason(r)
{
}

string
Slice::FileException::ice_id() const
{
    return "::Slice::FileException";
}

void
Slice::FileException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": " << _reason;
}

Slice::FileException*
Slice::FileException::ice_cloneImpl() const
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

Slice::FileTracker::FileTracker() :
    _curr(_generated.end())
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
Slice::FileTracker::setSource(const string& source)
{
    _source = source;
    pair<map<string, list<string> >::iterator, bool> p = _generated.insert(make_pair(source, list<string>()));
    assert(p.second);
    _curr = p.first;
}

void
Slice::FileTracker::error()
{
    assert(_curr != _generated.end());
    _generated.erase(_curr);
    _curr = _generated.end();
}

void
Slice::FileTracker::addFile(const string& file)
{
    _files.push_front(make_pair(file, false));
    if(_curr != _generated.end())
    {
        _curr->second.push_back(file);
    }
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
            IceUtilInternal::unlink(p->first);
        }
        else
        {
            IceUtilInternal::rmdir(p->first);
        }
    }
}

void
Slice::FileTracker::dumpxml()
{
    consoleOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    consoleOut << "<generated>";
    for(map<string, list<string> >::const_iterator p = _generated.begin(); p != _generated.end(); ++p)
    {
        if(!p->second.empty())
        {
            consoleOut << endl << "  <source name=\"" << p->first << "\">";
            for(list<string>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
            {
                consoleOut << endl << "    <file name=\"" << *q << "\"/>";
            }
            consoleOut << endl << "  </source>";
        }
    }
    consoleOut << endl << "</generated>" << endl;
}
