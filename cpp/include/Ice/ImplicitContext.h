// Copyright (c) ZeroC, Inc.

#ifndef ICE_IMPLICIT_CONTEXT_H
#define ICE_IMPLICIT_CONTEXT_H

#include "Config.h"
#include "Ice/Context.h"

#include <memory>
#include <mutex>
#include <string>

namespace Ice
{
    class OutputStream;

    /// Represents the request context associated with a communicator.
    /// When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy request
    /// context (if any) combined with the `ImplicitContext` associated with your communicator. The property
    /// `Ice.ImplicitContext` controls if your communicator has an associated implicit context, and when it does,
    /// whether this implicit context is per-thread or shared by all threads.
    /// @headerfile Ice/Ice.h
    class ICE_API ImplicitContext final
    {
    public:
        /// Gets a copy of the request context maintained by this object.
        /// @return A copy of the request context.
        [[nodiscard]] Context getContext() const;

        /// Sets the request context.
        /// @param newContext The new request context.
        void setContext(Context newContext);

        /// Checks if this key has an associated value in the request context.
        /// @param key The key.
        /// @return `true` if the key has an associated value, `false` otherwise.
        [[nodiscard]] bool containsKey(std::string_view key) const;

        /// Gets the value associated with the specified key in the request context.
        /// @param key The key.
        /// @return The value associated with the key, or the empty string if no value is associated with the key.
        [[nodiscard]] std::string get(std::string_view key) const;

        /// Creates or updates a key/value entry in the request context.
        /// @param key The key.
        /// @param value The value.
        /// @return The previous value associated with the key, if any.
        std::string put(std::string key, std::string value);

        /// Removes the entry for the specified key in the request context.
        /// @param key The key.
        /// @return The value associated with the key, if any.
        std::string remove(std::string_view key);

        /// Marshals this request context plus the specified request context. Entries in @p context overwrite entries
        /// in this request context.
        /// @param context The request context to write to the output stream.
        /// @param os The output stream.
        void write(const Context& context, Ice::OutputStream* os) const;

        /// Combines this request context plus the specified request context. Entries in @p context overwrite entries in
        /// this request context.
        /// @param context The request context to combine with this request context.
        /// @param combined The combined request context.
        void combine(const Context& context, Context& combined) const;

    private:
        mutable std::mutex _mutex;
        Context _context;
    };

    /// A shared pointer to an ImplicitContext.
    using ImplicitContextPtr = std::shared_ptr<ImplicitContext>;
}

#endif
