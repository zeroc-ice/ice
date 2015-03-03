// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;

namespace IceUtilInternal
{
    public sealed class Collections
    {
        public static bool SequenceEquals(ICollection seq1, ICollection seq2)
        {
            if(object.ReferenceEquals(seq1, seq2))
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
            if(object.ReferenceEquals(seq1, seq2))
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
            int h = 0;

            IEnumerator e = seq.GetEnumerator();
            while(e.MoveNext())
            {
                if(e.Current != null)
                {
                    h = 5 * h + e.Current.GetHashCode();
                }
            }

            return h;           
        }

        public static bool DictionaryEquals(IDictionary d1, IDictionary d2)
        {
            if(object.ReferenceEquals(d1, d2))
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
            int h = 0;

            IDictionaryEnumerator e = d.GetEnumerator();
            while(e.MoveNext())
            {
                h = 5 * h + e.Key.GetHashCode();
                if(e.Value != null)
                {
                    h = 5 * h + e.Key.GetHashCode();
                }
            }

            return h;           
        }
    }
}
