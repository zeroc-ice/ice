//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Test
{
    public static class Collections
    {
        public static bool Equals<TKey, TValue>(IReadOnlyDictionary<TKey, TValue>? lhs,
                                                IReadOnlyDictionary<TKey, TValue>? rhs) where TKey : notnull
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<TValue> comparer = EqualityComparer<TValue>.Default;
            foreach (KeyValuePair<TKey, TValue> entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out TValue value) || !comparer.Equals(entry.Value, value))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool Equals<T>(T[]? arr1, T[]? arr2)
        {
            if (ReferenceEquals(arr1, arr2))
            {
                return true;
            }

            if (arr1 == null || arr2 == null || arr1.Length != arr2.Length)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            for (int i = 0; i < arr1.Length; ++i)
            {
                if (!comparer.Equals(arr1[i], arr2[i]))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool Equals<T>(IReadOnlyList<T>? lhs, IReadOnlyList<T>? rhs) =>
            Equals(lhs as IReadOnlyCollection<T>, rhs as IReadOnlyCollection<T>);

        public static bool Equals<T>(LinkedList<T>? lhs, LinkedList<T>? rhs) =>
            Equals(lhs as IReadOnlyCollection<T>, rhs as IReadOnlyCollection<T>);

        private static bool Equals<T>(IReadOnlyCollection<T>? lhs, IReadOnlyCollection<T>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            IEnumerator<T> i = lhs.GetEnumerator();
            IEnumerator<T> j = rhs.GetEnumerator();
            while (i.MoveNext())
            {
                if (!j.MoveNext() || !comparer.Equals(i.Current, j.Current))
                {
                    return false;
                }
            }
            return true;
        }
    }
}
