// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class OutputStreamI implements OutputStream
{
    public
    OutputStreamI(Communicator communicator)
    {
        _communicator = communicator;
        IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        _os = new IceInternal.BasicStream(instance, instance.defaultsAndOverrides().defaultEncoding, false);
        _os.closure(this);
    }

    public
    OutputStreamI(Communicator communicator, EncodingVersion v)
    {
        _communicator = communicator;
        IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        _os = new IceInternal.BasicStream(instance, v, false);
        _os.closure(this);
    }

    public
    OutputStreamI(Communicator communicator, IceInternal.BasicStream os)
    {
        _communicator = communicator;
        _os = os;
        _os.closure(this);
    }

    @Override
    public Communicator
    communicator()
    {
        return _communicator;
    }

    @Override
    public void
    writeBool(boolean v)
    {
        _os.writeBool(v);
    }

    @Override
    public void
    writeBoolSeq(boolean[] v)
    {
        _os.writeBoolSeq(v);
    }

    @Override
    public void
    writeByte(byte v)
    {
        _os.writeByte(v);
    }

    @Override
    public void
    writeByteSeq(byte[] v)
    {
        _os.writeByteSeq(v);
    }

    @Override
    public void
    writeByteBuffer(java.nio.ByteBuffer v)
    {
        _os.writeByteBuffer(v);
    }

    @Override
    public void
    writeSerializable(java.io.Serializable v)
    {
        _os.writeSerializable(v);
    }

    @Override
    public void
    writeShort(short v)
    {
        _os.writeShort(v);
    }

    @Override
    public void
    writeShortSeq(short[] v)
    {
        _os.writeShortSeq(v);
    }

    @Override
    public void
    writeShortBuffer(java.nio.ShortBuffer v)
    {
        _os.writeShortBuffer(v);
    }

    @Override
    public void
    writeInt(int v)
    {
        _os.writeInt(v);
    }

    @Override
    public void
    writeIntSeq(int[] v)
    {
        _os.writeIntSeq(v);
    }

    @Override
    public void
    writeIntBuffer(java.nio.IntBuffer v)
    {
        _os.writeIntBuffer(v);
    }

    @Override
    public void
    writeLong(long v)
    {
        _os.writeLong(v);
    }

    @Override
    public void
    writeLongSeq(long[] v)
    {
        _os.writeLongSeq(v);
    }

    @Override
    public void
    writeLongBuffer(java.nio.LongBuffer v)
    {
        _os.writeLongBuffer(v);
    }

    @Override
    public void
    writeFloat(float v)
    {
        _os.writeFloat(v);
    }

    @Override
    public void
    writeFloatSeq(float[] v)
    {
        _os.writeFloatSeq(v);
    }

    @Override
    public void
    writeFloatBuffer(java.nio.FloatBuffer v)
    {
        _os.writeFloatBuffer(v);
    }

    @Override
    public void
    writeDouble(double v)
    {
        _os.writeDouble(v);
    }

    @Override
    public void
    writeDoubleSeq(double[] v)
    {
        _os.writeDoubleSeq(v);
    }

    @Override
    public void
    writeDoubleBuffer(java.nio.DoubleBuffer v)
    {
        _os.writeDoubleBuffer(v);
    }

    @Override
    public void
    writeString(String v)
    {
        _os.writeString(v);
    }

    @Override
    public void
    writeStringSeq(String[] v)
    {
        _os.writeStringSeq(v);
    }

    @Override
    public void
    writeSize(int sz)
    {
        if(sz < 0)
        {
            throw new MarshalException();
        }

        _os.writeSize(sz);
    }

    @Override
    public void
    writeProxy(ObjectPrx v)
    {
        _os.writeProxy(v);
    }

    @Override
    public void
    writeObject(Ice.Object v)
    {
        _os.writeObject(v);
    }

    @Override
    public void
    writeEnum(int v, int maxValue)
    {
        _os.writeEnum(v, maxValue);
    }

    @Override
    public void
    writeException(UserException v)
    {
        _os.writeUserException(v);
    }

    @Override
    public void
    startObject(SlicedData slicedData)
    {
        _os.startWriteObject(slicedData);
    }

    @Override
    public void
    endObject()
    {
        _os.endWriteObject();
    }

    @Override
    public void
    startException(SlicedData slicedData)
    {
        _os.startWriteException(slicedData);
    }

    @Override
    public void
    endException()
    {
        _os.endWriteException();
    }

    @Override
    public void
    startSlice(String typeId, int compactId, boolean last)
    {
        _os.startWriteSlice(typeId, compactId, last);
    }

    @Override
    public void
    endSlice()
    {
        _os.endWriteSlice();
    }

    @Override
    public void
    startEncapsulation(Ice.EncodingVersion encoding, Ice.FormatType format)
    {
        _os.startWriteEncaps(encoding, format);
    }

    @Override
    public void
    startEncapsulation()
    {
        _os.startWriteEncaps();
    }

    @Override
    public void
    endEncapsulation()
    {
        _os.endWriteEncapsChecked();
    }

    @Override
    public EncodingVersion
    getEncoding()
    {
        return _os.getWriteEncoding();
    }

    @Override
    public void
    writePendingObjects()
    {
        _os.writePendingObjects();
    }

    @Override
    public boolean
    writeOptional(int tag, Ice.OptionalFormat format)
    {
        return _os.writeOpt(tag, format);
    }

    @Override
    public int
    pos()
    {
        return _os.pos();
    }

    @Override
    public void
    rewrite(int sz, int pos)
    {
        _os.rewriteInt(sz, pos);
    }

    @Override
    public int
    startSize()
    {
        return _os.startSize();
    }

    @Override
    public void
    endSize(int pos)
    {
        _os.endSize(pos);
    }

    @Override
    public byte[]
    finished()
    {
        IceInternal.Buffer buf = _os.prepareWrite();
        byte[] result = new byte[buf.b.limit()];
        buf.b.get(result);

        return result;
    }

    @Override
    public void
    reset(boolean clearBuffer)
    {
        _os.clear();

        IceInternal.Buffer buf = _os.getBuffer();
        if(clearBuffer)
        {
            buf.clear();
        }
        else
        {
            buf.reset();
        }
        buf.b.position(0);
    }

    @Override
    public void
    destroy()
    {
        if(_os != null)
        {
            _os = null;
        }
    }

    private Communicator _communicator;
    private IceInternal.BasicStream _os;
}
