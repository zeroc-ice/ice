// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class Buffer
{
    public
    Buffer()
    {
        data = new byte[1500];
        pos = 0;
        len = 0;
    }

    public void
    resize(int n)
    {
        if (n > data.length)
        {
            int sz = data.length << 1;
            byte[] arr = new byte[n < sz ? sz : n];
            System.arraycopy(data, 0, arr, 0, len);
            data = arr;
        }
        len = n;
    }

    public void
    reserve(int n)
    {
        if (n > data.length)
        {
            int sz = data.length << 1;
            byte[] arr = new byte[n < sz ? sz : n];
            System.arraycopy(data, 0, arr, 0, len);
            data = arr;
        }
    }

    public byte[] data;
    public int pos;
    public int len;
}
