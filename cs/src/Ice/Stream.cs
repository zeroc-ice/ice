// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// Callback class to inform an application when a Slice class has been unmarshaled
    /// from an input stream.
    /// </summary>
    public interface ReadObjectCallback
    {
        /// <summary>
        /// The Ice run time calls this method when it has fully unmarshaled the state
        /// of a Slice class.
        /// </summary>
        /// <param name="obj">The unmarshaled Slice class.</param>
        void invoke(Ice.Object obj);
    }

    /// <summary>
    /// Interface for input streams used to extract Slice types from a sequence of bytes.
    /// </summary>
    public interface InputStream
    {
        /// <summary>
        /// Returns the communicator for this input stream.
        /// </summary>
        /// <returns>The communicator.</returns>
        Communicator communicator();

        /// <summary>
        /// Determines the behavior of the stream when extracting Slice objects.
        /// A Slice object is "sliced" when a factory cannot be found for a Slice type ID.
        /// </summary>
        /// <param name="slice">If true (the default), slicing is enabled; if false,
        /// slicing is disabled. If slicing is disabled and the stream encounters a Slice type ID
        /// during decoding for which no object factory is installed, it raises NoObjectFactoryException.</param>
        void sliceObjects(bool slice);

        /// <summary>
        /// Extracts a boolean value from the stream.
        /// </summary>
        /// <returns>The extracted boolean.</returns>
        bool readBool();

        /// <summary>
        /// Extracts a sequence of boolean values from the stream.
        /// </summary>
        /// <returns>The extracted boolean sequence.</returns>
        bool[] readBoolSeq();

        /// <summary>
        /// Extracts a byte value from the stream.
        /// </summary>
        /// <returns>The extracted byte.</returns>
        byte readByte();

        /// <summary>
        /// Extracts a sequence of byte values from the stream.
        /// </summary>
        /// <returns>The extracted byte sequence.</returns>
        byte[] readByteSeq();

        /// <summary>
        /// Extracts a serializable .NET object from the stream.
        /// </summary>
        /// <returns>The deserialized .NET object.</returns>
        object readSerializable();

        /// <summary>
        /// Extracts a short value from the stream.
        /// </summary>
        /// <returns>The extracted short value.</returns>
        short readShort();

        /// <summary>
        /// Extracts a sequence of short values from the stream.
        /// </summary>
        /// <returns>The extracted short sequence.</returns>
        short[] readShortSeq();

        /// <summary>
        /// Extracts an integer value from the stream.
        /// </summary>
        /// <returns>The extracted integer value.</returns>
        int readInt();

        /// <summary>
        /// Extracts a sequence of integer values from the stream.
        /// </summary>
        /// <returns>The extracted integer sequence.</returns>
        int[] readIntSeq();

        /// <summary>
        /// Extracts a long value from the stream.
        /// </summary>
        /// <returns>The extracted long value.</returns>
        long readLong();

        /// <summary>
        /// Extracts a sequence of long values from the stream.
        /// </summary>
        /// <returns>The extracted long sequence.</returns>
        long[] readLongSeq();

        /// <summary>
        /// Extracts a float value from the stream.
        /// </summary>
        /// <returns>The extracted float value.</returns>
        float readFloat();

        /// <summary>
        /// Extracts a sequence of float values from the stream.
        /// </summary>
        /// <returns>The extracted float sequence.</returns>
        float[] readFloatSeq();

        /// <summary>
        /// Extracts a double value from the stream.
        /// </summary>
        /// <returns>The extracted double value.</returns>
        double readDouble();

        /// <summary>
        /// Extracts a sequence of double values from the stream.
        /// </summary>
        /// <returns>The extracted double sequence.</returns>
        double[] readDoubleSeq();

        /// <summary>
        /// Extracts a string from the stream.
        /// </summary>
        /// <returns>The extracted double value.</returns>
        string readString();

        /// <summary>
        /// Extracts a sequence of strings from the stream.
        /// </summary>
        /// <returns>The extracted string sequence.</returns>
        string[] readStringSeq();

        /// <summary>
        /// Extracts a size from the stream.
        /// </summary>
        /// <returns>The extracted size.</returns>
        int readSize();

        /// <summary>
        /// Extracts and check a sequence size from the stream. The check ensures not too much memory will
        /// be pre-allocated for the sequence.
        /// </summary>
        /// <param name="minSize">The minimum size of an element of the sequence.</param>
        /// <returns>The extracted size.</returns>
        int readAndCheckSeqSize(int minSize);

        /// <summary>
        /// Extracts a proxy from the stream.
        /// </summary>
        /// <returns>The extracted proxy.</returns>
        ObjectPrx readProxy();

        /// <summary>
        /// Extracts the index of a Slice class from the stream.
        /// </summary>
        /// <param name="cb">The callback to notify the application when the extracted instance is available.
        /// The Ice run time extracts Slice classes in stages. The Ice run time calls ReadObjectCallback.invoke
        /// when the corresponding instance has been fully unmarshaled.</param>
        void readObject(ReadObjectCallback cb);

        /// <summary>
        /// Extracts a Slice type ID from the stream.
        /// </summary>
        /// <returns>The extracted type ID.</returns>
        string readTypeId();

        /// <summary>
        /// Extracts a user exception from the stream and throws it.
        /// </summary>
        void throwException();

        /// <summary>
        /// Reads the start of an object or exception slice.
        /// </summary>
        void startSlice();

        /// <summary>
        /// Indicates that the end of an object or exception slice has been reached.
        /// </summary>
        void endSlice();

        /// <summary>
        /// Skips over an object or exception slice.
        /// </summary>
        void skipSlice();

        /// <summary>
        /// Reads the start of an encapsulation.
        /// </summary>
        void startEncapsulation();

        /// <summary>
        /// Indicates that the end of an encapsulation has been reached.
        /// </summary>
        void endEncapsulation();

        /// <summary>
        /// Skips over an encapsulation.
        /// </summary>
        void skipEncapsulation();

        /// <summary>
        /// Returns the size of the current encapsulation.
        /// </summary>
        /// <returns>The size of the current encapsulation.</returns>
        int getEncapsulationSize();

        /// <summary>
        /// Reads the specified number of bytes from the stream.
        /// </summary>
        /// <returns>The bytes read.</returns>
        byte[] readBlob(int size);

        /// <summary>
        /// Indicates that unmarshaling is complete, except for any Slice objects. The application must
        /// call this method only if the stream actually contains Slice objects. Calling readPendingObjects
        /// triggers the calls to ReadObjectCallback.invoke that inform the application that unmarshaling
        /// of a Slice object is complete.
        /// </summary>
        void readPendingObjects();

        /// <summary>
        /// Resets the read position of the stream to the beginning.
        /// </summary>
        void rewind();

        /// <summary>
        /// Destroys the stream and its associated resources. The application must call destroy prior
        /// to releasing the last reference to a stream; failure to do so may result in resource leaks.
        /// </summary>
        void destroy();
    }

    /// <summary>
    /// Interface for output streams used to write Slice types to a sequence
    /// of bytes.
    /// </summary>
    public interface OutputStream
    {
        /// <summary>
        /// Returns the communicator for this output stream.
        /// </summary>
        Communicator communicator();

        /// <summary>
        /// Writes a boolean to the stream.
        /// </summary>
        /// <param name="v">The boolean to write to the stream.</param>
        void writeBool(bool v);

        /// <summary>
        /// Writes a sequence of booleans to the stream.
        /// </summary>
        /// <param name="v">The sequence of booleans to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeBoolSeq(bool[] v);

        /// <summary>
        /// Writes a byte to the stream.
        /// </summary>
        /// <param name="v">The byte to write to the stream.</param>
        void writeByte(byte v);

        /// <summary>
        /// Writes a sequence of bytes to the stream.
        /// </summary>
        /// <param name="v">The sequence of bytes to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeByteSeq(byte[] v);

        /// <summary>
        /// Writes a serializable .NET object to the stream.
        /// </summary>
        /// <param name="v">The serializable object to write.</param>
        void writeSerializable(object v);

        /// <summary>
        /// Writes a short to the stream.
        /// </summary>
        /// <param name="v">The short to write to the stream.</param>
        void writeShort(short v);

        /// <summary>
        /// Writes a sequence of shorts to the stream.
        /// </summary>
        /// <param name="v">The sequence of shorts to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeShortSeq(short[] v);

        /// <summary>
        /// Writes an integer to the stream.
        /// </summary>
        /// <param name="v">The integer to write to the stream.</param>
        void writeInt(int v);

        /// <summary>
        /// Writes a sequence of integers to the stream.
        /// </summary>
        /// <param name="v">The sequence of integers to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeIntSeq(int[] v);

        /// <summary>
        /// Writes a long to the stream.
        /// </summary>
        /// <param name="v">The long to write to the stream.</param>
        void writeLong(long v);

        /// <summary>
        /// Writes a sequence of longs to the stream.
        /// </summary>
        /// <param name="v">The sequence of longs to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeLongSeq(long[] v);

        /// <summary>
        /// Writes a float to the stream.
        /// </summary>
        /// <param name="v">The float to write to the stream.</param>
        void writeFloat(float v);

        /// <summary>
        /// Writes a sequence of floats to the stream.
        /// </summary>
        /// <param name="v">The sequence of floats to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeFloatSeq(float[] v);

        /// <summary>
        /// Writes a double to the stream.
        /// </summary>
        /// <param name="v">The double to write to the stream.</param>
        void writeDouble(double v);

        /// <summary>
        /// Writes a sequence of doubles to the stream.
        /// </summary>
        /// <param name="v">The sequence of doubles to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeDoubleSeq(double[] v);

        /// <summary>
        /// Writes a string to the stream.
        /// </summary>
        /// <param name="v">The string to write to the stream.
        /// Passing null causes an empty string to be written to the stream.</param>
        void writeString(string v);

        /// <summary>
        /// Writes a sequence of strings to the stream.
        /// </summary>
        /// <param name="v">The sequence of strings to write.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        void writeStringSeq(string[] v);

        /// <summary>
        /// Writes a size to the stream.
        /// </summary>
        /// <param name="sz">The size to write.</param>
        void writeSize(int sz);

        /// <summary>
        /// Writes a proxy to the stream.
        /// </summary>
        /// <param name="v">The proxy to write.</param>
        void writeProxy(ObjectPrx v);

        /// <summary>
        /// Writes a Slice class to the stream.
        /// </summary>
        /// <param name="v">The class to write. This method writes the index of a Slice class; the state of the
        /// class is written once writePendingObjects is called.</param>
        void writeObject(Ice.Object v);

        /// <summary>
        /// Writes a Slice type ID to the stream.
        /// </summary>
        /// <param name="id">The Slice type ID to write.</param>
        void writeTypeId(string id);

        /// <summary>
        /// Writes a user exception to the stream.
        /// </summary>
        /// <param name="ex">The user exception to write.</param>
        void writeException(UserException ex);

        /// <summary>
        /// Writes the start of a slice to the stream.
        /// </summary>
        void startSlice();

        /// <summary>
        /// Ends the previous slice.
        /// </summary>
        void endSlice();

        /// <summary>
        /// Writes the start of an encapsulation to the stream.
        /// </summary>
        void startEncapsulation();

        /// <summary>
        /// Ends the previous encapsulation.
        /// </summary>
        void endEncapsulation();

        /// <summary>
        /// Writes the specified bytes onto the stream.
        /// </summary>
        void writeBlob(byte[] data);

        /// <summary>
        /// Writes the state of Slice classes whose index was previously
        /// written with writeObject to the stream.
        /// </summary>
        void writePendingObjects();

        /// <summary>
        /// Indicates that the marshaling of a request or reply is finished.
        /// </summary>
        /// <returns>The byte sequence containing the encoded request or reply.</returns>
        byte[] finished();

        /// <summary>
        /// Resets this output stream. This method allows the stream to be reused, to avoid creating
        /// unnecessary garbage.
        /// </summary>
        ///
        /// <param name="clearBuffer">If true, the stream's internal buffer becomes eligible for
        /// garbage collection; if false, the stream's internal buffer is retained, to avoid
        /// creating unnecessary garbage. If retained, the internal buffer may be resized to a smaller
        /// capacity. Either way, reset resets the stream's writing position to zero.</param>
        void reset(bool clearBuffer);

        /// <summary>
        /// Destroys the stream and its associated resources. The application must call destroy prior
        /// to releasing the last reference to a stream; failure to do so may result in resource leaks.
        /// </summary>
        void destroy();
    }

    /// <summary>
    /// Base class for extracting objects from an input stream.
    /// </summary>
    public abstract class ObjectReader : ObjectImpl
    {
        /// <summary>
        /// @param in The input stream to read from.
        /// @param rid If <code>true</code>, extraction begins by reading a Slice type ID
        /// first. If <code>false</code>, the leading type ID is not read. This is used
        /// by the unmarshaling code in case the type ID has already been read as part
        /// of other unmarshaling activities.
        /// </summary>
        public abstract void read(InputStream inStream, bool rid);

        public override void write__(IceInternal.BasicStream os)
        {
            Debug.Assert(false);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            InputStream stream = (InputStream)istr.closure();
            read(stream, rid);
        }
    }

    /// <summary>
    /// Base class for writing objects to an output stream.
    /// </summary>
    public abstract class ObjectWriter : ObjectImpl
    {
        /// <summary>
        /// Writes the state of this Slice class to an output stream.
        /// </summary>
        /// <param name="outStream">The stream to write to.</param>
        public abstract void write(OutputStream outStream);

        public override void write__(IceInternal.BasicStream os)
        {
            OutputStream stream = (OutputStream)os.closure();
            write(stream);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            Debug.Assert(false);
        }
    }

    public abstract class UserExceptionWriter : UserException
    {
        public UserExceptionWriter(Communicator communicator)
        {
            communicator_ = communicator;
        }

        public abstract void write(OutputStream os);
        public abstract bool usesClasses();

        public override void write__(IceInternal.BasicStream os)
        {
            OutputStream stream = (OutputStream)os.closure();
            if(stream == null)
            {
                stream = new OutputStreamI(communicator_, os);
            }
            write(stream);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            Debug.Assert(false);
        }

        public override void write__(Ice.OutputStream ostr)
        {
            write(ostr);
        }

        public override void read__(Ice.InputStream istr, bool rid)
        {
            Debug.Assert(false);
        }

        public override bool usesClasses__()
        {
            return usesClasses();
        }

        protected Communicator communicator_;
    }
}
