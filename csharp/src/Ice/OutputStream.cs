//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Protocol = IceInternal.Protocol;

namespace Ice
{
    /// <summary>
    /// Interface for output streams used to write Slice types to a sequence
    /// of bytes.
    /// </summary>
    public class OutputStream
    {
        public Communicator Communicator { get; }
        public EncodingVersion Encoding { get; private set; }
        private InstanceData? _current;
        private Encaps? _mainEncaps;
        private Encaps? _endpointEncaps;

        // Encapsulation fields for class instance marshaling.
        private Dictionary<AnyClass, int>? _instanceMap;
        private Dictionary<string, int>? _typeIdMap;

        /// <summary>
        /// This constructor uses the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        public OutputStream(Communicator communicator)
            : this(communicator, communicator.DefaultsAndOverrides.defaultEncoding, new IceInternal.Buffer())
        {
        }

        /// <summary>
        /// This constructor uses the given communicator and encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="encoding">The desired encoding version.</param>
        public OutputStream(Communicator communicator, EncodingVersion encoding)
            : this(communicator, encoding, new IceInternal.Buffer())
        {
        }

        public OutputStream(Ice.Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
            : this(communicator, encoding, new IceInternal.Buffer(buf, adopt))
        {
        }

        private OutputStream(Ice.Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf)
        {
            Communicator = communicator;
            Encoding = encoding;
            _buf = buf;
        }

        /// <summary>
        /// Resets this output stream. This method allows the stream to be reused, to avoid creating
        /// unnecessary garbage.
        /// </summary>
        public void Reset()
        {
            _buf.reset();
            Clear();
            _format = Communicator.DefaultsAndOverrides.defaultFormat;
        }

        /// <summary>
        /// Releases any data retained by encapsulations. The reset() method internally calls clear().
        /// </summary>
        public void Clear()
        {
            ResetEncapsulation();
        }

        /// <summary>
        /// Indicates that the marshaling of a request or reply is finished.
        /// </summary>
        /// <returns>The byte sequence containing the encoded request or reply.</returns>
        public byte[] Finished()
        {
            IceInternal.Buffer buf = PrepareWrite();
            byte[] result = new byte[buf.b.limit()];
            buf.b.get(result);
            return result;
        }

        /// <summary>
        /// Swaps the contents of one stream with another.
        /// </summary>
        /// <param name="other">The other stream.</param>
        public void Swap(OutputStream other)
        {
            Debug.Assert(Communicator == other.Communicator);

            IceInternal.Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            EncodingVersion tmpEncoding = other.Encoding;
            other.Encoding = Encoding;
            Encoding = tmpEncoding;

            //
            // Swap is never called for streams that have encapsulations being written. However,
            // encapsulations might still be set in case marshalling failed. We just
            // reset the encapsulations if there are still some set.
            //
            ResetEncapsulation();
            other.ResetEncapsulation();
        }

        private void ResetEncapsulation()
        {
            _mainEncaps = null;
            _endpointEncaps = null;
            _current = null;
            _instanceMap?.Clear();
            _typeIdMap?.Clear();
        }

        /// <summary>
        /// Resizes the stream to a new size.
        /// </summary>
        /// <param name="sz">The new size.</param>
        public void Resize(int sz)
        {
            _buf.resize(sz, false);
            _buf.b.position(sz);
        }

        /// <summary>
        /// Prepares the internal data buffer to be written to a socket.
        /// </summary>
        public IceInternal.Buffer PrepareWrite()
        {
            _buf.b.limit(_buf.size());
            _buf.b.position(0);
            return _buf;
        }

        /// <summary>
        /// Retrieves the internal data buffer.
        /// </summary>
        /// <returns>The buffer.</returns>
        public IceInternal.Buffer GetBuffer()
        {
            return _buf;
        }

        /// <summary>
        /// Writes the start of an encapsulation to the stream.
        /// </summary>
        public void StartEncapsulation()
        {
            StartEncapsulation(Encoding);
        }

        internal void StartEndpointEncapsulation()
        {
            StartEndpointEncapsulation(Encoding);
        }

        /// <summary>
        /// Writes the start of an encapsulation to the stream.
        /// </summary>
        /// <param name="encoding">The encoding version of the encapsulation.</param>
        /// <param name="format">Specify the compact or sliced format; when null, uses the communicator's default
        /// format.</param>
        public void StartEncapsulation(EncodingVersion encoding, FormatType? format = null)
        {
            Debug.Assert(_mainEncaps == null && _endpointEncaps == null);
            Protocol.checkSupportedEncoding(encoding);

            _mainEncaps = new Encaps(Encoding, _format, _buf.b.position());

            Encoding = encoding;
            if (format.HasValue)
            {
                _format = format.Value;
            }

            WriteInt(0); // Placeholder for the encapsulation length.
            WriteByte(Encoding.major);
            WriteByte(Encoding.minor);
        }

        internal void StartEndpointEncapsulation(EncodingVersion encoding)
        {
            Debug.Assert(_endpointEncaps == null);
            Protocol.checkSupportedEncoding(encoding);

            _endpointEncaps = new Encaps(Encoding, _format, _buf.b.position());
            Encoding = encoding;
            // we don't change format

            WriteInt(0); // Placeholder for the encapsulation length.
            WriteByte(Encoding.major);
            WriteByte(Encoding.minor);
        }

        /// <summary>
        /// Ends the previous main encapsulation.
        /// </summary>
        public void EndEncapsulation()
        {
            Debug.Assert(_mainEncaps.HasValue && _endpointEncaps == null);

            // Size includes size and version.
            int start = _mainEncaps.Value.Start;
            int sz = _buf.size() - start;
            _buf.b.putInt(start, sz);

            Encoding = _mainEncaps.Value.OldEncoding;
            _format = _mainEncaps.Value.OldFormat;
            ResetEncapsulation();
        }

        internal void EndEndpointEncapsulation()
        {
            Debug.Assert(_endpointEncaps.HasValue);

            // Size includes size and version.
            int start = _endpointEncaps.Value.Start;
            int sz = _buf.size() - start;
            _buf.b.putInt(start, sz);

            Encoding = _endpointEncaps.Value.OldEncoding;
            // No need to restore format since it didn't change.
            _endpointEncaps = null;
        }

        /// <summary>
        /// Writes an empty encapsulation using the given encoding version.
        /// </summary>
        /// <param name="encoding">The encoding version of the encapsulation.</param>
        public void WriteEmptyEncapsulation(EncodingVersion encoding)
        {
            Protocol.checkSupportedEncoding(encoding);
            WriteInt(6); // Size
            WriteByte(encoding.major);
            WriteByte(encoding.minor);
        }

        /// <summary>
        /// Writes a pre-encoded encapsulation.
        /// </summary>
        /// <param name="v">The encapsulation data.</param>
        public void WriteEncapsulation(byte[] v)
        {
            if (v.Length < 6)
            {
                throw new EncapsulationException();
            }
            expand(v.Length);
            _buf.b.put(v);
        }

        /// <summary>
        /// Marks the start of a new slice for a class instance or user exception.
        /// </summary>
        public void IceStartSlice(string typeId, bool firstSlice, SlicedData? slicedData = null, int? compactId = null)
        {
            Debug.Assert(_mainEncaps != null);
            StartSliceImpl(typeId, firstSlice, slicedData, compactId);
        }

        /// <summary>
        /// Marks the end of a slice for a class instance or user exception.
        /// </summary>
        public void IceEndSlice(bool lastSlice)
        {
            Debug.Assert(_mainEncaps != null);
            endSlice(lastSlice);
        }

        /// <summary>
        /// Writes a size to the stream.
        /// </summary>
        /// <param name="v">The size to write.</param>
        public void WriteSize(int v)
        {
            if (v > 254)
            {
                expand(5);
                _buf.b.put(255);
                _buf.b.putInt(v);
            }
            else
            {
                expand(1);
                _buf.b.put((byte)v);
            }
        }

        /// <summary>
        /// Returns the current position and allocates four bytes for a fixed-length (32-bit) size value.
        /// </summary>
        public int StartSize()
        {
            int pos = _buf.b.position();
            WriteInt(0); // Placeholder for 32-bit size
            return pos;
        }

        /// <summary>
        /// Computes the amount of data written since the previous call to startSize and writes that value
        /// at the saved position.
        /// </summary>
        /// <param name="pos">The saved position.</param>
        public void EndSize(int pos)
        {
            Debug.Assert(pos >= 0);
            RewriteInt(_buf.b.position() - pos - 4, pos);
        }

        /// <summary>
        /// Writes a blob of bytes to the stream.
        /// </summary>
        /// <param name="v">The byte array to be written. All of the bytes in the array are written.</param>
        public void WriteBlob(byte[] v)
        {
            if (v == null)
            {
                return;
            }
            expand(v.Length);
            _buf.b.put(v);
        }

        /// <summary>
        /// Write the header information for an optional value.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="format">The optional format of the value.</param>
        public bool WriteOptional(int tag, OptionalFormat format)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            return writeOptional(tag, format);
        }

