// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class InputStreamI implements InputStream
{
    public
    InputStreamI(Communicator communicator, byte[] data, boolean copyData)
    {
        _communicator = communicator;
        IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        initialize(instance, data, instance.defaultsAndOverrides().defaultEncoding, copyData);
    }

    public
    InputStreamI(Communicator communicator, byte[] data, EncodingVersion v, boolean copyData)
    {
        _communicator = communicator;
        initialize(IceInternal.Util.getInstance(communicator), data, v, copyData);
    }

    private void
    initialize(IceInternal.Instance instance, byte[] data, EncodingVersion v, boolean copyData)
    {
        if(copyData)
        {
            _is = new IceInternal.BasicStream(instance, v, false);
            _is.resize(data.length, true);
            IceInternal.Buffer buf = _is.getBuffer();
            buf.b.position(0);
            buf.b.put(data);
            buf.b.position(0);
        }
        else
        {
            _is = new IceInternal.BasicStream(instance, v, data);
        }
        _is.closure(this);
    }

    @Override
    public Communicator
    communicator()
    {
        return _communicator;
    }

    @Override
    public void
    sliceObjects(boolean slice)
    {
        _is.sliceObjects(slice);
    }

    @Override
    public boolean
    readBool()
    {
        return _is.readBool();
    }

    @Override
    public boolean[]
    readBoolSeq()
    {
        return _is.readBoolSeq();
    }

    @Override
    public byte
    readByte()
    {
        return _is.readByte();
    }

    @Override
    public byte[]
    readByteSeq()
    {
        return _is.readByteSeq();
    }

    @Override
    public java.nio.ByteBuffer
    readByteBuffer()
    {
        return _is.readByteBuffer();
    }

    @Override
    public java.io.Serializable
    readSerializable()
    {
        return _is.readSerializable();
    }

    @Override
    public short
    readShort()
    {
        return _is.readShort();
    }

    @Override
    public short[]
    readShortSeq()
    {
        return _is.readShortSeq();
    }

    @Override
    public java.nio.ShortBuffer
    readShortBuffer()
    {
        return _is.readShortBuffer();
    }

    @Override
    public int
    readInt()
    {
        return _is.readInt();
    }

    @Override
    public int[]
    readIntSeq()
    {
        return _is.readIntSeq();
    }

    @Override
    public java.nio.IntBuffer
    readIntBuffer()
    {
        return _is.readIntBuffer();
    }

    @Override
    public long
    readLong()
    {
        return _is.readLong();
    }

    @Override
    public long[]
    readLongSeq()
    {
        return _is.readLongSeq();
    }

    @Override
    public java.nio.LongBuffer
    readLongBuffer()
    {
        return _is.readLongBuffer();
    }

    @Override
    public float
    readFloat()
    {
        return _is.readFloat();
    }

    @Override
    public float[]
    readFloatSeq()
    {
        return _is.readFloatSeq();
    }

    @Override
    public java.nio.FloatBuffer
    readFloatBuffer()
    {
        return _is.readFloatBuffer();
    }

    @Override
    public double
    readDouble()
    {
        return _is.readDouble();
    }

    @Override
    public double[]
    readDoubleSeq()
    {
        return _is.readDoubleSeq();
    }

    @Override
    public java.nio.DoubleBuffer
    readDoubleBuffer()
    {
        return _is.readDoubleBuffer();
    }

    @Override
    public String
    readString()
    {
        return _is.readString();
    }

    @Override
    public String[]
    readStringSeq()
    {
        return _is.readStringSeq();
    }

    @Override
    public int
    readSize()
    {
        return _is.readSize();
    }

    @Override
    public int
    readAndCheckSeqSize(int minWire)
    {
        return _is.readAndCheckSeqSize(minWire);
    }

    @Override
    public ObjectPrx
    readProxy()
    {
        return _is.readProxy();
    }

    private static class Patcher implements IceInternal.Patcher
    {
        Patcher(ReadObjectCallback cb)
        {
            _cb = cb;
        }

        @Override
        public void
        patch(Ice.Object v)
        {
            _cb.invoke(v);
        }

        @Override
        public String
        type()
        {
            return "unknown";
        }

        ReadObjectCallback _cb;
    }

    @Override
    public void
    readObject(ReadObjectCallback cb)
    {
        _is.readObject(new Patcher(cb));
    }

    @Override
    public int
    readEnum(int maxValue)
    {
        return _is.readEnum(maxValue);
    }

    @Override
    public void
    throwException()
        throws UserException
    {
        _is.throwException(null);
    }

    private static class UserExceptionFactoryI implements IceInternal.UserExceptionFactory
    {
        UserExceptionFactoryI(UserExceptionReaderFactory factory)
        {
            _factory = factory;
        }

        @Override
        public void
        createAndThrow(String id)
            throws UserException
        {
            _factory.createAndThrow(id);
        }

        @Override
        public void
        destroy()
        {
        }

        private UserExceptionReaderFactory _factory;
    }

    @Override
    public void
    throwException(UserExceptionReaderFactory factory)
        throws UserException
    {
        _is.throwException(new UserExceptionFactoryI(factory));
    }

    @Override
    public void
    startObject()
    {
        _is.startReadObject();
    }

    @Override
    public SlicedData
    endObject(boolean preserve)
    {
        return _is.endReadObject(preserve);
    }

    @Override
    public void
    startException()
    {
        _is.startReadException();
    }

    @Override
    public SlicedData
    endException(boolean preserve)
    {
        return _is.endReadException(preserve);
    }

    @Override
    public String
    startSlice()
    {
        return _is.startReadSlice();
    }

    @Override
    public void
    endSlice()
    {
        _is.endReadSlice();
    }

    @Override
    public void
    skipSlice()
    {
        _is.skipSlice();
    }

    @Override
    public Ice.EncodingVersion
    startEncapsulation()
    {
        return _is.startReadEncaps();
    }

    @Override
    public void
    endEncapsulation()
    {
        _is.endReadEncapsChecked();
    }

    @Override
    public Ice.EncodingVersion
    skipEncapsulation()
    {
        return _is.skipEncaps();
    }

    @Override
    public EncodingVersion
    getEncoding()
    {
        return _is.getReadEncoding();
    }

    @Override
    public void
    readPendingObjects()
    {
        _is.readPendingObjects();
    }

    @Override
    public void
    rewind()
    {
        _is.clear();
        _is.getBuffer().b.position(0);
    }

    @Override
    public void
    skip(int sz)
    {
        _is.skip(sz);
    }

    @Override
    public void
    skipSize()
    {
        _is.skipSize();
    }

    @Override
    public boolean
    readOptional(int tag, OptionalFormat format)
    {
        return _is.readOpt(tag, format);
    }

    @Override
    public int
    pos()
    {
        return _is.pos();
    }

    @Override
    public void
    destroy()
    {
        if(_is != null)
        {
            _is = null;
        }
    }

    private Communicator _communicator;
    private IceInternal.BasicStream _is;
}
