//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Protocol = IceInternal.Protocol;

namespace Ice
{
    public interface IStreamableStruct // value with the value-type semantics
    {
        public void IceWrite(OutputStream ostr);
    }

    public delegate void OutputStreamWriter<T>(OutputStream os, T value);

    /// <summary>
    /// Interface for output streams used to write Slice types to a sequence of bytes.
    /// </summary>
    public class OutputStream
    {
        /// <summary>
        /// The communicator associated with this stream.
        /// </summary>
        /// <value>The communicator.</value>
        public Communicator Communicator { get; }

        /// <summary>
        /// The encoding used when writing from this stream.
        /// </summary>
        /// <value>The encoding.</value>
        public EncodingVersion Encoding { get; private set; }

        /// <summary>
        /// Determines the current size of the stream.
        /// </summary>
        /// <value>The current size.</value>
        internal int Size => _buf.Size();

        /// <summary>
        /// Determines whether the stream is empty.
        /// </summary>
        /// <returns>True if no data has been written yet, false otherwise.</returns>
        internal bool IsEmpty => _buf.Empty();

        // The position (offset) in the underlying buffer.
        internal int Pos
        {
            get => _buf.B.Position();
            set => _buf.B.Position(value);
        }

        // When set, we are writing to a top-level encapsulation.
        private Encaps? _mainEncaps;

        // When set, we are writing an endpoint encapsulation. An endpoint encaps is a lightweight encaps that cannot
        // contain classes, exceptions, tagged members/parameters, or another endpoint. It is often but not always set
        // when _mainEncaps is set (so nested inside _mainEncaps).
        private Encaps? _endpointEncaps;

        private IceInternal.Buffer _buf;

        // The current class/exception format, can be either Compact or Sliced.
        private FormatType _format;

        // Map of class instance to instance ID, where the instance IDs start at 2.
        // When writing a top-level encapsulation:
        //  - Instance ID = 0 means null.
        //  - Instance ID = 1 means the instance is encoded inline afterwards.
        //  - Instance ID > 1 means a reference to a previously encoded instance, found in this map.
        private Dictionary<AnyClass, int>? _instanceMap;

        // Map of type ID string to type ID index.
        // When writing into a top-level encapsulation, we assign a type ID index (starting with 1) to each type ID we
        // write, in order.
        private Dictionary<string, int>? _typeIdMap;

        // Data for the class or exception instance that is currently getting marshaled.
        private InstanceData? _current;

        /// <summary>
        /// This constructor uses the communicator's default encoding version.
        /// </summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        public OutputStream(Communicator communicator)
            : this(communicator, communicator.DefaultsAndOverrides.DefaultEncoding, new IceInternal.Buffer())
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

        public OutputStream(Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf, bool adopt)
            : this(communicator, encoding, new IceInternal.Buffer(buf, adopt))
        {
        }

        /// <summary>
        /// Indicates that the marshaling of a request or reply is finished.
        /// </summary>
        /// <returns>The byte sequence containing the encoded request or reply.</returns>
        public byte[] Finished()
        {
            IceInternal.Buffer buf = PrepareWrite();
            byte[] result = new byte[buf.B.Limit()];
            buf.B.Get(result);
            return result;
        }

        /// <summary>
        /// Writes the start of an encapsulation to the stream.
        /// </summary>
        public void StartEncapsulation() => StartEncapsulation(Encoding);

        /// <summary>
        /// Writes the start of an encapsulation to the stream.
        /// </summary>
        /// <param name="encoding">The encoding version of the encapsulation.</param>
        /// <param name="format">Specify the compact or sliced format; when null, keep the stream's format.</param>
        public void StartEncapsulation(EncodingVersion encoding, FormatType? format = null)
        {
            Debug.Assert(_mainEncaps == null && _endpointEncaps == null);
            Protocol.checkSupportedEncoding(encoding);

            _mainEncaps = new Encaps(Encoding, _format, _buf.B.Position());

            Encoding = encoding;
            if (format.HasValue)
            {
                _format = format.Value;
            }

            WriteEncapsulationHeader(0, Encoding);
        }

