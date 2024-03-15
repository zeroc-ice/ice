//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ImplicitContext.h>

using namespace std;
using namespace Ice;

Context
ImplicitContext::getContext() const
{
    lock_guard lock(_mutex);
    return _context;
}

void
ImplicitContext::setContext(const Context& newContext)
{
    lock_guard lock(_mutex);
    _context = newContext;
}

bool
ImplicitContext::containsKey(const string& key) const
{
    lock_guard lock(_mutex);
    return _context.find(key) != _context.end();
}

string
ImplicitContext::get(const string& key) const
{
    lock_guard lock(_mutex);
    Context::const_iterator p = _context.find(key);
    if (p == _context.end())
    {
        return "";
    }
    return p->second;
}

string
ImplicitContext::put(const string& key, const string& value)
{
    lock_guard lock(_mutex);
    string& val = _context[key];

    string oldVal = val;
    val = value;
    return oldVal;
}

string
ImplicitContext::remove(const string& key)
{
    lock_guard lock(_mutex);
    Context::iterator p = _context.find(key);
    if (p == _context.end())
    {
        return "";
    }
    else
    {
        string oldVal = p->second;
        _context.erase(p);
        return oldVal;
    }
}

void
ImplicitContext::write(const Context& contex, ::Ice::OutputStream* os) const
{
    unique_lock lock(_mutex);
    if (contex.size() == 0)
    {
        os->write(_context);
    }
    else if (_context.size() == 0)
    {
        lock.unlock();
        os->write(contex);
    }
    else
    {
        Context combined = contex;
        combined.insert(_context.begin(), _context.end());
        lock.unlock();
        os->write(combined);
    }
}

void
ImplicitContext::combine(const Context& context, Context& combined) const
{
    lock_guard lock(_mutex);
    if (context.size() == 0)
    {
        combined = _context;
    }
    else if (_context.size() == 0)
    {
        combined = context;
    }
    else
    {
        combined = context;
        combined.insert(_context.begin(), _context.end());
    }
}
