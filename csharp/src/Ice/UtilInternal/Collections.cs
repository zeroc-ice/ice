// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.UtilInternal;

public static class Collections
{
    public static void Shuffle<T>(ref List<T> l)
    {
        lock (_rand)
        {
            for (int j = 0; j < l.Count - 1; ++j)
            {
                int r = _rand.Next(l.Count - j) + j;
                Debug.Assert(r >= j && r < l.Count);
                if (r != j)
                {
                    T tmp = l[j];
                    l[j] = l[r];
                    l[r] = tmp;
                }
            }
        }
    }

    internal static void Sort<T>(ref List<T> array, IComparer<T> comparator) =>
        //
        // This Sort method implements the merge sort algorithm
        // which is a stable sort (unlike the Sort method of the
        // System.Collections.ArrayList which is unstable).
        //
        Sort1(ref array, 0, array.Count, comparator);

    private static void Sort1<T>(ref List<T> array, int begin, int end, IComparer<T> comparator)
    {
        int mid;
        if (end - begin <= 1)
        {
            return;
        }

        mid = (begin + end) / 2;
        Sort1(ref array, begin, mid, comparator);
        Sort1(ref array, mid, end, comparator);
        Merge(ref array, begin, mid, end, comparator);
    }

    private static void Merge<T>(ref List<T> array, int begin, int mid, int end, IComparer<T> comparator)
    {
        int i = begin;
        int j = mid;
        int k = 0;

        var tmp = new T[end - begin];
        while (i < mid && j < end)
        {
            if (comparator.Compare(array[i], array[j]) <= 0)
            {
                tmp[k++] = array[i++];
            }
            else
            {
                tmp[k++] = array[j++];
            }
        }

        while (i < mid)
        {
            tmp[k++] = array[i++];
        }
        while (j < end)
        {
            tmp[k++] = array[j++];
        }
        for (i = 0; i < (end - begin); ++i)
        {
            array[begin + i] = tmp[i];
        }
    }

    private static readonly Random _rand = new(unchecked((int)DateTime.Now.Ticks));
}
