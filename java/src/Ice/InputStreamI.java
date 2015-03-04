// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
            _is = new IceInternal.BasicStream(instance, v, true, false);
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

    public Communicator
    communicator()
    {
        return _communicator;
    }

    public void
    sliceObjects(boolean slice)
    {
        _is.sliceObjects(slice);
    }

    public boolean
    readBool()
    {
        return _is.readBool();
    }

    public boolean[]
    readBoolSeq()
    {
        return _is.readBoolSeq();
    }

    public byte
    readByte()
    {
        return _is.readByte();
    }

    public byte[]
    readByteSeq()
    {
        return _is.readByteSeq();
    }

    public java.io.Serializable
    readSerializable()
    {
        return _is.readSerializable();
    }

    public short
    readShort()
    {
        return _is.readShort();
    }

    public short[]
    readShortSeq()
    {
        return _is.readShortSeq();
    }

    public int
    readInt()
    {
        return _is.readInt();
    }

    public int[]
    readIntSeq()
    {
        return _is.readIntSeq();
    }

    public long
    readLong()
    {
        return _is.readLong();
    }

    public long[]
    readLongSeq()
    {
        return _is.readLongSeq();
    }

    public float
    readFloat()
    {
        return _is.readFloat();
    }

    public float[]
    readFloatSeq()
    {
        return _is.readFloatSeq();
    }

    public double
    readDouble()
    {
        return _is.readDouble();
    }

    public double[]
    readDoubleSeq()
    {
        return _is.readDoubleSeq();
    }

    public String
    readString()
    {
        return _is.readString();
    }

    public String[]
    readStringSeq()
    {
        return _is.readStringSeq();
    }

    public int
    readSize()
    {
        return _is.readSize();
    }

    public int
    readAndCheckSeqSize(int minWire)
    {
        return _is.readAndCheckSeqSize(minWire);
    }

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

        public void
        patch(Ice.Object v)
        {
            _cb.invoke(v);
        }

        public String
        type()
        {
            return "unknown";
        }

        ReadObjectCallback _cb;
    }

    public void
    readObject(ReadObjectCallback cb)
    {
        _is.readObject(new Patcher(cb));
    }

    public int
    readEnum(int maxValue)
    {
        return _is.readEnum(maxValue);
    }

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

        public void
        createAndThrow(String id)
            throws UserException
        {
            _factory.createAndThrow(id);
        }

        public void
        destroy()
        {
        }

        private UserExceptionReaderFactory _factory;
    }

    public void
    throwException(UserExceptionReaderFactory factory)
        throws UserException
    {
        _is.throwException(new UserExceptionFactoryI(factory));
    }

    public void
    startObject()
    {
        _is.startReadObject();
    }

    public SlicedData
    endObject(boolean preserve)
    {
        return _is.endReadObject(preserve);
    }

    public void
    startException()
    {
        _is.startReadException();
    }

    public SlicedData
    endException(boolean preserve)
    {
        return _is.endReadException(preserve);
    }

    public String
    startSlice()
    {
        return _is.startReadSlice();
    }

    public void
    endSlice()
    {
        _is.endReadSlice();
    }

    public void
    skipSlice()
    {
        _is.skipSlice();
    }

    public Ice.EncodingVersion
    startEncapsulation()
    {
        return _is.startReadEncaps();
    }

    public void
    endEncapsulation()
    {
        _is.endReadEncapsChecked();
    }

    public Ice.EncodingVersion
    skipEncapsulation()
    {
        return _is.skipEncaps();
    }

    public EncodingVersion
    getEncoding()
    {
        return _is.getReadEncoding();
    }

    public void
    readPendingObjects()
    {
        _is.readPendingObjects();
    }

    public void
    rewind()
    {
        _is.clear();
        _is.getBuffer().b.position(0);
    }

    public void
    skip(int sz)
    {
        _is.skip(sz);
    }

    public void
    skipSize()
    {
        _is.skipSize();
    }

    public boolean
    readOptional(int tag, OptionalFormat format)
    {
        return _is.readOpt(tag, format);
    }

    public int
    pos()
    {
        return _is.pos();
    }

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
