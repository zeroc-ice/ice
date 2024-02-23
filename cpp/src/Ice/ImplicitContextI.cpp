//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ImplicitContextI.h>
#include <Ice/OutputStream.h>
#include <Ice/Object.h>

using namespace std;
using namespace Ice;

Context
ImplicitContextI::getContext() const
{
    lock_guard lock(_mutex);
    return _context;
}

void
ImplicitContextI::setContext(const Context& newContext)
{
    lock_guard lock(_mutex);
    _context = newContext;
}

bool
ImplicitContextI::containsKey(const string& k) const
{
    lock_guard lock(_mutex);
    Context::const_iterator p = _context.find(k);
    return p != _context.end();
}

string
ImplicitContextI::get(const string& k) const
{
    lock_guard lock(_mutex);
    Context::const_iterator p = _context.find(k);
    if(p == _context.end())
    {
        return "";
    }
    return p->second;
}

string
ImplicitContextI::put(const string& k, const string& v)
{
    lock_guard lock(_mutex);
    string& val = _context[k];

    string oldVal = val;
    val = v;
    return oldVal;
}

string
ImplicitContextI::remove(const string& k)
{
    lock_guard lock(_mutex);
    Context::iterator p = _context.find(k);
    if(p == _context.end())
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
ImplicitContextI::write(const Context& proxyCtx, ::Ice::OutputStream* s) const
{
    unique_lock lock(_mutex);
    if(proxyCtx.size() == 0)
    {
        s->write(_context);
    }
    else if(_context.size() == 0)
    {
        lock.unlock();
        s->write(proxyCtx);
    }
    else
    {
        Context combined = proxyCtx;
        combined.insert(_context.begin(), _context.end());
        lock.unlock();
        s->write(combined);
    }
}

void
ImplicitContextI::combine(const Context& proxyCtx, Context& ctx) const
{
    lock_guard lock(_mutex);
    if(proxyCtx.size() == 0)
    {
        ctx = _context;
    }
    else if(_context.size() == 0)
    {
        ctx = proxyCtx;
    }
    else
    {
        ctx = proxyCtx;
        ctx.insert(_context.begin(), _context.end());
    }
}
