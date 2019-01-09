// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public class BZip2
{
    public static Buffer compress(Buffer buf, int headerSize, int compressionLevel)
    {
        assert(supported());

        int uncompressedLen = buf.size() - headerSize;
        int compressedLen = (int)(uncompressedLen * 1.01 + 600);
        byte[] compressed = new byte[compressedLen];

        byte[] data = null;
        int offset = 0;
        try
        {
            //
            // If the ByteBuffer is backed by an array then we can avoid
            // an extra copy by using the array directly.
            //
            data = buf.b.array();
            offset = buf.b.arrayOffset();
        }
        catch(Exception ex)
        {
            //
            // Otherwise, allocate an array to hold a copy of the uncompressed data.
            //
            data = new byte[buf.size()];
            buf.position(0);
            buf.b.get(data);
        }

        try
        {
            //
            // Compress the data using the class org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream.
            // Its constructor requires an OutputStream argument, therefore we pass the
            // compressed buffer in an OutputStream wrapper.
            //
            BufferedOutputStream bos = new BufferedOutputStream(compressed);
            java.lang.Object[] args = new java.lang.Object[]{ bos, Integer.valueOf(compressionLevel) };
            java.io.OutputStream os = (java.io.OutputStream)_bzOutputStreamCtor.newInstance(args);
            os.write(data, offset + headerSize, uncompressedLen);
            os.close();
            compressedLen = bos.pos();
        }
        catch(Exception ex)
        {
            throw new Ice.CompressionException("bzip2 compression failure", ex);
        }

        //
        // Don't bother if the compressed data is larger than the
        // uncompressed data.
        //
        if(compressedLen >= uncompressedLen)
        {
            return null;
        }

        Buffer r = new Buffer(false);
        r.resize(headerSize + 4 + compressedLen, false);
        r.position(0);

        //
        // Copy the header from the uncompressed stream to the compressed one.
        //
        r.b.put(data, offset, headerSize);

        //
        // Add the size of the uncompressed stream before the message body.
        //
        r.b.putInt(buf.size());

        //
        // Add the compressed message body.
        //
        r.b.put(compressed, 0, compressedLen);

        return r;
    }

    public static Buffer uncompress(Buffer buf, int headerSize, int messageSizeMax)
    {
        assert(supported());

        buf.position(headerSize);
        int uncompressedSize = buf.b.getInt();
        if(uncompressedSize <= headerSize)
        {
            throw new Ice.IllegalMessageSizeException();
        }
        if(uncompressedSize > messageSizeMax)
        {
            IceInternal.Ex.throwMemoryLimitException(uncompressedSize, messageSizeMax);
        }

        int compressedLen = buf.size() - headerSize - 4;

        byte[] compressed = null;
        int offset = 0;
        try
        {
            //
            // If the ByteBuffer is backed by an array then we can avoid
            // an extra copy by using the array directly.
            //
            compressed = buf.b.array();
            offset = buf.b.arrayOffset();
        }
        catch(Exception ex)
        {
            //
            // Otherwise, allocate an array to hold a copy of the compressed data.
            //
            compressed = new byte[buf.size()];
            buf.position(0);
            buf.b.get(compressed);
        }

        Buffer r = new Buffer(false);
        r.resize(uncompressedSize, false);

        try
        {
            //
            // Uncompress the data using the class org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream.
            // Its constructor requires an InputStream argument, therefore we pass the
            // compressed data in a ByteArrayInputStream.
            //
            java.io.ByteArrayInputStream bais =
                new java.io.ByteArrayInputStream(compressed, offset + headerSize + 4, compressedLen);
            java.lang.Object[] args = new java.lang.Object[]{ bais };
            java.io.InputStream is = (java.io.InputStream)_bzInputStreamCtor.newInstance(args);
            r.position(headerSize);
            byte[] arr = new byte[8 * 1024];
            int n;
            while((n = is.read(arr)) != -1)
            {
                r.b.put(arr, 0, n);
            }
            is.close();
        }
        catch(Exception ex)
        {
            throw new Ice.CompressionException("bzip2 uncompression failure", ex);
        }

        //
        // Copy the header from the compressed stream to the uncompressed one.
        //
        r.position(0);
        r.b.put(compressed, offset, headerSize);

        return r;
    }

    private static boolean _checked = false;
    private static java.lang.reflect.Constructor<?> _bzInputStreamCtor;
    private static java.lang.reflect.Constructor<?> _bzOutputStreamCtor;

    public synchronized static boolean supported()
    {
        //
        // Use lazy initialization when determining whether support for bzip2 compression is available.
        //
        if(!_checked)
        {
            _checked = true;
            try
            {
                Class<?> cls;
                Class<?>[] types = new Class<?>[1];
                cls = IceInternal.Util.findClass("org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream", null);
                if(cls != null)
                {
                    types[0] = java.io.InputStream.class;
                    _bzInputStreamCtor = cls.getDeclaredConstructor(types);
                }
                cls = IceInternal.Util.findClass("org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream", null);
                if(cls != null)
                {
                    types = new Class[2];
                    types[0] = java.io.OutputStream.class;
                    types[1] = Integer.TYPE;
                    _bzOutputStreamCtor = cls.getDeclaredConstructor(types);
                }
            }
            catch(Exception ex)
            {
                // Ignore - bzip2 compression not available.
            }
        }
        return _bzInputStreamCtor != null && _bzOutputStreamCtor != null;
    }

    private static class BufferedOutputStream extends java.io.OutputStream
    {
        BufferedOutputStream(byte[] data)
        {
            _data = data;
        }

        @Override
        public void close()
            throws java.io.IOException
        {
        }

        @Override
        public void flush()
            throws java.io.IOException
        {
        }

        @Override
        public void write(byte[] b)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= b.length);
            System.arraycopy(b, 0, _data, _pos, b.length);
            _pos += b.length;
        }

        @Override
        public void write(byte[] b, int off, int len)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= len);
            System.arraycopy(b, off, _data, _pos, len);
            _pos += len;
        }

        @Override
        public void write(int b)
            throws java.io.IOException
        {
            assert(_data.length - _pos >= 1);
            _data[_pos] = (byte)b;
            ++_pos;
        }

        int
        pos()
        {
            return _pos;
        }

        private byte[] _data;
        private int _pos;
    }
}
