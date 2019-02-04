//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Holder class for integers that are out- or inout-parameters.
 **/
public final class IntHolder extends Holder<Integer>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    IntHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>int</code> value for this holder.
     **/
    public
    IntHolder(int value)
    {
        super(value);
    }
}
