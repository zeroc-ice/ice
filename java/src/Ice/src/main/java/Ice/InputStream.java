// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Interface for input streams used to extract Slice types from a sequence
 * of bytes.
 *
 * @see OutputStream
 **/
public interface InputStream
{
    /**
     * Returns the communicator for this input stream.
     *
     * @return The communicator.
     **/
    Communicator communicator();

    /**
     * Determines the behavior of the stream when extracting Slice objects.
     * A Slice object is "sliced" when a factory cannot be found for a Slice type ID.
     *
     * @param slice If <code>true</code> (the default), slicing is enabled; if <code>false</code>,
     * slicing is disabled. If slicing is disabled and the stream encounters a Slice type ID
     * during decoding for which no object factory is installed, it raises {@link NoObjectFactoryException}.
     **/
    void sliceObjects(boolean slice);

    /**
     * Extracts a boolean value from the stream.
     *
     * @return The extracted boolean.
     **/
    boolean readBool();

    /**
     * Extracts a sequence of boolean values from the stream.
     *
     * @return The extracted boolean sequence.
     **/
    boolean[] readBoolSeq();

    /**
     * Extracts a byte value from the stream.
     *
     * @return The extracted byte.
     **/
    byte readByte();

    /**
     * Extracts a sequence of byte values from the stream.
     *
     * @return The extracted byte sequence.
     **/
    byte[] readByteSeq();

    /**
     * Extracts a sequence of byte values from the stream as a ByteBuffer.
     *
     * @return The ByteBuffer wrapping the sequence.
     **/
    java.nio.ByteBuffer readByteBuffer();

    /**
     * Extracts a serializable Java object from the stream.
     *
     * @return The deserialized Java object.
     **/
    java.io.Serializable readSerializable();

    /**
     * Extracts a short value from the stream.
     *
     * @return The extracted short value.
     **/
    short readShort();

    /**
     * Extracts a sequence of short values from the stream.
     *
     * @return The extracted short sequence.
     **/
    short[] readShortSeq();

    /**
     * Extracts a sequence of short values from the stream as a ShortBuffer.
     *
     * @return The ShortBuffer wrapping the sequence.
     **/
    java.nio.ShortBuffer readShortBuffer();

    /**
     * Extracts an integer value from the stream.
     *
     * @return The extracted integer.
     **/
    int readInt();

    /**
     * Extracts a sequence of integer values from the stream.
     *
     * @return The extracted integer sequence.
     **/
    int[] readIntSeq();

    /**
     * Extracts a sequence of int values from the stream as a IntBuffer.
     *
     * @return The IntBuffer wrapping the sequence.
     **/
    java.nio.IntBuffer readIntBuffer();

    /**
     * Extracts a long value from the stream.
     *
     * @return The extracted long value.
     **/
    long readLong();

    /**
     * Extracts a sequence of long values from the stream.
     *
     * @return The extracted long sequence.
     **/
    long[] readLongSeq();

    /**
     * Extracts a sequence of long values from the stream as a LongBuffer.
     *
     * @return The LongBuffer wrapping the sequence.
     **/
    java.nio.LongBuffer readLongBuffer();

    /**
     * Extracts a float value from the stream.
     *
     * @return The extracted float value.
     **/
    float readFloat();

    /**
     * Extracts a sequence of float values from the stream.
     *
     * @return The extracted float sequence.
     **/
    float[] readFloatSeq();

    /**
     * Extracts a sequence of float values from the stream as a FloatBuffer.
     *
     * @return The FloatBuffer wrapping the sequence.
     **/
    java.nio.FloatBuffer readFloatBuffer();

    /**
     * Extracts a double value from the stream.
     *
     * @return The extracted double value.
     **/
    double readDouble();

    /**
     * Extracts a sequence of double values from the stream.
     *
     * @return The extracted float sequence.
     **/
    double[] readDoubleSeq();

    /**
     * Extracts a sequence of double values from the stream as a DoubleBuffer.
     *
     * @return The DoubleBuffer wrapping the sequence.
     **/
    java.nio.DoubleBuffer readDoubleBuffer();

