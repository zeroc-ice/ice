//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Generic holder class for values that are in- or in-out parameters.
 **/
public class Holder<T>
{
    /**
     * Instantiates the class with the default-initialized value of type <code>T</code>.
     **/
    public
    Holder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The value stored by this holder.
     **/
    public
    Holder(T value)
    {
        this.value = value;
    }

    /**
     * The <code>T</code> value stored by this holder.
     **/
    public T value;
}
