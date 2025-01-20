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

    /**
     * An interface to associate implicit contexts with communicators. When you make a remote invocation without an
     * explicit context parameter, Ice uses the per-proxy context (if any) combined with the
     * <code>ImplicitContext</code> associated with the communicator. Ice provides several implementations of
     * <code>ImplicitContext</code>. The implementation used depends on the value of the
     * <code>Ice.ImplicitContext</code> property. <dl> <dt><code>None</code> (default)</dt> <dd>No implicit context at
     * all.</dd> <dt><code>PerThread</code></dt> <dd>The implementation maintains a context per thread.</dd>
     * <dt><code>Shared</code></dt>
     * <dd>The implementation maintains a single context shared by all threads.</dd>
     * </dl>
     * <code>ImplicitContext</code> also provides a number of operations to create, update or retrieve an entry in the
     * underlying context without first retrieving a copy of the entire context. These operations correspond to a subset
     * of the <code>java.util.Map</code> methods, with <code>java.lang.Object</code> replaced by <code>string</code> and
     * null replaced by the empty-string.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ImplicitContext final
    {
    public:
        /**
         * Get a copy of the underlying context.
         * @return A copy of the underlying context.
         */
        [[nodiscard]] Context getContext() const;

        /**
         * Set the underlying context.
         * @param newContext The new context.
         */
        void setContext(const Context& newContext);

        /**
         * Check if this key has an associated value in the underlying context.
         * @param key The key.
         * @return True if the key has an associated value, False otherwise.
         */
        [[nodiscard]] bool containsKey(std::string_view key) const;

        /**
         * Get the value associated with the given key in the underlying context. Returns an empty string if no value is
         * associated with the key. {@link #containsKey} allows you to distinguish between an empty-string value and no
         * value at all.
         * @param key The key.
         * @return The value associated with the key.
         */
        [[nodiscard]] std::string get(std::string_view key) const;

        /**
         * Create or update a key/value entry in the underlying context.
         * @param key The key.
         * @param value The value.
         * @return The previous value associated with the key, if any.
         */
        std::string put(std::string key, std::string value);

        /**
         * Remove the entry for the given key in the underlying context.
         * @param key The key.
         * @return The value associated with the key, if any.
         */
        std::string remove(std::string_view key);

        /**
         * Marshals the underlying context plus the given context. Entries in the given context overwrite entries in the
         * underlying context.
         * @param context The context to write to the output stream.
         * @param os The output stream.
         */
        void write(const Context& context, Ice::OutputStream* os) const;

        /**
         * Combines the underlying context plus the given context. Entries in the given context overwrite entries in the
         * underlying context.
         * @param context The context to combine with the underlying context.
         * @param combined The combined context.
         */
        void combine(const Context& context, Context& combined) const;

    private:
        mutable std::mutex _mutex;
        Context _context;
    };

    using ImplicitContextPtr = std::shared_ptr<ImplicitContext>;
}

#endif
