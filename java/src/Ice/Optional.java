// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Generic class for optional parameters.
 **/
public class Optional<T>
{
    /**
     * The value defaults to unset.
     **/
    public Optional()
    {
        _isSet = false;
    }

    /**
     * Sets the value to the given argument.
     *
     * @param v The initial value.
     **/
    public Optional(T v)
    {
        _value = v;
        _isSet = true;
    }

    /**
     * Sets the value to a shallow copy of the given optional.
     *
     * @param opt The source value.
     **/
    public Optional(Optional<T> opt)
    {
        _value = opt._value;
        _isSet = opt._isSet;
    }

    /**
     * Obtains the current value.
     *
     * @return The current value.
     * @throws IllegalStateException If the value is not set.
     **/
    public T get()
    {
        if(!_isSet)
        {
            throw new IllegalStateException("no value is set");
        }
        return _value;
    }

    /**
     * Sets the value to the given argument.
     *
     * @param v The new value.
     **/
    public void set(T v)
    {
        _value = v;
        _isSet = true;
    }

    /**
     * Determines whether the value is set.
     *
     * @return True if the value is set, false otherwise.
     **/
    public boolean isSet()
    {
        return _isSet;
    }

    private T _value;
    private boolean _isSet;
}
