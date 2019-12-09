//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * The batch compression option when flushing queued batch requests.
 **/
public enum CompressBatch
{
    /**
     * Compress the batch requests.
     **/
    Yes(0),
    /**
     * Don't compress the batch requests.
     **/
    No(1),
    /**
     * Compress the batch requests if at least one request was
     * made on a compressed proxy.
     **/
    BasedOnProxy(2);

    public int value()
    {
        return _value;
    }

    public static CompressBatch valueOf(int v)
    {
        switch(v)
        {
        case 0:
            return Yes;
        case 1:
            return No;
        case 2:
            return BasedOnProxy;
        }
        return null;
    }

    private CompressBatch(int v)
    {
        _value = v;
    }

    private final int _value;
}
