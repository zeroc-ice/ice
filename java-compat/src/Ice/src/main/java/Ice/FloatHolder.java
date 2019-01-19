//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Holder class for floats that are out- or inout-parameters.
 **/
public final class FloatHolder extends Holder<Float>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    FloatHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>float</code> value stored by this holder.
     **/
    public
    FloatHolder(float value)
    {
        super(value);
    }
}
