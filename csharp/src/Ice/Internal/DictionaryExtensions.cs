// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

/// <summary>Extension methods for dictionaries.</summary>
public static class DictionaryExtensions
{
    /// <summary>Checks if two dictionaries are equal. The order of the elements in the dictionaries does not
    /// matter.</summary>
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
