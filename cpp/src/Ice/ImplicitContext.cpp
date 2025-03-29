// Copyright (c) ZeroC, Inc.

#include "Ice/ImplicitContext.h"
#include "Ice/StreamHelpers.h"

using namespace std;
using namespace Ice;

Context
ImplicitContext::getContext() const
{
    lock_guard lock(_mutex);
    return _context;
}

void
ImplicitContext::setContext(Context newContext)
{
    lock_guard lock(_mutex);
    _context = std::move(newContext);
}

bool
ImplicitContext::containsKey(string_view key) const
{
    lock_guard lock(_mutex);
    return _context.find(key) != _context.end();
}

string
ImplicitContext::get(string_view key) const
{
    lock_guard lock(_mutex);
    auto p = _context.find(key);
    if (p == _context.end())
    {
        return "";
    }
    return p->second;
}

string
ImplicitContext::put(string key, string value)
{
    lock_guard lock(_mutex);
    string& val = _context[std::move(key)];

    string oldVal = val;
    val = std::move(value);
    return oldVal;
}

string
ImplicitContext::remove(string_view key)
{
    lock_guard lock(_mutex);
    auto p = _context.find(key);
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
ImplicitContext::write(const Context& context, Ice::OutputStream* os) const
{
    unique_lock lock(_mutex);
    if (context.size() == 0)
    {
        os->write(_context);
    }
    else if (_context.size() == 0)
    {
        lock.unlock();
        os->write(context);
    }
    else
    {
        Context combined = context;
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
