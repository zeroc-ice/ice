// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.io.*;

//
// Class to provide a java.io.OutputStream on top of our stream.
// We use this to serialize arbitrary Java serializable classes into
//
// Slice sequences are encoded on the wire as a count of elements, followed
// by the sequence contents. For arbitrary Java classes, we do not know how
// big the sequence that is eventually written will be. To avoid excessive
// data copying, this class mantains a private _bytes array of 254 bytes and,
// initially, writes data into that array. If more than 254 bytes end up being
// written, we write a dummy sequence size of 255 (which occupies five bytes
// on the wire) into the stream and, once this stream is closed, patch
// that size to match the actual size. Otherwise, if the _bytes buffer contains
// fewer than 255 bytes when this stream is closed, we write the sequence size
// as a single byte, followed by the contents of the _bytes buffer.
//

public class OutputStreamWrapper extends java.io.OutputStream
{
    public
    OutputStreamWrapper(com.zeroc.Ice.OutputStream s)
    {
        _s = s;
        _spos = s.pos();
        _bytes = new byte[254];
        _pos = 0;
    }

    @Override
    public void
    write(int b) throws IOException
    {
        try
        {
            if(_bytes != null)
            {
                //
                // If we can fit the data into the first 254 bytes, write it to _bytes.
                //
                if(_pos < _bytes.length)
                {
                    _bytes[_pos++] = (byte)b;
                    return;
                }

                _s.writeSize(255); // Dummy size, until we know how big the stream
                                   // really is and can patch the size.

                if(_pos > 0)
                {
                    //
                    // Write the current contents of _bytes.
                    //
                    _s.expand(_pos);
                    _s.getBuffer().b.put(_bytes, 0, _pos);
                }
                _bytes = null;
            }

            //
            // Write data passed by caller.
            //
            _s.expand(1);
            _s.getBuffer().b.put((byte)b);
            _pos += 1;
        }
        catch(java.lang.Exception ex)
        {
            throw new IOException(ex.toString());
        }
    }

    @Override
    public void
    write(byte[] b) throws IOException
    {
        write(b, 0, b.length);
    }

    @Override
    public void
    write(byte[] bytes, int offset, int count) throws IOException
    {
        try
        {
            if(_bytes != null)
            {
                //
                // If we can fit the data into the first 254 bytes, write it to _bytes.
                //
                if(count <= _bytes.length - _pos)
                {
                    System.arraycopy(bytes, offset, _bytes, _pos, count);
                    _pos += count;
                    return;
                }

                _s.writeSize(255); // Dummy size, until we know how big the stream
                                   // really is and can patch the size.

                if(_pos > 0)
                {
                    //
                    // Write the current contents of _bytes.
                    //
                    _s.expand(_pos);
                    _s.getBuffer().b.put(_bytes, 0, _pos);
                }
                _bytes = null;
            }

            //
            // Write data passed by caller.
            //
            _s.expand(count);
            _s.getBuffer().b.put(bytes, offset, count);
            _pos += count;
        }
        catch(java.lang.Exception ex)
        {
            throw new IOException(ex.toString());
        }
    }

    @Override
    public void
    flush() throws IOException
    {
        // This does nothing because we do not know the final size of a writable stream until it is closed,
        // and we cannot write to the stream until we know whether the final size is < 255 or not.
    }

    @Override
    public void
    close() throws IOException
    {
        try
        {
            if(_bytes != null)
            {
                assert(_pos <= _bytes.length);
                _s.pos(_spos);
                _s.writeSize(_pos);
                _s.expand(_pos);
                _s.getBuffer().b.put(_bytes, 0, _pos);
                _bytes = null;
            }
            else
            {
                int currentPos = _s.pos();
                _s.pos(_spos);
                _s.writeSize(_pos); // Patch previously-written dummy value.
                _s.pos(currentPos);
            }
        }
        catch(java.lang.Exception ex)
        {
            throw new IOException(ex.toString());
        }
    }

    private com.zeroc.Ice.OutputStream _s;
    private int _spos;
    private byte[] _bytes;
    private int _pos;
}
