// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/FileTracker.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
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

Slice::FileException*
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
Slice::FileTracker::setOutput(const string& output, bool error)
{
    assert(!_source.empty());
    _errors.insert(make_pair(_source, output));
    if(error)
    {
        _generated.erase(_curr);
        _curr = _generated.end();
    }
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

void
Slice::FileTracker::dumpxml()
{
    cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    cout << "<generated>" << endl;
    for(map<string, string>::const_iterator p = _errors.begin(); p != _errors.end(); ++p)
    {
        cout << "  <source name=\"" << p->first << "\"";

        map<string, list<string> >::const_iterator q = _generated.find(p->first);
        if(q == _generated.end())
        {
            cout << " error=\"true\">" << endl;
        }
        else
        {
            cout << ">" << endl;
            for(list<string>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
            {
                cout << "    <file name=\"" << *r << "\"/>" << endl;
            }
        }
        cout << "    <output>" << escape(p->second) << "</output>" << endl;
        cout << "  </source>" << endl;
    }
    cout << "</generated>" << endl;
}

string
Slice::FileTracker::escape(const string& str) const
{
    ostringstream ostr;

    for(string::const_iterator p = str.begin(); p != str.end(); ++p)
    {
        switch(*p)
        {
        case '<':
            ostr << "&lt;";
            break;
        case '>':
            ostr << "&gt;";
            break;
        case '&':
            ostr << "&amp;";
            break;
        case '"':
            ostr << "&quot;";
            break;
        default:
            ostr << *p;
            break;
        }
    }

    return ostr.str();
}
