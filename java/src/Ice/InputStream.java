// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
     * Extracts a Slice type ID from the stream.
     *
     * @return The extracted type ID.
     **/
    String readTypeId();

    /**
     * Extracts a user exception from the stream and throws it.
     **/
    void throwException() throws UserException;

    /**
     * Reads the start of an object or exception slice.
     **/
    void startSlice();

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
     **/
    void startEncapsulation();

    /**
     * Skips over an encapsulation.
     **/
    void skipEncapsulation();

    /**
     * Indicates that the end of an encapsulation has been reached.
     **/
    void endEncapsulation();

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
     * Destroys the stream and its associated resources. The application must call <code>destroy</code> prior
     * to releasing the last reference to a stream; failure to do so may result in resource leaks.
     **/
    void destroy();
}
