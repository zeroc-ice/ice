// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

/// <summary>
/// Extension methods for dictionaries.
/// </summary>
public static class DictionaryExtensions
{
    /// <summary>
    /// Checks if two dictionaries are equal. The order of the elements in the dictionaries does not matter.
    /// </summary>
    /// <typeparam name="TKey">The type of the keys in the dictionaries.</typeparam>
    /// <typeparam name="TValue">The type of the values in the dictionaries.</typeparam>
    /// <param name="lhs">The first dictionary to compare.</param>
    /// <param name="rhs">The second dictionary to compare.</param>
    /// <param name="valueComparer">The comparer to use to compare the values in the dictionaries or null to use the
    /// value's default equality comparer.</param>
    /// <returns><see langword="true"/> if the dictionaries are equal; <see langword="false"/>, otherwise.</returns>
    public static bool DictionaryEqual<TKey, TValue>(
        this IReadOnlyDictionary<TKey, TValue> lhs,
        IReadOnlyDictionary<TKey, TValue> rhs,
        IEqualityComparer<TValue>? valueComparer = null)
    {
        if (rhs.Count != lhs.Count)
        {
            return false;
        }

        valueComparer ??= EqualityComparer<TValue>.Default;

        foreach ((TKey key, TValue value) in lhs)
        {
            if (!rhs.TryGetValue(key, out TValue? otherValue) || !valueComparer.Equals(value, otherValue))
            {
                return false;
            }
        }

        return true;
    }
}