    /**
     * Extracts a string from the stream.
     *
     * @return The extracted string.
     **/
    String readString();

    /**
     * Extracts a string sequence from the stream.
     *
     * @return The extracted string sequence.
     */
    String[] readStringSeq();

    /**
     * Extracts a size from the stream.
     *
     * @return The extracted size.
     **/
    int readSize();

    /**
     * Extracts and check a sequence size from the stream. The check ensures not too much memory will
     * be pre-allocated for the sequence.
     *
     * @param minSize The minimum size of an element of the sequence.
     *
     * @return The extracted size.
     **/
    int readAndCheckSeqSize(int minSize);

    /**
     * Extracts a proxy from the stream.
     *
     * @return The extracted proxy.
     **/
    ObjectPrx readProxy();

    /**
     * Extracts the index of a Slice class from the stream.
     *
     * @param cb The callback to notify the application when the extracted instance is available.
     * The Ice run time extracts Slice classes in stages. The Ice run time calls {@link ReadObjectCallback#invoke}
     * when the corresponding instance has been fully unmarshaled.
     *
     * @see ReadObjectCallback
     **/
    void readObject(ReadObjectCallback cb);

    /**
     * Read an enumerated value.
     *
     * @param maxValue The maximum enumerator value in the definition.
     * @return The enumerator.
     **/
    int readEnum(int maxValue);

    /**
     * Extracts a user exception from the stream and throws it.
     **/
    void throwException() throws UserException;

    /**
     * Extracts a user exception from the stream and throws it, using the supplied
     * factory to instantiate a UserExceptionReader.
     *
     * @param factory A factory that creates UserExceptionReader instances.
     **/
    void throwException(UserExceptionReaderFactory factory) throws UserException;

    /**
     * Marks the start of an Ice object.
     **/
    void startObject();

    /**
     * Marks the end of an Ice object.
     *
     * @return A SlicedData object containing the preserved slices for unknown types.
     **/
    SlicedData endObject(boolean preserve);

    /**
     * Marks the start of a user exception.
     **/
    void startException();

    /**
     * Marks the end of a user exception.
     *
     * @return A SlicedData object containing the preserved slices for unknown types.
     **/
    SlicedData endException(boolean preserve);

    /**
     * Reads the start of an object or exception slice.
     *
     * @return The Slice type ID for this slice.
     **/
    String startSlice();

    /**
     * Indicates that the end of an object or exception slice has been reached.
     **/
    void endSlice();

    /**
     * Skips over an object or exception slice.
     **/
    void skipSlice();

    /**
     * Reads the start of an encapsulation.
     *
     * @return The encapsulation encoding version.
     **/
    EncodingVersion startEncapsulation();

    /**
     * Indicates that the end of an encapsulation has been reached.
     **/
    void endEncapsulation();

    /**
     * Skips over an encapsulation.
     *
     * @return The encapsulation encoding version.
     **/
    EncodingVersion skipEncapsulation();

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     **/
    EncodingVersion getEncoding();

    /**
     * Indicates that unmarshaling is complete, except for any Slice objects. The application must call this method
     * only if the stream actually contains Slice objects. Calling <code>readPendingObjects</code> triggers the
     * calls to {@link ReadObjectCallback#invoke} that inform the application that unmarshaling of a Slice
     * object is complete.
     **/
    void readPendingObjects();

    /**
     * Resets the read position of the stream to the beginning.
     **/
    void rewind();

    /**
     * Skips ahead in the stream.
     *
     * @param sz The number of bytes to skip.
     **/
    void skip(int sz);

    /**
     * Skips over a size value.
     **/
    void skipSize();

    /**
     * Determine if an optional value is available for reading.
     *
     * @param tag The tag associated with the value.
     * @param format The optional format for the value.
     * @return True if the value is present, false otherwise.
     **/
    boolean readOptional(int tag, OptionalFormat format);

    int pos();

    /**
     * Destroys the stream and its associated resources. The application must call <code>destroy</code> prior
     * to releasing the last reference to a stream; failure to do so may result in resource leaks.
     **/
    void destroy();
}
