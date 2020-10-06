// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace ZeroC.Ice
{
    internal static class EnumerableExtensions
    {
        private static readonly ThreadLocal<Random> _rand = new ThreadLocal<Random>(() => new Random());

        /// <summary>Shuffles the elements of a sequence.</summary>
        /// <typeparam name="TSource">The type of the elements of source.</typeparam>
        /// <param name="source">A sequence of values to shuffle.</param>
        /// <returns>An <see cref="IEnumerable{TSouce}"/> whose elements are shuffled.</returns>
        internal static IEnumerable<TSource> Shuffle<TSource>(this IEnumerable<TSource> source) =>
            source.OrderBy(element => _rand.Value!.Next());
    }
}
