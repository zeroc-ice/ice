//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
     * If the given argument is set, this optional is set to a shallow copy of the argument,
     * otherwise this optional is unset.
     *
     * @param opt The source value.
     **/
    public void set(Optional<T> opt)
    {
        _value = opt._value;
        _isSet = opt._isSet;
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

    /**
     * Unsets this value.
     **/
    public void clear()
    {
        _isSet = false;
        _value = null;
    }

    /**
     * Helper function for creating Optional instances.
     *
     * @param v The initial value of the Optional.
     * @param <T> The type of the Optional.
     *
     * @return A new Optional instance set to the given value.
     **/
    public static <T> Optional<T> O(T v)
    {
        return new Optional<T>(v);
    }

    /**
     * Helper function for creating BooleanOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new BooleanOptional instance set to the given value.
     **/
    public static BooleanOptional O(boolean v)
    {
        return new BooleanOptional(v);
    }

    /**
     * Helper function for creating ByteOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new ByteOptional instance set to the given value.
     **/
    public static ByteOptional O(byte v)
    {
        return new ByteOptional(v);
    }

    /**
     * Helper function for creating ShortOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new ShortOptional instance set to the given value.
     **/
    public static ShortOptional O(short v)
    {
        return new ShortOptional(v);
    }

    /**
     * Helper function for creating IntOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new IntOptional instance set to the given value.
     **/
    public static IntOptional O(int v)
    {
        return new IntOptional(v);
    }

    /**
     * Helper function for creating LongOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new LongOptional instance set to the given value.
     **/
    public static LongOptional O(long v)
    {
        return new LongOptional(v);
    }

    /**
     * Helper function for creating FloatOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new FloatOptional instance set to the given value.
     **/
    public static FloatOptional O(float v)
    {
        return new FloatOptional(v);
    }

    /**
     * Helper function for creating DoubleOptional instances.
     *
     * @param v The initial value of the Optional.
     *
     * @return A new DoubleOptional instance set to the given value.
     **/
    public static DoubleOptional O(double v)
    {
        return new DoubleOptional(v);
    }

    private T _value;
    private boolean _isSet;
}