        /// <summary>
        /// Writes a byte to the stream.
        /// </summary>
        /// <param name="v">The byte to write to the stream.</param>
        public void WriteByte(byte v)
        {
            expand(1);
            _buf.b.put(v);
        }

        /// <summary>
        /// Writes an optional byte to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The byte to write to the stream.</param>
        public void WriteByte(int tag, byte? v)
        {
            if (v is byte value && WriteOptional(tag, OptionalFormat.F1))
            {
                WriteByte(value);
            }
        }

        /// <summary>
        /// Writes a byte to the stream at the given position. The current position of the stream is not modified.
        /// </summary>
        /// <param name="v">The byte to write to the stream.</param>
        /// <param name="dest">The position at which to store the byte in the buffer.</param>
        private void RewriteByte(byte v, int dest)
        {
            _buf.b.put(dest, v);
        }

        /// <summary>
        /// Writes a byte sequence to the stream.
        /// </summary>
        /// <param name="v">The byte sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteByteSeq(byte[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length);
                _buf.b.put(v);
            }
        }

        /// <summary>
        /// Writes a byte sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteByteSeq(int count, IEnumerable<byte> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<byte>? value = v as List<byte>;
                if (value != null)
                {
                    WriteByteSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<byte>? value = v as LinkedList<byte>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count);
                    IEnumerator<byte> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.put(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<byte>? value = v as Queue<byte>;
                if (value != null)
                {
                    WriteByteSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<byte>? value = v as Stack<byte>;
                if (value != null)
                {
                    WriteByteSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count);
            foreach (byte b in v)
            {
                _buf.b.put(b);
            }
        }

        /// <summary>
        /// Writes an optional byte sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The byte sequence to write to the stream.</param>
        public void WriteByteSeq(int tag, byte[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteByteSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional byte sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the byte sequence.</param>
        public void WriteByteSeq(int tag, int count, IEnumerable<byte> v)
        {
            if (WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteByteSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a serializable object to the stream.
        /// </summary>
        /// <param name="o">The serializable object to write.</param>
        public void WriteSerializable(object o)
        {
            if (o == null)
            {
                WriteSize(0);
                return;
            }
            try
            {
                IceInternal.OutputStreamWrapper w = new IceInternal.OutputStreamWrapper(this);
                IFormatter f = new BinaryFormatter();
                f.Serialize(w, o);
                w.Close();
            }
            catch (System.Exception ex)
            {
                throw new MarshalException("cannot serialize object:", ex);
            }
        }

        /// <summary>
        /// Writes a boolean to the stream.
        /// </summary>
        /// <param name="v">The boolean to write to the stream.</param>
        public void WriteBool(bool v)
        {
            expand(1);
            _buf.b.put(v ? (byte)1 : (byte)0);
        }

        /// <summary>
        /// Writes an optional boolean to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The boolean to write to the stream.</param>
        public void WriteBool(int tag, bool? v)
        {
            if (v is bool value && WriteOptional(tag, OptionalFormat.F1))
            {
                WriteBool(value);
            }
        }

        /// <summary>
        /// Writes a boolean sequence to the stream.
        /// </summary>
        /// <param name="v">The boolean sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteBoolSeq(bool[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length);
                _buf.b.putBoolSeq(v);
            }
        }

        /// <summary>
        /// Writes a boolean sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteBoolSeq(int count, IEnumerable<bool> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<bool>? value = v as List<bool>;
                if (value != null)
                {
                    WriteBoolSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<bool>? value = v as LinkedList<bool>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count);
                    IEnumerator<bool> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putBool(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<bool>? value = v as Queue<bool>;
                if (value != null)
                {
                    WriteBoolSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<bool>? value = v as Stack<bool>;
                if (value != null)
                {
                    WriteBoolSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count);
            foreach (bool b in v)
            {
                _buf.b.putBool(b);
            }
        }

        /// <summary>
        /// Writes an optional boolean sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The optional boolean sequence to write to the stream.</param>
        public void WriteBoolSeq(int tag, bool[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteBoolSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional boolean sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the optional boolean sequence.</param>
        public void WriteBoolSeq<T>(int tag, int count, T? v) where T : class, IEnumerable<bool>
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteBoolSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a short to the stream.
        /// </summary>
        /// <param name="v">The short to write to the stream.</param>
        public void WriteShort(short v)
        {
            expand(2);
            _buf.b.putShort(v);
        }

        /// <summary>
        /// Writes an optional short to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The short to write to the stream.</param>
        public void WriteShort(int tag, short? v)
        {
            if (v is short value && WriteOptional(tag, OptionalFormat.F2))
            {
                WriteShort(value);
            }
        }

        /// <summary>
        /// Writes a short sequence to the stream.
        /// </summary>
        /// <param name="v">The short sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteShortSeq(short[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length * 2);
                _buf.b.putShortSeq(v);
            }
        }

        /// <summary>
        /// Writes a short sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteShortSeq(int count, IEnumerable<short> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<short>? value = v as List<short>;
                if (value != null)
                {
                    WriteShortSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<short>? value = v as LinkedList<short>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count * 2);
                    IEnumerator<short> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putShort(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<short>? value = v as Queue<short>;
                if (value != null)
                {
                    WriteShortSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<short>? value = v as Stack<short>;
                if (value != null)
                {
                    WriteShortSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count * 2);
            foreach (short s in v)
            {
                _buf.b.putShort(s);
            }
        }

        /// <summary>
        /// Writes an optional short sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The short sequence to write to the stream.</param>
        public void WriteShortSeq(int tag, short[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : v.Length * 2 + (v.Length > 254 ? 5 : 1));
                WriteShortSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional short sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the short sequence.</param>
        public void WriteShortSeq(int tag, int count, IEnumerable<short>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(count == 0 ? 1 : count * 2 + (count > 254 ? 5 : 1));
                WriteShortSeq(count, v);
            }
        }

        /// <summary>
        /// Writes an int to the stream.
        /// </summary>
        /// <param name="v">The int to write to the stream.</param>
        public void WriteInt(int v)
        {
            expand(4);
            _buf.b.putInt(v);
        }

        /// <summary>
        /// Writes an optional int to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The int to write to the stream.</param>
        public void WriteInt(int tag, int? v)
        {
            if (v is int value && WriteOptional(tag, OptionalFormat.F4))
            {
                WriteInt(value);
            }
        }

        /// <summary>
        /// Writes an int to the stream at the given position. The current position of the stream is not modified.
        /// </summary>
        /// <param name="v">The int to write to the stream.</param>
        /// <param name="dest">The position at which to store the int in the buffer.</param>
        internal void RewriteInt(int v, int dest)
        {
            _buf.b.putInt(dest, v);
        }

        /// <summary>
        /// Writes an int sequence to the stream.
        /// </summary>
        /// <param name="v">The int sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteIntSeq(int[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length * 4);
                _buf.b.putIntSeq(v);
            }
        }

        /// <summary>
        /// Writes an int sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteIntSeq(int count, IEnumerable<int> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<int>? value = v as List<int>;
                if (value != null)
                {
                    WriteIntSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<int>? value = v as LinkedList<int>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count * 4);
                    IEnumerator<int> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putInt(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<int>? value = v as Queue<int>;
                if (value != null)
                {
                    WriteIntSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<int>? value = v as Stack<int>;
                if (value != null)
                {
                    WriteIntSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count * 4);
            foreach (int i in v)
            {
                _buf.b.putInt(i);
            }
        }

        /// <summary>
        /// Writes an optional int sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The int sequence to write to the stream.</param>
        public void WriteIntSeq(int tag, int[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : v.Length * 4 + (v.Length > 254 ? 5 : 1));
                WriteIntSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional int sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the int sequence.</param>
        public void WriteIntSeq(int tag, int count, IEnumerable<int>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                WriteIntSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a long to the stream.
        /// </summary>
        /// <param name="v">The long to write to the stream.</param>
        public void WriteLong(long v)
        {
            expand(8);
            _buf.b.putLong(v);
        }

        /// <summary>
        /// Writes an optional long to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The long to write to the stream.</param>
        public void WriteLong(int tag, long? v)
        {
            if (v is long value && WriteOptional(tag, OptionalFormat.F8))
            {
                WriteLong(value);
            }
        }

        /// <summary>
        /// Writes a long sequence to the stream.
        /// </summary>
        /// <param name="v">The long sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteLongSeq(long[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length * 8);
                _buf.b.putLongSeq(v);
            }
        }

        /// <summary>
        /// Writes a long sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteLongSeq(int count, IEnumerable<long> v)
        {
            if (count == 0 || v == null)
            {
                WriteSize(0);
                return;
            }

            {
                List<long>? value = v as List<long>;
                if (value != null)
                {
                    WriteLongSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<long>? value = v as LinkedList<long>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count * 8);
                    IEnumerator<long> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putLong(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<long>? value = v as Queue<long>;
                if (value != null)
                {
                    WriteLongSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<long>? value = v as Stack<long>;
                if (value != null)
                {
                    WriteLongSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count * 8);
            foreach (long l in v)
            {
                _buf.b.putLong(l);
            }
        }

        /// <summary>
        /// Writes an optional long sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The long sequence to write to the stream.</param>
        public void WriteLongSeq(int tag, long[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : v.Length * 8 + (v.Length > 254 ? 5 : 1));
                WriteLongSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional long sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the long sequence.</param>
        public void WriteLongSeq(int tag, int count, IEnumerable<long>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                WriteLongSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a float to the stream.
        /// </summary>
        /// <param name="v">The float to write to the stream.</param>
        public void WriteFloat(float v)
        {
            expand(4);
            _buf.b.putFloat(v);
        }

        /// <summary>
        /// Writes an optional float to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The float to write to the stream.</param>
        public void WriteFloat(int tag, float? v)
        {
            if (v is float value && WriteOptional(tag, OptionalFormat.F4))
            {
                WriteFloat(value);
            }
        }

        /// <summary>
        /// Writes a float sequence to the stream.
        /// </summary>
        /// <param name="v">The float sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteFloatSeq(float[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length * 4);
                _buf.b.putFloatSeq(v);
            }
        }

        /// <summary>
        /// Writes a float sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteFloatSeq(int count, IEnumerable<float> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<float>? value = v as List<float>;
                if (value != null)
                {
                    WriteFloatSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<float>? value = v as LinkedList<float>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count * 4);
                    IEnumerator<float> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putFloat(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<float>? value = v as Queue<float>;
                if (value != null)
                {
                    WriteFloatSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<float>? value = v as Stack<float>;
                if (value != null)
                {
                    WriteFloatSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count * 4);
            foreach (float f in v)
            {
                _buf.b.putFloat(f);
            }
        }

        /// <summary>
        /// Writes an optional float sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The float sequence to write to the stream.</param>
        public void WriteFloatSeq(int tag, float[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : v.Length * 4 + (v.Length > 254 ? 5 : 1));
                WriteFloatSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional float sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the float sequence.</param>
        public void WriteFloatSeq(int tag, int count, IEnumerable<float>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(count == 0 ? 1 : count * 4 + (count > 254 ? 5 : 1));
                WriteFloatSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a double to the stream.
        /// </summary>
        /// <param name="v">The double to write to the stream.</param>
        public void WriteDouble(double v)
        {
            expand(8);
            _buf.b.putDouble(v);
        }

        /// <summary>
        /// Writes an optional double to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The double to write to the stream.</param>
        public void WriteDouble(int tag, double? v)
        {
            if (v is double value && WriteOptional(tag, OptionalFormat.F8))
            {
                WriteDouble(value);
            }
        }

        /// <summary>
        /// Writes a double sequence to the stream.
        /// </summary>
        /// <param name="v">The double sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteDoubleSeq(double[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                expand(v.Length * 8);
                _buf.b.putDoubleSeq(v);
            }
        }

        /// <summary>
        /// Writes a double sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteDoubleSeq(int count, IEnumerable<double> v)
        {
            if (count == 0)
            {
                WriteSize(0);
                return;
            }

            {
                List<double>? value = v as List<double>;
                if (value != null)
                {
                    WriteDoubleSeq(value.ToArray());
                    return;
                }
            }

            {
                LinkedList<double>? value = v as LinkedList<double>;
                if (value != null)
                {
                    WriteSize(count);
                    expand(count * 8);
                    IEnumerator<double> i = v.GetEnumerator();
                    while (i.MoveNext())
                    {
                        _buf.b.putDouble(i.Current);
                    }
                    return;
                }
            }

            {
                Queue<double>? value = v as Queue<double>;
                if (value != null)
                {
                    WriteDoubleSeq(value.ToArray());
                    return;
                }
            }

            {
                Stack<double>? value = v as Stack<double>;
                if (value != null)
                {
                    WriteDoubleSeq(value.ToArray());
                    return;
                }
            }

            WriteSize(count);
            expand(count * 8);
            foreach (double d in v)
            {
                _buf.b.putDouble(d);
            }
        }

        /// <summary>
        /// Writes an optional double sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The double sequence to write to the stream.</param>
        public void WriteDoubleSeq(int tag, double[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : v.Length * 8 + (v.Length > 254 ? 5 : 1));
                WriteDoubleSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional double sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the double sequence.</param>
        public void WriteDoubleSeq(int tag, int count, IEnumerable<double>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(count == 0 ? 1 : count * 8 + (count > 254 ? 5 : 1));
                WriteDoubleSeq(count, v);
            }
        }

        private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>
        /// Writes a string to the stream.
        /// </summary>
        /// <param name="v">The string to write to the stream. Passing null causes
        /// an empty string to be written to the stream.</param>
        public void WriteString(string? v)
        {
            if (v == null || v.Length == 0)
            {
                WriteSize(0);
                return;
            }
            byte[] arr = utf8.GetBytes(v);
            WriteSize(arr.Length);
            expand(arr.Length);
            _buf.b.put(arr);
        }

        /// <summary>
        /// Writes an optional string to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The string to write to the stream.</param>
        public void WriteString(int tag, string? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteString(v);
            }
        }

        /// <summary>
        /// Writes a string sequence to the stream.
        /// </summary>
        /// <param name="v">The string sequence to write to the stream.
        /// Passing null causes an empty sequence to be written to the stream.</param>
        public void WriteStringSeq(string[]? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                for (int i = 0; i < v.Length; i++)
                {
                    WriteString(v[i]);
                }
            }
        }

        /// <summary>
        /// Writes a string sequence to the stream.
        /// </summary>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the container holding the sequence.</param>
        public void WriteStringSeq(int count, IEnumerable<string> v)
        {
            WriteSize(count);
            foreach (string s in v)
            {
                WriteString(s);
            }
        }

        /// <summary>
        /// Writes an optional string sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The string sequence to write to the stream.</param>
        public void WriteStringSeq(int tag, string[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteStringSeq(v);
                EndSize(pos);
            }
        }

        /// <summary>
        /// Writes an optional string sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="count">The number of elements in the sequence.</param>
        /// <param name="v">An enumerator for the string sequence.</param>
        public void WriteStringSeq(int tag, int count, IEnumerable<string>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteStringSeq(count, v);
                EndSize(pos);
            }
        }

        /// <summary>
        /// Writes a proxy to the stream.
        /// </summary>
        /// <param name="v">The proxy to write.</param>
        public void WriteProxy(IObjectPrx? v)
        {
            if (v != null)
            {
                v.IceWrite(this);
            }
            else
            {
                new Identity().IceWrite(this);
            }
        }

        /// <summary>
        /// Writes an optional proxy to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The proxy to write.</param>
        public void WriteProxy(int tag, IObjectPrx? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteProxy(v);
                EndSize(pos);
            }
        }

        /// <summary>
        /// Writes an enumerated value.
        /// </summary>
        /// <param name="v">The enumerator.</param>
        /// <param name="maxValue">The maximum enumerator value in the definition.</param>
        public void WriteEnum(int v, int maxValue)
        {
            if (Encoding.Equals(Util.Encoding_1_0))
            {
                if (maxValue < 127)
                {
                    WriteByte((byte)v);
                }
                else if (maxValue < 32767)
                {
                    WriteShort((short)v);
                }
                else
                {
                    WriteInt(v);
                }
            }
            else
            {
                WriteSize(v);
            }
        }

        /// <summary>
        /// Writes an optional enumerator to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The enumerator.</param>
        /// <param name="maxValue">The maximum enumerator value in the definition.</param>
        public void WriteEnum(int tag, int? v, int maxValue)
        {
            if (v is int value && WriteOptional(tag, OptionalFormat.Size))
            {
                WriteEnum(value, maxValue);
            }
        }

        /// <summary>
        /// Writes a class instance to the stream.
        /// </summary>
        /// <param name="v">The value to write.</param>
        public void WriteClass(AnyClass? v)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            WriteClassImpl(v);
        }

        /// <summary>
        /// Writes an optional class instance to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The value to write.</param>
        public void WriteClass(int tag, AnyClass? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.Class))
            {
                WriteClass(v);
            }
        }

        /// <summary>
        /// Writes a user exception to the stream.
        /// </summary>
        /// <param name="v">The user exception to write.</param>
        public void WriteException(UserException v)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null && _current == null);
            Push(InstanceType.Exception);
            v.Write(this);
            Pop(null);
        }

        private bool writeOptionalImpl(int tag, OptionalFormat format)
        {
            if (Encoding.Equals(Util.Encoding_1_0))
            {
                return false; // Optional members aren't supported with the 1.0 encoding.
            }

            int v = (int)format;
            if (tag < 30)
            {
                v |= tag << 3;
                WriteByte((byte)v);
            }
            else
            {
                v |= 0x0F0; // tag = 30
                WriteByte((byte)v);
                WriteSize(tag);
            }
            return true;
        }

        /// <summary>
        /// Determines the current position in the stream.
        /// </summary>
        /// <returns>The current position.</returns>
        public int pos()
        {
            return _buf.b.position();
        }

        /// <summary>
        /// Sets the current position in the stream.
        /// </summary>
        /// <param name="n">The new position.</param>
        public void pos(int n)
        {
            _buf.b.position(n);
        }

        /// <summary>
        /// Determines the current size of the stream.
        /// </summary>
        /// <returns>The current size.</returns>
        public int size()
        {
            return _buf.size();
        }

        /// <summary>
        /// Determines whether the stream is empty.
        /// </summary>
        /// <returns>True if no data has been written yet, false otherwise.</returns>
        public bool isEmpty()
        {
            return _buf.empty();
        }

        /// <summary>
        /// Expand the stream to accept more data.
        /// </summary>
        /// <param name="n">The number of bytes to accommodate in the stream.</param>
        public void expand(int n)
        {
            _buf.expand(n);
        }

        private IceInternal.Buffer _buf;
        private FormatType _format;

        private enum InstanceType { Class, Exception }

        private int RegisterTypeId(string typeId)
        {
            _typeIdMap ??= new Dictionary<string, int>();

            if (_typeIdMap.TryGetValue(typeId, out int index))
            {
                return index;
            }
            else
            {
                index = _typeIdMap.Count + 1;
                _typeIdMap.Add(typeId, index);
                return -1;
            }
        }

        private void WriteClassImpl(AnyClass? v)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else if (_current != null && _format == FormatType.SlicedFormat)
            {
                _current.IndirectionTable ??= new List<AnyClass>();
                _current.IndirectionMap ??= new Dictionary<AnyClass, int>();

                // If writing an instance within a slice and using the sliced format, write an index of that slice's
                // indirection table.
                int index = 0;
                if (_current.IndirectionMap != null && _current.IndirectionMap.TryGetValue(v, out index))
                {
                    // Found, index is position in indirection table + 1
                    Debug.Assert(index > 0);
                }
                else
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    _current.IndirectionMap ??= new Dictionary<AnyClass, int>();

                    _current.IndirectionTable.Add(v);
                    index = _current.IndirectionTable.Count; // Position + 1 (0 is reserved for null)
                    _current.IndirectionMap.Add(v, index);
                }
                WriteSize(index);
            }
            else
            {
                writeInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        private void StartSliceImpl(string typeId, bool firstSlice, SlicedData? slicedData, int? compactId)
        {
            Debug.Assert(_current != null);
            if (slicedData.HasValue)
            {
                Debug.Assert(firstSlice);
                if (WriteSlicedData(slicedData.Value))
                {
                    firstSlice = false;
                } // else we didn't write anything and it's still the first slice
            }

            _current.SliceFlagsPos = pos();
            _current.SliceFlags = 0;

            if (_format == FormatType.SlicedFormat)
            {
                //
                // Encode the slice size if using the sliced format.
                //
                _current.SliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
            }

            WriteByte(0); // Placeholder for the slice flags

            //
            // For instance slices, encode the flag and the type ID either as a
            // string or index. For exception slices, always encode the type
            // ID a string.
            //
            if (_current.InstanceType == InstanceType.Class)
            {
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                // This  also shows that the firtSlice is currently useful/used only for class instances in
                // compact format.
                if (_format == FormatType.SlicedFormat || firstSlice)
                {
                    if (compactId.HasValue)
                    {
                        _current.SliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                        WriteSize(compactId.Value);
                    }
                    else
                    {
                        int index = RegisterTypeId(typeId);
                        if (index < 0)
                        {
                            _current.SliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING;
                            WriteString(typeId);
                        }
                        else
                        {
                            _current.SliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX;
                            WriteSize(index);
                        }
                    }
                }
            }
            else
            {
                WriteString(typeId);
            }

            if ((_current.SliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                WriteInt(0); // Placeholder for the slice length.
            }

            _current.SliceFirstMemberPos = pos();
        }

        private void endSlice(bool lastSlice)
        {
            Debug.Assert(_current != null);

            if (lastSlice)
            {
                _current.SliceFlags |= Protocol.FLAG_IS_LAST_SLICE;
            }

            // Write the tagged member end marker if some tagged members were encoded. Note that the optional members
            // are encoded before the indirection table and are included in the slice size.
            if ((_current.SliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                WriteByte(Protocol.OPTIONAL_END_MARKER);
            }

            // Write the slice size if necessary.
            if ((_current.SliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                int sz = pos() - _current.SliceFirstMemberPos + 4;
                RewriteInt(sz, _current.SliceFirstMemberPos - 4);
            }

            if (_current.IndirectionTable?.Count > 0)
            {
                Debug.Assert(_format == FormatType.SlicedFormat);
                _current.SliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE;

                WriteSize(_current.IndirectionTable.Count);
                foreach (var v in _current.IndirectionTable)
                {
                    writeInstance(v);
                }
                _current.IndirectionTable.Clear();
                _current.IndirectionMap?.Clear();
            }

            // Update SliceFlags in case they were updated.
            RewriteByte(_current.SliceFlags, _current.SliceFlagsPos);
        }

        private bool writeOptional(int tag, OptionalFormat format)
        {
            if (_current == null)
            {
                return writeOptionalImpl(tag, format);
            }
            else
            {
                if (writeOptionalImpl(tag, format))
                {
                    _current.SliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        // Returns true when something was written, and false otherwise
        internal bool WriteSlicedData(SlicedData slicedData)
        {
            // We only remarshal preserved slices if we are using the sliced format. Otherwise, we ignore the preserved
            // slices, which essentially "slices" the instance into the most-derived type known by the sender.
            if (_format != FormatType.SlicedFormat || Encoding != slicedData.Encoding)
            {
                // TODO: if the encodings don't match, do we just drop these slices, or throw an exception?
                return false;
            }

            bool firstSlice = true;
            foreach (var info in slicedData.Slices)
            {
                StartSliceImpl(info.TypeId ?? "", firstSlice, null, info.CompactId);
                firstSlice = false;

                // Write the bytes associated with this slice. TODO: need better write bytes API
                var sliceBytes = new byte[info.Bytes.Count];
                info.Bytes.CopyTo(sliceBytes, 0);
                WriteBlob(sliceBytes);

                if (info.HasOptionalMembers)
                {
                    _current.SliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                // Make sure to also re-write the instance indirection table.
                // These instances will be marshaled (and assigned instance IDs) in endSlice.
                if (info.Instances.Count > 0)
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    _current.IndirectionTable.AddRange(info.Instances);
                }
                endSlice(info.IsLastSlice); // TODO: can we check it's indeed the last slice?
            }
            return firstSlice == false; // we wrote at least one slice
        }

        private void writeInstance(AnyClass v)
        {
            Debug.Assert(v != null);

            // If the instance was already marshaled, just write its instance ID.
            if (_instanceMap != null && _instanceMap.TryGetValue(v, out int instanceId))
            {
                WriteSize(instanceId);
                return;
            }
            else
            {
                _instanceMap ??= new Dictionary<AnyClass, int>();

                // We haven't seen this instance previously, so we create a new instance ID and insert the instance
                // and its ID in the marshaled map, before writing the instance inline.
                // The instance IDs start at 2 (0 means null and 1 means the instance is written immediately after).
                instanceId = _instanceMap.Count + 2;
                _instanceMap.Add(v, instanceId);

                WriteSize(1); // Class instance marker.

                var savedInstanceData = Push(InstanceType.Class);
                v.Write(this);
                Pop(savedInstanceData);
            }
        }

        private InstanceData? Push(InstanceType instanceType)
        {
            var savedInstanceData = _current;
            _current = new InstanceData(instanceType);
            return savedInstanceData;
        }

        private void Pop(InstanceData? savedInstanceData)
        {
            _current = savedInstanceData;
        }

        private sealed class InstanceData
        {
            internal readonly InstanceType InstanceType;

            // The following fields are used and reused for all the slices of a class or exception instance.
            internal byte SliceFlags = 0;

            // Position of the first data member in the slice, just after the optional slice size.
            internal int SliceFirstMemberPos = 0;

            // Position of the slice flags.
            internal int SliceFlagsPos = 0;

            // The indirection table and indirection map are only used for the sliced format.
            internal List<AnyClass>? IndirectionTable;
            internal Dictionary<AnyClass, int>? IndirectionMap;

            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;
        }

        private readonly struct Encaps
        {
            // Old Encoding
            internal readonly EncodingVersion OldEncoding;

            // Previous format (compact or sliced).
            internal readonly FormatType OldFormat;

            internal readonly int Start;

            internal Encaps(EncodingVersion oldEncoding, FormatType oldFormat, int start)
            {
                OldEncoding = oldEncoding;
                OldFormat = oldFormat;
                Start = start;
            }
        }
    }
}
