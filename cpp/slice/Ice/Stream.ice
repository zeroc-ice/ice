// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STREAM_ICE
#define ICE_STREAM_ICE

module Ice
{

/** A sequence of bools. **/
sequence<bool> BoolSeq;

/** A sequence of bytes. **/
sequence<byte> ByteSeq;

/** A sequence of shorts. **/
sequence<short> ShortSeq;

/** A sequence of ints. **/
sequence<int> IntSeq;

/** A sequence of longs. **/
sequence<long> LongSeq;

/** A sequence of floats. **/
sequence<float> FloatSeq;

/** A sequence of doubles. **/
sequence<double> DoubleSeq;

/** A sequence of strings. **/
sequence<string> StringSeq;

/** A sequence of wstrings. **/
sequence<wstring> WStringSeq;

/**
 *
 * A stream for marshaling and unmarshaling data.
 *
 * @see Communicator::createStream
 *
 **/
local interface Stream
{
    /**
     *
     * Resize the stream's buffer.
     *
     * @param len The new length of the stream's buffer.
     *
     **/
    void resize(int len);

    /**
     *
     * Resize space in the stream's buffer.
     *
     * @param len The space to reserve.
     *
     **/
    void reserve(int len);

    /** Start writing an encapsulation. */
    void startWriteEncaps();
    
    /** End writing an encapsulation. */
    void endWriteEncaps();
    
    /** Start reading an encapsulation. */
    void startReadEncaps();
    
    /** End reading an encapsulation. */
    void endReadEncaps();
    
    /** Skip an encapsulation. */
    void skipEncaps();

    /**
     *
     * Write a single byte.
     *
     * @param value The byte to write.
     *
     **/
    void writeByte(byte value);

    /**
     *
     * Write a sequence of bytes.
     *
     * @param value The sequence of bytes to write.
     *
     **/
    void writeByteSeq(ByteSeq value);

    /**
     *
     * Read a single byte.
     *
     * @param value The byte that has been read.
     *
     **/
    void readByte(; byte value);

    /**
     *
     * Read a sequence of bytes.
     *
     * @param value The bytes that have been read.
     *
     **/
    void readByteSeq(; ByteSeq value);

    /**
     *
     * Write a single bool.
     *
     * @param value The bool to write.
     *
     **/
    void writeBool(bool value);

    /**
     *
     * Write a sequence of bools.
     *
     * @param value The sequence of bools to write.
     *
     **/
    void writeBoolSeq(BoolSeq value);

    /**
     *
     * Read a single bool.
     *
     * @param value The bool that has been read.
     *
     **/
    void readBool(; bool value);

    /**
     *
     * Read a sequence of bools.
     *
     * @param value The bools that have been read.
     *
     **/
    void readBoolSeq(; BoolSeq value);

    /**
     *
     * Write a single short.
     *
     * @param value The short to write.
     *
     **/
    void writeShort(short value);

    /**
     *
     * Write a sequence of shorts.
     *
     * @param value The sequence of shorts to write.
     *
     **/
    void writeShortSeq(ShortSeq value);

    /**
     *
     * Read a single short.
     *
     * @param value The short that has been read.
     *
     **/
    void readShort(; short value);

    /**
     *
     * Read a sequence of shorts.
     *
     * @param value The shorts that have been read.
     *
     **/
    void readShortSeq(; ShortSeq value);

    /**
     *
     * Write a single int.
     *
     * @param value The int to write.
     *
     **/
    void writeInt(int value);

    /**
     *
     * Write a sequence of ints.
     *
     * @param value The sequence of ints to write.
     *
     **/
    void writeIntSeq(IntSeq value);

    /**
     *
     * Read a single int.
     *
     * @param value The int that has been read.
     *
     **/
    void readInt(; int value);

    /**
     *
     * Read a sequence of ints.
     *
     * @param value The ints that have been read.
     *
     **/
    void readIntSeq(; IntSeq value);

    /**
     *
     * Write a single long.
     *
     * @param value The long to write.
     *
     **/
    void writeLong(long value);

    /**
     *
     * Write a sequence of longs.
     *
     * @param value The sequence of longs to write.
     *
     **/
    void writeLongSeq(LongSeq value);

    /**
     *
     * Read a single long.
     *
     * @param value The long that has been read.
     *
     **/
    void readLong(; long value);

    /**
     *
     * Read a sequence of longs.
     *
     * @param value The longs that have been read.
     *
     **/
    void readLongSeq(; LongSeq value);

    /**
     *
     * Write a single float.
     *
     * @param value The float to write.
     *
     **/
    void writeFloat(float value);

    /**
     *
     * Write a sequence of floats.
     *
     * @param value The sequence of floats to write.
     *
     **/
    void writeFloatSeq(FloatSeq value);

    /**
     *
     * Read a single float.
     *
     * @param value The float that has been read.
     *
     **/
    void readFloat(; float value);

    /**
     *
     * Read a sequence of floats.
     *
     * @param value The floats that have been read.
     *
     **/
    void readFloatSeq(; FloatSeq value);

    /**
     *
     * Write a single double.
     *
     * @param value The double to write.
     *
     **/
    void writeDouble(double value);

    /**
     *
     * Write a sequence of doubles.
     *
     * @param value The sequence of doubles to write.
     *
     **/
    void writeDoubleSeq(DoubleSeq value);

    /**
     *
     * Read a single double.
     *
     * @param value The double that has been read.
     *
     **/
    void readDouble(; double value);

    /**
     *
     * Read a sequence of doubles.
     *
     * @param value The doubles that have been read.
     *
     **/
    void readDoubleSeq(; DoubleSeq value);

    /**
     *
     * Write a single string.
     *
     * @param value The string to write.
     *
     **/
    void writeString(string value);

    /**
     *
     * Write a sequence of strings.
     *
     * @param value The sequence of strings to write.
     *
     **/
    void writeStringSeq(StringSeq value);

    /**
     *
     * Read a single string.
     *
     * @param value The string that has been read.
     *
     **/
    void readString(; string value);

    /**
     *
     * Read a sequence of strings.
     *
     * @param value The strings that have been read.
     *
     **/
    void readStringSeq(; StringSeq value);

    /**
     *
     * Write a single wstring.
     *
     * @param value The wstring to write.
     *
     **/
    void writeWString(wstring value);

    /**
     *
     * Write a sequence of wstrings.
     *
     * @param value The sequence of wstrings to write.
     *
     **/
    void writeWStringSeq(WStringSeq value);

    /**
     *
     * Read a single wstring.
     *
     * @param value The wstring that has been read.
     *
     **/
    void readWString(; wstring value);

    /**
     *
     * Read a sequence of wstrings.
     *
     * @param value The wstrings that have been read.
     *
     **/
    void readWStringSeq(; WStringSeq value);

    /**
     *
     * Write a single proxy.
     *
     * @param value The proxy to write.
     *
     **/
    void writeProxy(Object* value);

    /**
     *
     * Read a single proxy.
     *
     * @param value The proxy that has been read.
     *
     **/
    void readProxy(; Object* value);

    /**
     *
     * Write a single object.
     *
     * @param value The object to write.
     *
     **/
    void writeObject(Object value);

    /**
     *
     * Read a single object. This operation raises
     * [NoObjectFactoryException] if no suitable factory for the
     * object to be read can be found.
     *
     * @param value The object that has been read.
     *
     * @see ObjectFactory
     *
     **/
    void readObject(; Object value);
};

};

#endif
