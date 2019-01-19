//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceUtilInternal
{
    public sealed class Collections
    {
        public static bool SequenceEquals(ICollection seq1, ICollection seq2)
        {
            if(ReferenceEquals(seq1, seq2))
            {
                return true;
            }

            if((seq1 == null && seq2 != null) || (seq1 != null && seq2 == null))
            {
                return false;
            }

            if(seq1.Count == seq2.Count)
            {
                IEnumerator e1 = seq1.GetEnumerator();
                IEnumerator e2 = seq2.GetEnumerator();
                while(e1.MoveNext())
                {
                    e2.MoveNext();
                    if(e1.Current == null)
                    {
                        if(e2.Current != null)
                        {
                            return false;
                        }
                    }
                    else if(!e1.Current.Equals(e2.Current))
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }

        public static bool SequenceEquals(IEnumerable seq1, IEnumerable seq2)
        {
            if(ReferenceEquals(seq1, seq2))
            {
                return true;
            }

            if((seq1 == null && seq2 != null) || (seq1 != null && seq2 == null))
            {
                return false;
            }

            IEnumerator e1 = seq1.GetEnumerator();
            IEnumerator e2 = seq2.GetEnumerator();
            while(e1.MoveNext())
            {
                if(!e2.MoveNext())
                {
                    return false;
                }
                if(e1.Current == null)
                {
                    if(e2.Current != null)
                    {
                        return false;
                    }
                }
                else if(!e1.Current.Equals(e2.Current))
                {
                    return false;
                }
            }

            if(e2.MoveNext())
            {
                return false;
            }

            return true;
        }

        public static int SequenceGetHashCode(IEnumerable seq)
        {
            int h = 5381;
            IEnumerator e = seq.GetEnumerator();
            while(e.MoveNext())
            {
                IceInternal.HashUtil.hashAdd(ref h, e.Current);
            }
            return h;
        }

        public static bool DictionaryEquals(IDictionary d1, IDictionary d2)
        {
            if(ReferenceEquals(d1, d2))
            {
                return true;
            }

            if((d1 == null && d2 != null) || (d1 != null && d2 == null))
            {
                return false;
            }

            if(d1.Count == d2.Count)
            {
                IDictionaryEnumerator e1 = d1.GetEnumerator();
                IDictionaryEnumerator e2 = d2.GetEnumerator();
                while(e1.MoveNext())
                {
                    e2.MoveNext();
                    if(!e1.Key.Equals(e2.Key))
                    {
                        return false;
                    }
                    if(e1.Value == null)
                    {
                        if(e2.Value != null)
                        {
                            return false;
                        }
                    }
                    else if(!e1.Value.Equals(e2.Value))
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }

        public static int DictionaryGetHashCode(IDictionary d)
        {
            int h = 5381;
            IDictionaryEnumerator e = d.GetEnumerator();
            while(e.MoveNext())
            {
                IceInternal.HashUtil.hashAdd(ref h, e.Key);
                IceInternal.HashUtil.hashAdd(ref h, e.Value);
            }
            return h;
        }

        public static void Shuffle<T>(ref List<T> l)
        {
            lock(_rand)
            {
                for(int j = 0; j < l.Count - 1; ++j)
                {
                    int r = _rand.Next(l.Count - j) + j;
                    Debug.Assert(r >= j && r < l.Count);
                    if(r != j)
                    {
                        T tmp = l[j];
                        l[j] = l[r];
                        l[r] = tmp;
                    }
                }
            }
        }

        public static void Sort<T>(ref List<T> array, IComparer<T> comparator)
        {
            //
            // This Sort method implements the merge sort algorithm
            // which is a stable sort (unlike the Sort method of the
            // System.Collections.ArrayList which is unstable).
            //
            Sort1(ref array, 0, array.Count, comparator);
        }

        private static void Sort1<T>(ref List<T> array, int begin, int end, IComparer<T> comparator)
        {
            int mid;
            if(end - begin <= 1)
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

            T[] tmp = new T[end - begin];
            while(i < mid && j < end)
            {
                if(comparator.Compare(array[i], array[j]) <= 0)
                {
                    tmp[k++] = array[i++];
                }
                else
                {
                    tmp[k++] = array[j++];
                }
            }

            while(i < mid)
            {
                tmp[k++] = array[i++];
            }
            while(j < end)
            {
                tmp[k++] = array[j++];
            }
            for(i = 0; i < (end - begin); ++i)
            {
                array[begin + i] = tmp[i];
            }
        }

        private static System.Random _rand = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
    }
}
