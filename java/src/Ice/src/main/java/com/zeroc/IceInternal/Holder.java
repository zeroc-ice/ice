//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class Holder<T>
{
    public Holder()
    {
    }

    public Holder(T value)
    {
        this.value = value;
    }

    public T value;
}
