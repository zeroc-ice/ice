//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    public interface IImplicitContext
    {
        /// <summary>
        /// Get a copy of the underlying context.
        /// </summary>
        /// <returns>A copy of the underlying context.</returns>
        Dictionary<string, string> GetContext();

        /// <summary>
        /// Set the underlying context.
        /// </summary>
        /// <param name="newContext">The new context.</param>
        void SetContext(Dictionary<string, string> newContext);

        /// <summary>
        /// Check if this key has an associated value in the underlying context.
        /// </summary>
        /// <param name="key">The key.
        ///
        /// </param>
        /// <returns>True if the key has an associated value, False otherwise.</returns>
        bool ContainsKey(string key);

        /// <summary>
        /// Get the value associated with the given key in the underlying context.
        /// Returns an empty string if no value is associated with the key.
        /// containsKey allows you to distinguish between an empty-string value and
        /// no value at all.
        ///
        /// </summary>
        /// <param name="key">The key.
        ///
        /// </param>
        /// <returns>The value associated with the key.</returns>
        string Get(string key);

        /// <summary>
        /// Create or update a key/value entry in the underlying context.
        /// </summary>
        /// <param name="key">The key.
        ///
        /// </param>
        /// <param name="value">The value.
        ///
        /// </param>
        /// <returns>The previous value associated with the key, if any.</returns>
        string Put(string key, string value);

        /// <summary>
        /// Remove the entry for the given key in the underlying context.
        /// </summary>
        /// <param name="key">The key.
        ///
        /// </param>
        /// <returns>The value associated with the key, if any.</returns>
        string Remove(string key);
    }
}
