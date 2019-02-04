//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Base class for writing Slice classes to an output stream.
 **/
public abstract class ObjectWriter extends ObjectImpl
{
    /**
     * Writes the state of this Slice class to an output stream.
     *
     * @param out The stream to write to.
     **/
    public abstract void write(OutputStream out);

    @Override
    public void _iceWrite(OutputStream os)
    {
        write(os);
    }

    @Override
    public void _iceRead(Ice.InputStream is)
    {
        assert(false);
    }
}
