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

#include <Ice/BuiltinSequences.ice>
#include <Ice/ObjectFactoryF.ice>

module Ice
{

/**
 *
 * A stream for marshaling and unmarshaling data.
 *
 **/
local interface Stream
{
    /**
     *
     * Start the writing of a dictionary.
     *
     * @param name The name of the parameter.
     *
     * @param size The number of elements in the dictionary.
     *
     * @see [endWriteDictionary]
     *
     **/
    void startWriteDictionary(string name, int size);

    /**
     *
     * End the writing of a dictionary.
     *
     * @see [startWriteDictionary]
     *
     **/
    void endWriteDictionary();

    /**
     *
     * Start the writing of a dictionary element.
     *
     * @see [endWriteDictionaryElement]
     *
     **/
    void startWriteDictionaryElement();

    /**
     *
     * End the writing of a dictionary element.
     *
     * @see [startWriteDictionaryElement]
     *
     **/
    void endWriteDictionaryElement();

    /**
     *
     * Start the reading of a dictionary.
     *
     * @param name The name of the parameter.
     *
     * @param size The number of elements in the dictionary.
     *
     * @see [endReadDictionary]
     *
     **/
    void startReadDictionary(string name ; int size);

    /**
     *
     * End the reading of a dictionary.
     *
     * @see [startReadDictionary]
     *
     **/
    void endReadDictionary();

    /**
     *
     * Start the reading of a dictionary element.
     *
     * @see [endReadDictionaryElement]
     *
     **/
    void startReadDictionaryElement();

    /**
     *
     * End the reading of a dictionary element.
     *
     * @see [startReadDictionaryElement]
     *
     **/
    void endReadDictionaryElement();

    /**
     *
     * Start the writing of a sequence.
     *
     * @param name The name of the parameter.
     *
     * @param size The number of elements in the sequence.
     *
     * @see [endWriteSequence]
     *
     **/
    void startWriteSequence(string name, int size);

    /**
     *
     * End the writing of a sequence.
     *
     * @see [startWriteSequence]
     *
     **/
    void endWriteSequence();

    /**
     *
     * Start the writing of a sequence element.
     *
     * @see [endWriteSequenceElement]
     *
     **/
    void startWriteSequenceElement();

    /**
     *
     * End the writing of a sequence.
     *
     * @see [startWriteSequence]
     *
     **/
    void endWriteSequenceElement();

    /**
     *
     * Start the reading of a sequence.
     *
     * @param name The name of the parameter.
     *
     * @param size The number of elements in the sequence.
     *
     * @see [endReadSequence]
     *
     **/
    void startReadSequence(string name ; int size);

    /**
     *
     * End the reading of a sequence.
     *
     * @see [startReadSequence]
     *
     **/
    void endReadSequence();

    /**
     *
     * Start the reading of a sequence element.
     *
     * @see [endReadSequenceElement]
     *
     **/
    void startReadSequenceElement();

    /**
     *
     * End the reading of a sequence element.
     *
     * @see [startReadSequenceElement]
     *
     **/
    void endReadSequenceElement();

    /**
     *
     * Start the writing of a struct.
     *
     * @param name The name of the parameter.
     *
     * @see [endWriteStruct]
     *
     **/
    void startWriteStruct(string name);

    /**
     *
     * End the writing of a struct.
     *
     * @see [startWriteStruct]
     *
     **/
    void endWriteStruct();

    /**
     *
     * Start the reading of a struct.
     *
     * @param name The name of the parameter.
     *
     * @see [endReadStruct]
     *
     **/
    void startReadStruct(string name);

    /**
     *
     * End the reading of a struct.
     *
     * @see [startReadStruct]
     *
     **/
    void endReadStruct();

    /**
     *
     * Start the writing of an exception
     *
     * @param name The name of the parameter.
     *
     * @see [endWriteException]
     *
     **/
    void startWriteException(string name);

    /**
     *
     * End the writing of an exception
     *
     * @see [startWriteException]
     *
     **/
    void endWriteException();

    /**
     *
     * Start the reading of an exception
     *
     * @param name The name of the parameter.
     *
     * @see [endReadException]
     *
     **/
    void startReadException(string name);

    /**
     *
     * End the reading of an exception
     *
     * @see [startReadException]
     *
     **/
    void endReadException();

    /**
     *
     * Write an enumeration.
     *
     * @param name The name of the parameter.
     *
     * @param table The enumeration elements.
     *
     * @param ordinal The index of the enumeration value.
     *
     * @see [readEnum]
     *
     **/
    void writeEnum(string name, StringSeq table, int ordinal);

    /**
     *
     * Read an enumeration.
     *
     * @param name The name of the parameter.
     *
     * @param table The enumeration elements.
     *
     * @param ordinal The index of the enumeration value.
     *
     * @see [readEnum]
     *
     **/
    void readEnum(string name, StringSeq table ; int ordinal);

    /**
     *
     * Write a single byte.
     *
     * @param name The name of the parameter.
     *
     * @param value The byte to write.
     *
     * @see [readByte]
     *
     **/
    void writeByte(string name, byte value);

    /**
     *
     * Write a sequence of bytes.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of bytes to write.
     *
     * @see [readByteSeq]
     *
     **/
    void writeByteSeq(string name, ByteSeq value);

    /**
     *
     * Read a single byte.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The byte that has been read.
     *
     * @see [writeByte]
     *
     **/
    void readByte(string name ; byte value);

    /**
     *
     * Read a sequence of bytes.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The bytes that have been read.
     *
     * @see [writeByteSeq]
     *
     **/
    void readByteSeq(string name ; ByteSeq value);

    /**
     *
     * Write a single bool.
     *
     * @param name The name of the parameter.
     *
     * @param value The bool to write.
     *
     * @see [readBool]
     *
     **/
    void writeBool(string name, bool value);

    /**
     *
     * Write a sequence of bools.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of bools to write.
     *
     * @see [readBoolSeq]
     *
     **/
    void writeBoolSeq(string name, BoolSeq value);

    /**
     *
     * Read a single bool.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The bool that has been read.
     *
     * @see [writeBool]
     *
     **/
    void readBool(string name ; bool value);

    /**
     *
     * Read a sequence of bools.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The bools that have been read.
     *
     * @see [writeBoolSeq]
     *
     **/
    void readBoolSeq(string name ; BoolSeq value);

    /**
     *
     * Write a single short.
     *
     * @param name The name of the parameter.
     *
     * @param value The short to write.
     *
     * @see [readShort]
     *
     **/
    void writeShort(string name, short value);

    /**
     *
     * Write a sequence of shorts.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of shorts to write.
     *
     * @see [readShortSeq]
     *
     **/
    void writeShortSeq(string name, ShortSeq value);

    /**
     *
     * Read a single short.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The short that has been read.
     *
     * @see [writeShort]
     *
     **/
    void readShort(string name ; short value);

    /**
     *
     * Read a sequence of shorts.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The shorts that have been read.
     *
     * @see [writeShortSeq]
     *
     **/
    void readShortSeq(string name ; ShortSeq value);

    /**
     *
     * Write a single int.
     *
     * @param name The name of the parameter.
     *
     * @param value The int to write.
     *
     * @see [readInt]
     *
     **/
    void writeInt(string name, int value);

    /**
     *
     * Write a sequence of ints.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of ints to write.
     *
     * @see [readIntSeq]
     *
     **/
    void writeIntSeq(string name, IntSeq value);

    /**
     *
     * Read a single int.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The int that has been read.
     *
     * @see [writeInt]
     *
     **/
    void readInt(string name ; int value);

    /**
     *
     * Read a sequence of ints.
     *
     * @param name The name of the parameter that has been read..
     *
     * @param value The ints that have been read.
     *
     * @see [writeIntSeq]
     *
     **/
    void readIntSeq(string name ; IntSeq value);

    /**
     *
     * Write a single long.
     *
     * @param name The name of the parameter.
     *
     * @param value The long to write.
     *
     * @see [readLong]
     *
     **/
    void writeLong(string name, long value);

    /**
     *
     * Write a sequence of longs.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of longs to write.
     *
     * @see [readLongSeq]
     *
     **/
    void writeLongSeq(string name, LongSeq value);

    /**
     *
     * Read a single long.
     *
     * @param name The name of the parameter.
     *
     * @param value The long that has been read.
     *
     * @see [readLong]
     *
     **/
    void readLong(string name ; long value);

    /**
     *
     * Read a sequence of longs.
     *
     * @param name The name of the parameter.
     *
     * @param value The longs that have been read.
     *
     * @see [readLongSeq]
     *
     **/
    void readLongSeq(string name ; LongSeq value);

    /**
     *
     * Write a single float.
     *
     * @param name The name of the parameter.
     *
     * @param value The float to write.
     *
     * @see [readFloat]
     *
     **/
    void writeFloat(string name, float value);

    /**
     *
     * Write a sequence of floats.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of floats to write.
     *
     * @see [readFloatSeq]
     *
     **/
    void writeFloatSeq(string name, FloatSeq value);

    /**
     *
     * Read a single float.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The float that has been read.
     *
     * @see [writeFloat]
     *
     **/
    void readFloat(string name ; float value);

    /**
     *
     * Read a sequence of floats.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The floats that have been read.
     *
     * @see [writeFloatSeq]
     *
     **/
    void readFloatSeq(string name ; FloatSeq value);

    /**
     *
     * Write a single double.
     *
     * @param name The name of the parameter.
     *
     * @param value The double to write.
     *
     * @see [readDouble]
     *
     **/
    void writeDouble(string name, double value);

    /**
     *
     * Write a sequence of doubles.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of doubles to write.
     *
     * @see [readDoubleSeq]
     *
     **/
    void writeDoubleSeq(string name, DoubleSeq value);

    /**
     *
     * Read a single double.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The double that has been read.
     *
     * @see [writeDouble]
     *
     **/
    void readDouble(string name ; double value);

    /**
     *
     * Read a sequence of doubles.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The doubles that have been read.
     *
     * @see [writeDoubleSeq]
     *
     **/
    void readDoubleSeq(string name ; DoubleSeq value);

    /**
     *
     * Write a single string.
     *
     * @param name The name of the parameter.
     *
     * @param value The string to write.
     *
     * @see [readString]
     *
     **/
    void writeString(string name, string value);

    /**
     *
     * Write a sequence of strings.
     *
     * @param name The name of the parameter.
     *
     * @param value The sequence of strings to write.
     *
     * @see [readStringSeq]
     *
     **/
    void writeStringSeq(string name, StringSeq value);

    /**
     *
     * Read a single string.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The string that has been read.
     *
     * @see [writeString]
     *
     **/
    void readString(string name ; string value);

    /**
     *
     * Read a sequence of strings.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The strings that have been read.
     *
     * @see [writeStringSeq]
     *
     **/
    void readStringSeq(string name ; StringSeq value);

    /**
     *
     * Write a single proxy.
     *
     * @param name The name of the parameter.
     *
     * @param value The proxy to write.
     *
     * @see [readProxy]
     *
     **/
    void writeProxy(string name, Object* value);

    /**
     *
     * Read a single proxy.
     *
     * @param name The name of the parameter that has been read.
     *
     * @param value The proxy that has been read.
     *
     * @see [writeProxy]
     *
     **/
    void readProxy(string name ; Object* value);

    /**
     *
     * Write a single object.
     *
     * @param name The name of the parameter.
     *
     * @param value The object to write.
     *
     * @see [readObject]
     *
     **/
    void writeObject(string name, Object value);

    /**
     *
     * Read a single object. Uses the provided [factory] if there is
     * no suitable factory for the object available and then
     * [signatureType] parameter matches the type of the object being
     * read. If no suitable factory is available, and if the
     * [signatureType] parameter doesn't match the type of the object
     * that has been read, [NoObjectFactoryException] is raised.
     *
     * @param name The name of the parameter.
     *
     * @param signatureType The signature type, as described above.
     *
     * @param factory The object factory to use as desribed above.
     *
     * @param value The object that has been read.
     *
     * @see ObjectFactory, [readObject]
     *
     **/
    void readObject(string name, string signatureType, ObjectFactory factory; Object value);
};

};

#endif
