//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace seqMapping
    {
        public class Custom<T> : IEnumerable<T>
        {
            System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
            {
                return _list.GetEnumerator();
            }

            public IEnumerator<T> GetEnumerator()
            {
                return _list.GetEnumerator();
            }

            public int Count
            {
                get
                {
                    return _list.Count;
                }
            }

            public T this[int index]
            {
                get
                {
                    return _list[index];
                }

                set
                {
                    _list[index] = value;
                }

            }

            public void Add(T elmt)
            {
                _list.Add(elmt);
            }

            public override bool Equals(object o)
            {
                try
                {
                    Custom<T> tmp = (Custom<T>)o;
                    IEnumerator<T> e = tmp.GetEnumerator();
                    foreach (T elmt in _list)
                    {
                        if (!e.MoveNext())
                        {
                            return false;
                        }
                        if (elmt == null)
                        {
                            if (e.Current != null)
                            {
                                return false;
                            }
                        }
                        else
                        {
                            if (!elmt.Equals(e.Current))
                            {
                                return false;
                            }
                        }
                    }
                    return true;
                }
                catch (Exception)
                {
                    return false;
                }
            }

            public override int GetHashCode()
            {
                return base.GetHashCode();
            }

            private List<T> _list = new List<T>();
        }
    }
}