        /// <summary>
        /// Ends the previous main encapsulation.
        /// </summary>
        public void EndEncapsulation()
        {
            Debug.Assert(_mainEncaps.HasValue && _endpointEncaps == null);

            Encaps encaps = _mainEncaps.Value;

            // Size includes size and version.
            int sz = _buf.Size() - encaps.StartPos;
            _buf.B.PutInt(encaps.StartPos, sz);

            Encoding = encaps.OldEncoding;
            _format = encaps.OldFormat;
            ResetEncapsulation();
        }

        // Start writing a slice of a class or exception instance.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the type ID of this slice.
        // firstSlice is true when writing the first (most derived) slice of an instance.
        // slicedData is the preserved sliced-off slices, if any. Can only be provided when firstSlice is true.
        // compactId is the compact type ID of this slice, if specified.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId, bool firstSlice, SlicedData? slicedData = null, int? compactId = null)
        {
            Debug.Assert(_mainEncaps != null && _current != null);
            if (slicedData.HasValue)
            {
                Debug.Assert(firstSlice);
                if (WriteSlicedData(slicedData.Value))
                {
                    firstSlice = false;
                } // else we didn't write anything and it's still the first slice
            }

            _current.SliceFlagsPos = Pos;
            _current.SliceFlags = 0;

            if (_format == FormatType.SlicedFormat)
            {
                // Encode the slice size if using the sliced format.
                _current.SliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
            }

            WriteByte(0); // Placeholder for the slice flags

            // For instance slices, encode the flag and the type ID either as a string or index. For exception slices,
            // always encode the type ID a string.
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

            _current.SliceFirstMemberPos = Pos;
        }

        // Marks the end of a slice for a class instance or user exception.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // lastSlice is true when it's the last (least derived) slice of the instance.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice(bool lastSlice)
        {
            Debug.Assert(_mainEncaps != null && _current != null);

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
                int sz = Pos - _current.SliceFirstMemberPos + 4;
                RewriteInt(sz, _current.SliceFirstMemberPos - 4);
            }

            if (_current.IndirectionTable?.Count > 0)
            {
                Debug.Assert(_format == FormatType.SlicedFormat);
                _current.SliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE;

                WriteSize(_current.IndirectionTable.Count);
                foreach (AnyClass v in _current.IndirectionTable)
                {
                    WriteInstance(v);
                }
                _current.IndirectionTable.Clear();
                _current.IndirectionMap?.Clear(); // IndirectionMap is null when writing SlicedData.
            }

            // Update SliceFlags in case they were updated.
            RewriteByte(_current.SliceFlags, _current.SliceFlagsPos);
        }

        /// <summary>
        /// Writes a size to the stream.
        /// </summary>
        /// <param name="v">The size to write.</param>
        public void WriteSize(int v)
        {
            if (v > 254)
            {
                Expand(5);
                _buf.B.Put(255);
                _buf.B.PutInt(v);
            }
            else
            {
                Expand(1);
                _buf.B.Put((byte)v);
            }
        }

        /// <summary>
        /// Returns the current position and allocates four bytes for a fixed-length (32-bit) size value.
        /// </summary>
        public int StartSize()
        {
            int pos = _buf.B.Position();
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
            RewriteInt(_buf.B.Position() - pos - 4, pos);
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
            Expand(v.Length);
            _buf.B.Put(v);
        }

