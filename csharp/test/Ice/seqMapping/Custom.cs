// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping;

public class Custom<T> : IEnumerable<T>
{
    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => _list.GetEnumerator();

    public IEnumerator<T> GetEnumerator() => _list.GetEnumerator();

    public int Count => _list.Count;

    public T this[int index]
    {
        get => _list[index];

        set => _list[index] = value;
    }

    public void Add(T elmt) => _list.Add(elmt);

    public override bool Equals(object obj)
    {
        try
        {
            var tmp = (Custom<T>)obj;
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

    public override int GetHashCode() => base.GetHashCode();

    private readonly List<T> _list = new List<T>();
}
