//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IMPLICIT_CONTEXT_I_H
#define ICE_IMPLICIT_CONTEXT_I_H

#include <Ice/ImplicitContext.h>

#include <mutex>
#include <string>

namespace Ice
{

//
// The base class for all ImplicitContext implementations
//
class ImplicitContextI;
using ImplicitContextIPtr = std::shared_ptr<ImplicitContextI>;

class ImplicitContextI final : public ImplicitContext
{
public:
    //
    // Marshals the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    void write(const Context&, Ice::OutputStream*) const;

    //
    // Combines the underlying context plus the given context
    // (entries in the given context overwrite entries in
    // the underlying context)
    //
    void combine(const Context&, Context&) const;

    Ice::Context getContext() const final;

    void setContext(const Context& newContext) final;

    bool containsKey(const std::string& key) const final;

    std::string get(const std::string& key) const final;

    std::string put(const std::string& key, const std::string& value) final;

    std::string remove(const std::string& key) final;

private:

    Context _context;
    mutable std::mutex _mutex;
};

using ImplicitContextIPtr = std::shared_ptr<ImplicitContextI>;

}
#endif
