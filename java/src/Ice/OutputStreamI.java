// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        _os = new IceInternal.BasicStream(instance, instance.defaultsAndOverrides().defaultEncoding, true, false);
        _os.closure(this);
    }

    public
    OutputStreamI(Communicator communicator, EncodingVersion v)
    {
        _communicator = communicator;
        IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
        _os = new IceInternal.BasicStream(instance, v, true, false);
        _os.closure(this);
    }

    public
    OutputStreamI(Communicator communicator, IceInternal.BasicStream os)
    {
        _communicator = communicator;
        _os = os;
        _os.closure(this);
    }

    public Communicator
    communicator()
    {
        return _communicator;
    }

    public void
    writeBool(boolean v)
    {
        _os.writeBool(v);
    }

    public void
    writeBoolSeq(boolean[] v)
    {
        _os.writeBoolSeq(v);
    }

    public void
    writeByte(byte v)
    {
        _os.writeByte(v);
    }

    public void
    writeByteSeq(byte[] v)
    {
        _os.writeByteSeq(v);
    }

    public void
    writeSerializable(java.io.Serializable v)
    {
        _os.writeSerializable(v);
    }

    public void
    writeShort(short v)
    {
        _os.writeShort(v);
    }

    public void
    writeShortSeq(short[] v)
    {
        _os.writeShortSeq(v);
    }

    public void
    writeInt(int v)
    {
        _os.writeInt(v);
    }

    public void
    writeIntSeq(int[] v)
    {
        _os.writeIntSeq(v);
    }

    public void
    writeLong(long v)
    {
        _os.writeLong(v);
    }

    public void
    writeLongSeq(long[] v)
    {
        _os.writeLongSeq(v);
    }

    public void
    writeFloat(float v)
    {
        _os.writeFloat(v);
    }

    public void
    writeFloatSeq(float[] v)
    {
        _os.writeFloatSeq(v);
    }

    public void
    writeDouble(double v)
    {
        _os.writeDouble(v);
    }

    public void
    writeDoubleSeq(double[] v)
    {
        _os.writeDoubleSeq(v);
    }

    public void
    writeString(String v)
    {
        _os.writeString(v);
    }

    public void
    writeStringSeq(String[] v)
    {
        _os.writeStringSeq(v);
    }

    public void
    writeSize(int sz)
    {
        if(sz < 0)
        {
            throw new MarshalException();
        }

        _os.writeSize(sz);
    }

    public void
    writeProxy(ObjectPrx v)
    {
        _os.writeProxy(v);
    }

    public void
    writeObject(Ice.Object v)
    {
        _os.writeObject(v);
    }

    public void
    writeEnum(int v, int maxValue)
    {
        _os.writeEnum(v, maxValue);
    }

    public void
    writeException(UserException v)
    {
        _os.writeUserException(v);
    }

    public void
    startObject(SlicedData slicedData)
    {
        _os.startWriteObject(slicedData);
    }

    public void
    endObject()
    {
        _os.endWriteObject();
    }

    public void
    startException(SlicedData slicedData)
    {
        _os.startWriteException(slicedData);
    }

    public void
    endException()
    {
        _os.endWriteException();
    }

    public void
    startSlice(String typeId, int compactId, boolean last)
    {
        _os.startWriteSlice(typeId, compactId, last);
    }

    public void
    endSlice()
    {
        _os.endWriteSlice();
    }

    public void
    startEncapsulation(Ice.EncodingVersion encoding, Ice.FormatType format)
    {
        _os.startWriteEncaps(encoding, format);
    }

    public void
    startEncapsulation()
    {
        _os.startWriteEncaps();
    }

    public void
    endEncapsulation()
    {
        _os.endWriteEncapsChecked();
    }

    public EncodingVersion
    getEncoding()
    {
        return _os.getWriteEncoding();
    }

    public void
    writePendingObjects()
    {
        _os.writePendingObjects();
    }

    public boolean
    writeOptional(int tag, Ice.OptionalFormat format)
    {
        return _os.writeOpt(tag, format);
    }

    public int
    pos()
    {
        return _os.pos();
    }

    public void
    rewrite(int sz, int pos)
    {
        _os.rewriteInt(sz, pos);
    }

    public void
    startSize()
    {
        _os.startSize();
    }

    public void
    endSize()
    {
        _os.endSize();
    }

    public byte[]
    finished()
    {
        IceInternal.Buffer buf = _os.prepareWrite();
        byte[] result = new byte[buf.b.limit()];
        buf.b.get(result);

        return result;
    }

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
