// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public static class DictionaryExtensions
    {
        /// <summary>Compares two dictionaries for equality. Enumerable.SequenceEqual provides the equivalent
        /// functionality for sequences.</summary>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <returns>True if the two dictionaries have the exact same entries using the value's default equality
        /// comparison; otherwise, false.</returns>
        public static bool DictionaryEqual<TKey, TValue>(
            this IReadOnlyDictionary<TKey, TValue> lhs,
            IReadOnlyDictionary<TKey, TValue> rhs) where TKey : notnull =>
            DictionaryEqual(lhs, rhs, EqualityComparer<TValue>.Default);

        /// <summary>Compares two dictionaries for equality. Enumerable.SequenceEqual provides the equivalent
        /// functionality for sequences.</summary>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <param name="valueComparer">The comparer used to compare values for equality.</param>
        /// <returns>True if the two dictionaries have the exact same entries; otherwise, false.</returns>
        public static bool DictionaryEqual<TKey, TValue>(
            this IReadOnlyDictionary<TKey, TValue> lhs,
            IReadOnlyDictionary<TKey, TValue> rhs,
            IEqualityComparer<TValue> valueComparer) where TKey : notnull
        {
            if (lhs == null)
            {
                throw new ArgumentNullException($"{nameof(lhs)} is null", nameof(lhs));
            }
            if (rhs == null)
            {
                throw new ArgumentNullException($"{nameof(rhs)} is null", nameof(rhs));
            }

            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs.Count != rhs.Count)
            {
                return false;
            }

            foreach (KeyValuePair<TKey, TValue> entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out TValue value) || !valueComparer.Equals(entry.Value, value))
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>Computes the hash code for a dictionary.</summary>
        /// <param name="dict">The dictionary.</param>
        /// <returns>A hash code computed using the dictionary's entries.</returns>
        public static int GetDictionaryHashCode<TKey, TValue>(this IReadOnlyDictionary<TKey, TValue> dict)
            where TKey : notnull
        {
            var hash = new System.HashCode();
            foreach (KeyValuePair<TKey, TValue> e in dict)
            {
                hash.Add(e.Key);
                hash.Add(e.Value);
            }
            return hash.ToHashCode();
        }
    }
}
