// Copyright (c) ZeroC, Inc.

#include "FileTracker.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"

#include <cassert>

using namespace IceInternal;
using namespace std;

static Slice::FileTrackerPtr Instance;

Slice::FileTracker::FileTracker() : _curr(_generated.end()) {}

Slice::FileTracker::~FileTracker() = default;

// The file tracker is not supposed to be thread safe.
Slice::FileTrackerPtr
Slice::FileTracker::instance()
{
    if (!Instance)
    {
        Instance = make_shared<FileTracker>();
    }
    return Instance;
}

void
Slice::FileTracker::setSource(const string& source)
{
    _source = source;
    pair<map<string, list<string>>::iterator, bool> p = _generated.insert(make_pair(source, list<string>()));
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
    _files.emplace_front(file, false);
    if (_curr != _generated.end())
    {
        _curr->second.push_back(file);
    }
}

void
Slice::FileTracker::addDirectory(const string& dir)
{
    _files.emplace_front(dir, true);
}

void
Slice::FileTracker::cleanup()
{
    for (const auto& file : _files)
    {
        if (!file.second)
        {
            IceInternal::unlink(file.first);
        }
        else
        {
            IceInternal::rmdir(file.first);
        }
    }
}

void
Slice::FileTracker::dumpxml()
{
    consoleOut << R"(<?xml version="1.0" encoding="UTF-8"?>)" << endl;
    consoleOut << "<generated>";
    for (const auto& p : _generated)
    {
        if (!p.second.empty())
        {
            consoleOut << endl << "  <source name=\"" << p.first << "\">";
            for (const auto& q : p.second)
            {
                consoleOut << endl << "    <file name=\"" << q << "\"/>";
            }
            consoleOut << endl << "  </source>";
        }
    }
    consoleOut << endl << "</generated>" << endl;
}