        /// <summary>
        /// Write the header information for an optional value.
        /// </summary>
        /// <param name="tag">The numeric tag associated with the value.</param>
        /// <param name="format">The optional format of the value.</param>
        public bool WriteOptional(int tag, OptionalFormat format)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);

            if (Encoding.Equals(Util.Encoding_1_0))
            {
                // TODO: eliminate this block and return value
                return false; // Tagged members aren't supported with the 1.0 encoding.
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
            if (_current != null)
            {
                _current.SliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
            }
            return true;
        }

        /// <summary>
        /// Writes a byte to the stream.
        /// </summary>
        /// <param name="v">The byte to write to the stream.</param>
        public void WriteByte(byte v)
        {
            Expand(1);
            _buf.B.Put(v);
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
        private void RewriteByte(byte v, int dest) => _buf.B.Put(dest, v);

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
                Expand(v.Length);
                _buf.B.Put(v);
            }
        }

        /// <summary>
        /// Writes a byte sequence to the stream.
        /// </summary>
        /// <param name="v">The byte sequence to write to the stream.</param>
        public void WriteByteSeq(IReadOnlyCollection<byte> v) => WriteSeq(v, (ostr, value) => ostr.WriteByte(value));

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
        /// <param name="v">An enumerator for the byte sequence.</param>
        public void WriteByteSeq(int tag, IReadOnlyCollection<byte> v)
        {
            if (WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteByteSeq(v);
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
                var w = new IceInternal.OutputStreamWrapper(this);
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
            Expand(1);
            _buf.B.Put(v ? (byte)1 : (byte)0);
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
                Expand(v.Length);
                _buf.B.PutBoolSeq(v);
            }
        }

        /// <summary>
        /// Writes a boolean sequence to the stream.
        /// </summary>
        /// <param name="v">The boolean sequence to write to the stream.</param>
        public void WriteBoolSeq(IReadOnlyCollection<bool> v) => WriteSeq(v, (ostr, value) => ostr.WriteBool(value));

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
        /// <param name="v">An enumerator for the optional boolean sequence.</param>
        public void WriteBoolSeq(int tag, IReadOnlyCollection<bool>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteBoolSeq(v);
            }
        }

        /// <summary>
        /// Writes a short to the stream.
        /// </summary>
        /// <param name="v">The short to write to the stream.</param>
        public void WriteShort(short v)
        {
            Expand(2);
            _buf.B.PutShort(v);
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
                Expand(v.Length * 2);
                _buf.B.PutShortSeq(v);
            }
        }

        /// <summary>
        /// Writes a short sequence to the stream.
        /// </summary>
        /// <param name="v">The short sequence to write to the stream.</param>
        public void WriteShortSeq(IReadOnlyCollection<short> v) => WriteSeq(v, (ostr, value) => ostr.WriteShort(value));

        /// <summary>
        /// Writes an optional short sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The short sequence to write to the stream.</param>
        public void WriteShortSeq(int tag, short[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : (v.Length * 2) + (v.Length > 254 ? 5 : 1));
                WriteShortSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional short sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">An enumerator for the short sequence.</param>
        public void WriteShortSeq(int tag, IReadOnlyCollection<short>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = v.Count;
                WriteSize(count == 0 ? 1 : (count * 2) + (count > 254 ? 5 : 1));
                WriteShortSeq(v);
            }
        }

        /// <summary>
        /// Writes an int to the stream.
        /// </summary>
        /// <param name="v">The int to write to the stream.</param>
        public void WriteInt(int v)
        {
            Expand(4);
            _buf.B.PutInt(v);
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
        internal void RewriteInt(int v, int dest) => _buf.B.PutInt(dest, v);

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
                Expand(v.Length * 4);
                _buf.B.PutIntSeq(v);
            }
        }

        /// <summary>
        /// Writes an int sequence to the stream.
        /// </summary>
        /// <param name="v">The int sequence to write to the stream.</param>
        public void WriteIntSeq(IReadOnlyCollection<int> v) => WriteSeq(v, (ostr, value) => ostr.WriteInt(value));

        /// <summary>
        /// Writes an optional int sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The int sequence to write to the stream.</param>
        public void WriteIntSeq(int tag, int[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : (v.Length * 4) + (v.Length > 254 ? 5 : 1));
                WriteIntSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional int sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">An enumerator for the int sequence.</param>
        public void WriteIntSeq(int tag, IReadOnlyCollection<int>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = v.Count;
                WriteSize(count == 0 ? 1 : (count * 4) + (count > 254 ? 5 : 1));
                WriteIntSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a long to the stream.
        /// </summary>
        /// <param name="v">The long to write to the stream.</param>
        public void WriteLong(long v)
        {
            Expand(8);
            _buf.B.PutLong(v);
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
                Expand(v.Length * 8);
                _buf.B.PutLongSeq(v);
            }
        }

        /// <summary>
        /// Writes a long sequence to the stream.
        /// </summary>
        /// <param name="v">The long sequence to write to the stream.</param>
        public void WriteLongSeq(IReadOnlyCollection<long> v) => WriteSeq(v, (ostr, value) => ostr.WriteLong(value));

        /// <summary>
        /// Writes an optional long sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The long sequence to write to the stream.</param>
        public void WriteLongSeq(int tag, long[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : (v.Length * 8) + (v.Length > 254 ? 5 : 1));
                WriteLongSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional long sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">An enumerator for the long sequence.</param>
        public void WriteLongSeq(int tag, IReadOnlyCollection<long>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = v.Count;
                WriteSize(count == 0 ? 1 : (count * 8) + (count > 254 ? 5 : 1));
                WriteLongSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a float to the stream.
        /// </summary>
        /// <param name="v">The float to write to the stream.</param>
        public void WriteFloat(float v)
        {
            Expand(4);
            _buf.B.PutFloat(v);
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
                Expand(v.Length * 4);
                _buf.B.PutFloatSeq(v);
            }
        }

        /// <summary>
        /// Writes a float sequence to the stream.
        /// </summary>
        /// <param name="v">The float sequence to write to the stream.</param>
        public void WriteFloatSeq(IReadOnlyCollection<float> v) => WriteSeq(v, (ostr, value) => ostr.WriteFloat(value));

        /// <summary>
        /// Writes an optional float sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The float sequence to write to the stream.</param>
        public void WriteFloatSeq(int tag, float[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : (v.Length * 4) + (v.Length > 254 ? 5 : 1));
                WriteFloatSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional float sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">An enumerator for the float sequence.</param>
        public void WriteFloatSeq(int tag, IReadOnlyCollection<float>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = v.Count;
                WriteSize(count == 0 ? 1 : (count * 4) + (count > 254 ? 5 : 1));
                WriteFloatSeq(count, v);
            }
        }

        /// <summary>
        /// Writes a double to the stream.
        /// </summary>
        /// <param name="v">The double to write to the stream.</param>
        public void WriteDouble(double v)
        {
            Expand(8);
            _buf.B.PutDouble(v);
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
                Expand(v.Length * 8);
                _buf.B.PutDoubleSeq(v);
            }
        }

        /// <summary>
        /// Writes a double sequence to the stream.
        /// </summary>
        /// <param name="v">The double sequence to write to the stream.</param>
        public void WriteDoubleSeq(IReadOnlyCollection<double> v) =>
            WriteSeq(v, (ostr, value) => ostr.WriteDouble(value));

        /// <summary>
        /// Writes an optional double sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The double sequence to write to the stream.</param>
        public void WriteDoubleSeq(int tag, double[]? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                WriteSize(v.Length == 0 ? 1 : (v.Length * 8) + (v.Length > 254 ? 5 : 1));
                WriteDoubleSeq(v);
            }
        }

        /// <summary>
        /// Writes an optional double sequence to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">An enumerator for the double sequence.</param>
        public void WriteDoubleSeq(int tag, IReadOnlyCollection<double>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.VSize))
            {
                int count = v.Count;
                WriteSize(count == 0 ? 1 : (count * 8) + (count > 254 ? 5 : 1));
                WriteDoubleSeq(count, v);
            }
        }

        private static readonly System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);

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
            byte[] arr = _utf8.GetBytes(v);
            WriteSize(arr.Length);
            Expand(arr.Length);
            _buf.B.Put(arr);
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
        /// <param name="v">The string sequence to write to the stream.</param>
        public void WriteStringSeq(IReadOnlyCollection<string> v) =>
            WriteSeq(v, (ostr, value) => ostr.WriteString(value));

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
        /// <param name="v">An enumerator for the string sequence.</param>
        public void WriteStringSeq(int tag, IReadOnlyCollection<string>? v)
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteStringSeq(v);
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

        public void WriteProxySeq<T>(T[]? v) where T : IObjectPrx
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                foreach (T prx in v)
                {
                    WriteProxy(prx);
                }
            }
        }

        public void WriteProxySeq<T>(IReadOnlyCollection<T>? v) where T : IObjectPrx
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Count);
                foreach (T prx in v)
                {
                    WriteProxy(prx);
                }
            }
        }

        public void WriteProxySeq<T>(int tag, T[]? v) where T : IObjectPrx
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteProxySeq(v);
                EndSize(pos);
            }
        }

        public void WriteProxySeq<T>(int tag, IReadOnlyCollection<T>? v) where T : IObjectPrx
        {
            if (v != null && WriteOptional(tag, OptionalFormat.FSize))
            {
                int pos = StartSize();
                WriteProxySeq(v);
                EndSize(pos);
            }
        }

        public void WriteStructSeq<T>(T[] v) where T : struct, IStreamableStruct
        {
            WriteSize(v.Length);
            for (int i = 0; i < v.Length; i++)
            {
                v[i].IceWrite(this);
            }
        }

        public void WriteStructSeq<T>(IReadOnlyCollection<T> v) where T : struct, IStreamableStruct
        {
            WriteSize(v.Count);
            foreach (T item in v)
            {
                item.IceWrite(this);
            }
        }

        /// <summary>
        /// Writes an enumerated value.
        /// </summary>
        /// <param name="v">The enumerator.</param>
        /// <param name="maxValue">The maximum enumerator value in the definition.</param>
        public void WriteEnum(int v) => WriteSize(v);

        /// <summary>
        /// Writes an optional enumerator to the stream.
        /// </summary>
        /// <param name="tag">The optional tag.</param>
        /// <param name="v">The enumerator.</param>
        public void WriteEnum(int tag, int? v)
        {
            if (v is int value && WriteOptional(tag, OptionalFormat.Size))
            {
                WriteEnum(value);
            }
        }

        public void WriteEnumSeq<T>(T[]? v,  OutputStreamWriter<T> writer) where T : System.Enum
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                foreach (T e in v)
                {
                    writer(this, e);
                }
            }
        }

        public void WriteEnumSeq<T>(IReadOnlyCollection<T>? v, OutputStreamWriter<T> writer) where T : System.Enum
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Count);
                foreach (T e in v)
                {
                    writer(this, e);
                }
            }
        }

        /// <summary>
        /// Writes a class instance to the stream.
        /// </summary>
        /// <param name="v">The value to write.</param>
        public void WriteClass(AnyClass? v)
        {
            Debug.Assert(_mainEncaps != null && _endpointEncaps == null);
            if (v == null)
            {
                WriteSize(0);
            }
            else if (_current != null && _format == FormatType.SlicedFormat)
            {
                // If writing an instance within a slice and using the sliced format, write an index of that slice's
                // indirection table.
                if (_current.IndirectionMap != null && _current.IndirectionMap.TryGetValue(v, out int index))
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
                WriteInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        public void WriteClassSeq<T>(T[]? v) where T : AnyClass
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                foreach (T c in v)
                {
                    WriteClass(c);
                }
            }
        }

        public void WriteClassSeq<T>(IReadOnlyCollection<T>? v) where T : AnyClass
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Count);
                foreach (T c in v)
                {
                    WriteClass(c);
                }
            }
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

        public void WriteSeq<T>(T[]? v, OutputStreamWriter<T> write)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Length);
                foreach (T item in v)
                {
                    write(this, item);
                }
            }
        }

        public void WriteSeq<T>(IReadOnlyCollection<T>? v, OutputStreamWriter<T> elementWriter)
        {
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(v.Count);
                foreach (T item in v)
                {
                    elementWriter(this, item);
                }
            }
        }

        public void WriteDict<TKey, TValue>(IDictionary<TKey, TValue> dict, OutputStreamWriter<TKey> keyWriter,
            OutputStreamWriter<TValue> valueWriter)
        {
            if (dict == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteSize(dict.Count);
                foreach (KeyValuePair<TKey, TValue> pair in dict)
                {
                    keyWriter(this, pair.Key);
                    valueWriter(this, pair.Value);
                }
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

        /// <summary>
        /// Expand the stream to accept more data.
        /// </summary>
        /// <param name="n">The number of bytes to accommodate in the stream.</param>
        internal void Expand(int n) => _buf.Expand(n);

        /// <summary>
        /// Resets this output stream. This method allows the stream to be reused, to avoid creating
        /// unnecessary garbage.
        /// </summary>
        internal void Reset()
        {
            _buf.Reset();
            Clear();
            _format = Communicator.DefaultsAndOverrides.DefaultFormat;
        }

        /// <summary>
        /// Releases any data retained by encapsulations. The reset() method internally calls clear().
        /// </summary>
        internal void Clear() => ResetEncapsulation();

        /// <summary>
        /// Swaps the contents of one stream with another.
        /// </summary>
        /// <param name="other">The other stream.</param>
        internal void Swap(OutputStream other)
        {
            Debug.Assert(Communicator == other.Communicator);

            IceInternal.Buffer tmpBuf = other._buf;
            other._buf = _buf;
            _buf = tmpBuf;

            EncodingVersion tmpEncoding = other.Encoding;
            other.Encoding = Encoding;
            Encoding = tmpEncoding;

            // Swap is never called for streams that have encapsulations being written. However,
            // encapsulations might still be set in case marshalling failed. We just
            // reset the encapsulations if there are still some set.
            ResetEncapsulation();
            other.ResetEncapsulation();
        }

        /// <summary>
        /// Resizes the stream to a new size.
        /// </summary>
        /// <param name="sz">The new size.</param>
        internal void Resize(int sz)
        {
            _buf.Resize(sz, false);
            _buf.B.Position(sz);
        }

        /// <summary>
        /// Prepares the internal data buffer to be written to a socket.
        /// </summary>
        internal IceInternal.Buffer PrepareWrite()
        {
            _buf.B.Limit(_buf.Size());
            _buf.B.Position(0);
            return _buf;
        }

        /// <summary>
        /// Retrieves the internal data buffer.
        /// </summary>
        /// <returns>The buffer.</returns>
        internal IceInternal.Buffer GetBuffer() => _buf;

        internal void StartEndpointEncapsulation() => StartEndpointEncapsulation(Encoding);

        internal void StartEndpointEncapsulation(EncodingVersion encoding)
        {
            Debug.Assert(_endpointEncaps == null);
            Protocol.checkSupportedEncoding(encoding);

            _endpointEncaps = new Encaps(Encoding, _format, _buf.B.Position());
            Encoding = encoding;
            // We didn't change _format, so no need to restore it.

            WriteEncapsulationHeader(0, Encoding);
        }

        internal void EndEndpointEncapsulation()
        {
            Debug.Assert(_endpointEncaps.HasValue);

            // Size includes size and version.
            int startPos = _endpointEncaps.Value.StartPos;
            int sz = _buf.Size() - startPos;
            _buf.B.PutInt(startPos, sz);

            Encoding = _endpointEncaps.Value.OldEncoding;
            // No need to restore format since it didn't change.
            _endpointEncaps = null;
        }

        /// <summary>
        /// Writes an empty encapsulation using the given encoding version.
        /// </summary>
        /// <param name="encoding">The encoding version of the encapsulation.</param>
        internal void WriteEmptyEncapsulation(EncodingVersion encoding)
        {
            Protocol.checkSupportedEncoding(encoding);
            WriteEncapsulationHeader(6, encoding);
        }

        /// <summary>
        /// Writes a pre-encoded encapsulation.
        /// </summary>
        /// <param name="v">The encapsulation data.</param>
        internal void WriteEncapsulation(byte[] v)
        {
            if (v.Length < 6)
            {
                throw new EncapsulationException();
            }
            Expand(v.Length);
            _buf.B.Put(v);
        }

        // Returns true when something was written, and false otherwise
        internal bool WriteSlicedData(SlicedData slicedData)
        {
            Debug.Assert(_current != null);
            // We only remarshal preserved slices if we are using the sliced format. Otherwise, we ignore the preserved
            // slices, which essentially "slices" the instance into the most-derived type known by the sender.
            if (_format != FormatType.SlicedFormat || Encoding != slicedData.Encoding)
            {
                // TODO: if the encodings don't match, do we just drop these slices, or throw an exception?
                return false;
            }

            bool firstSlice = true;
            foreach (SliceInfo info in slicedData.Slices)
            {
                IceStartSlice(info.TypeId ?? "", firstSlice, null, info.CompactId);
                firstSlice = false;

                // Write the bytes associated with this slice. TODO: need better write bytes API
                byte[] sliceBytes = new byte[info.Bytes.Count];
                info.Bytes.CopyTo(sliceBytes, 0);
                WriteBlob(sliceBytes);

                if (info.HasOptionalMembers)
                {
                    _current.SliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                // Make sure to also re-write the instance indirection table.
                // These instances will be marshaled (and assigned instance IDs) in IceEndSlice.
                if (info.Instances.Count > 0)
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    Debug.Assert(_current.IndirectionTable.Count == 0);
                    _current.IndirectionTable.AddRange(info.Instances);
                }
                IceEndSlice(info.IsLastSlice); // TODO: can we check it's indeed the last slice?
            }
            return firstSlice == false; // we wrote at least one slice
        }

        // Helper constructor used by the other constructors.
        private OutputStream(Ice.Communicator communicator, EncodingVersion encoding, IceInternal.Buffer buf)
        {
            Communicator = communicator;
            Encoding = encoding;
            _buf = buf;
        }

        private void WriteEncapsulationHeader(int size, EncodingVersion encoding)
        {
            WriteInt(size);
            WriteByte(encoding.major);
            WriteByte(encoding.minor);
        }

        private void ResetEncapsulation()
        {
            _mainEncaps = null;
            _endpointEncaps = null;
            _current = null;
            _instanceMap?.Clear();
            _typeIdMap?.Clear();
        }

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

        // Write this class instance inline if not previously marshaled, otherwise just write its instance ID.
        private void WriteInstance(AnyClass v)
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

                InstanceData? savedInstanceData = Push(InstanceType.Class);
                v.Write(this);
                Pop(savedInstanceData);
            }
        }

        private InstanceData? Push(InstanceType instanceType)
        {
            InstanceData? savedInstanceData = _current;
            _current = new InstanceData(instanceType);
            return savedInstanceData;
        }

        private void Pop(InstanceData? savedInstanceData) => _current = savedInstanceData;

        private enum InstanceType { Class, Exception }

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

            // Previous format (Compact or Sliced).
            internal readonly FormatType OldFormat;

            internal readonly int StartPos;

            internal Encaps(EncodingVersion oldEncoding, FormatType oldFormat, int startPos)
            {
                OldEncoding = oldEncoding;
                OldFormat = oldFormat;
                StartPos = startPos;
            }
        }
    }
}
