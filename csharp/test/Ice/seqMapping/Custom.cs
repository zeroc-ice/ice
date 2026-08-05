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

    public void Add(T element) => _list.Add(element);

    public override bool Equals(object obj)
    {
        try
        {
            var tmp = (Custom<T>)obj;
            IEnumerator<T> e = tmp.GetEnumerator();
            foreach (T element in _list)
            {
                if (!e.MoveNext())
                {
                    return false;
                }
                if (element == null)
                {
                    if (e.Current != null)
                    {
                        return false;
                    }
                }
                else
                {
                    if (!element.Equals(e.Current))
                    {
                        return false;
                    }
                }
            }
            return !e.MoveNext();
        }
        catch (Exception)
        {
            return false;
        }
    }

    public override int GetHashCode()
    {
        var hash = new HashCode();
        foreach (T element in _list)
        {
            hash.Add(element);
        }
        return hash.ToHashCode();
    }

    private readonly List<T> _list = new List<T>();
}
