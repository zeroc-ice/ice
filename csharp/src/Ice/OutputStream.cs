// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

using Protocol = Ice.Internal.Protocol;

namespace Ice;

/// <summary>
/// Interface for output streams used to write Slice types to a sequence
/// of bytes.
/// </summary>
public sealed class OutputStream
{
    /// <summary>
    /// Initializes a new instance of the <see cref="OutputStream" /> class. The output stream is initially empty.
    /// </summary>
    /// <param name="encoding">The encoding version. null is equivalent to encoding 1.1.</param>
    /// <param name="format">The class format.</param>
    public OutputStream(EncodingVersion? encoding = null, FormatType format = FormatType.CompactFormat)
    {
        _buf = new Internal.Buffer();
        _encoding = encoding ?? Util.currentEncoding;
        _format = format;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="OutputStream" /> class. The output stream is initially empty,
    /// and uses the communicator's default encoding version and default class format.
    /// </summary>
    /// <param name="communicator">The communicator that provides the encoding version and class format.</param>
    public OutputStream(Communicator communicator)
        : this(
            communicator.instance.defaultsAndOverrides().defaultEncoding,
            communicator.instance.defaultsAndOverrides().defaultFormat)
    {
    }

    internal OutputStream(
        Internal.Buffer buf,
        EncodingVersion? encoding = null,
        FormatType format = FormatType.CompactFormat)
        : this(encoding, format) => _buf = buf;

    /// <summary>
    /// Resets this output stream. This method allows the stream to be reused, to avoid creating
    /// unnecessary garbage.
    /// </summary>
    public void reset()
    {
        _buf.reset();
        clear();
    }

    /// <summary>
    /// Releases any data retained by encapsulations. The reset() method internally calls clear().
    /// </summary>
    public void clear()
    {
        if (_encapsStack != null)
        {
            Debug.Assert(_encapsStack.next == null);
            _encapsStack.next = _encapsCache;
            _encapsCache = _encapsStack;
            _encapsStack = null;
            _encapsCache.reset();
        }
    }

    /// <summary>
    /// Indicates that the marshaling of a request or reply is finished.
    /// </summary>
    /// <returns>The byte sequence containing the encoded request or reply.</returns>
    public byte[] finished()
    {
        Ice.Internal.Buffer buf = prepareWrite();
        byte[] result = new byte[buf.b.limit()];
        buf.b.get(result);
        return result;
    }

    /// <summary>
    /// Swaps the contents of one stream with another.
    /// </summary>
    /// <param name="other">The other stream.</param>
    public void swap(OutputStream other)
    {
        Ice.Internal.Buffer tmpBuf = other._buf;
        other._buf = _buf;
        _buf = tmpBuf;

        EncodingVersion tmpEncoding = other._encoding;
        other._encoding = _encoding;
        _encoding = tmpEncoding;

        //
        // Swap is never called for streams that have encapsulations being written. However,
        // encapsulations might still be set in case marshaling failed. We just
        // reset the encapsulations if there are still some set.
        //
        resetEncapsulation();
        other.resetEncapsulation();
    }

    private void resetEncapsulation() => _encapsStack = null;

    /// <summary>
    /// Resizes the stream to a new size.
    /// </summary>
    /// <param name="sz">The new size.</param>
    public void resize(int sz)
    {
        _buf.resize(sz, false);
        _buf.b.position(sz);
    }

    /// <summary>
    /// Marks the start of a class instance.
    /// </summary>
    /// <param name="data">Preserved slices for this instance, or null.</param>
    public void startValue(SlicedData? data)
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ValueSlice, data);
    }

    /// <summary>
    /// Marks the end of a class instance.
    /// </summary>
    public void endValue()
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /// <summary>
    /// Marks the start of a user exception.
    /// </summary>
    public void startException()
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startInstance(SliceType.ExceptionSlice, null);
    }

    /// <summary>
    /// Marks the end of a user exception.
    /// </summary>
    public void endException()
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endInstance();
    }

    /// <summary>
    /// Writes the start of an encapsulation to the stream.
    /// </summary>
    public void startEncapsulation()
    {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //

        if (_encapsStack != null)
        {
            startEncapsulation(_encapsStack.encoding, _encapsStack.format);
        }
        else
        {
            startEncapsulation(_encoding, format: null);
        }
    }

    /// <summary>
    /// Writes the start of an encapsulation to the stream.
    /// </summary>
    /// <param name="encoding">The encoding version of the encapsulation.</param>
    /// <param name="format">Specify the compact or sliced format. When null, use the OutputStream's class format.
    /// </param>
    public void startEncapsulation(EncodingVersion encoding, FormatType? format = null)
    {
        Protocol.checkSupportedEncoding(encoding);

        Encaps? curr = _encapsCache;
        if (curr != null)
        {
            curr.reset();
            _encapsCache = _encapsCache!.next;
        }
        else
        {
            curr = new Encaps();
        }
        curr.next = _encapsStack;
        _encapsStack = curr;

        _encapsStack.format = format ?? _format;
        _encapsStack.setEncoding(encoding);
        _encapsStack.start = _buf.b.position();

        writeInt(0); // Placeholder for the encapsulation length.
        EncodingVersion.ice_write(this, _encapsStack.encoding);
    }

    /// <summary>
    /// Ends the previous encapsulation.
    /// </summary>
    public void endEncapsulation()
    {
        Debug.Assert(_encapsStack != null);

        // Size includes size and version.
        int start = _encapsStack.start;
        int sz = _buf.size() - start;
        _buf.b.putInt(start, sz);

        Encaps curr = _encapsStack;
        _encapsStack = curr.next;
        curr.next = _encapsCache;
        _encapsCache = curr;
        _encapsCache.reset();
    }

    /// <summary>
    /// Writes an empty encapsulation using the given encoding version.
    /// </summary>
    /// <param name="encoding">The encoding version of the encapsulation.</param>
    public void writeEmptyEncapsulation(EncodingVersion encoding)
    {
        Protocol.checkSupportedEncoding(encoding);
        writeInt(6); // Size
        EncodingVersion.ice_write(this, encoding);
    }

    /// <summary>
    /// Writes a pre-encoded encapsulation.
    /// </summary>
    /// <param name="v">The encapsulation data.</param>
    public void writeEncapsulation(byte[] v)
    {
        if (v.Length < 6)
        {
            throw new MarshalException($"A byte sequence with {v.Length} bytes is not a valid encapsulation.");
        }
        expand(v.Length);
        _buf.b.put(v);
    }

    /// <summary>
    /// Determines the current encoding version.
    /// </summary>
    /// <returns>The encoding version.</returns>
    public EncodingVersion getEncoding() => _encapsStack != null ? _encapsStack.encoding : _encoding;

    /// <summary>
    /// Marks the start of a new slice for a class instance or user exception.
    /// </summary>
    /// <param name="typeId">The Slice type ID corresponding to this slice.</param>
    /// <param name="compactId">The Slice compact type ID corresponding to this slice or -1 if no compact ID
    /// is defined for the type ID.</param>
    /// <param name="last">True if this is the last slice, false otherwise.</param>
    public void startSlice(string typeId, int compactId, bool last)
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.startSlice(typeId, compactId, last);
    }

    /// <summary>
    /// Marks the end of a slice for a class instance or user exception.
    /// </summary>
    public void endSlice()
    {
        Debug.Assert(_encapsStack != null && _encapsStack.encoder != null);
        _encapsStack.encoder.endSlice();
    }

    /// <summary>
    /// Writes the state of Slice classes whose index was previously written with writeValue() to the stream.
    /// </summary>
    public void writePendingValues()
    {
        if (_encapsStack != null && _encapsStack.encoder != null)
        {
            _encapsStack.encoder.writePendingValues();
        }
        else if (_encapsStack != null ?
                _encapsStack.encoding_1_0 : _encoding.Equals(Util.Encoding_1_0))
        {
            //
            // If using the 1.0 encoding and no instances were written, we
            // still write an empty sequence for pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            writeSize(0);
        }
    }

    /// <summary>
    /// Writes a size to the stream.
    /// </summary>
    /// <param name="v">The size to write.</param>
    public void writeSize(int v)
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
    /// <returns>The position at which to store the size value.</returns>
    public int startSize()
    {
        int pos = _buf.b.position();
        writeInt(0); // Placeholder for 32-bit size
        return pos;
    }

    /// <summary>
    /// Computes the amount of data written since the previous call to startSize and writes that value
    /// at the saved position.
    /// </summary>
    /// <param name="pos">The saved position.</param>
    public void endSize(int pos)
    {
        Debug.Assert(pos >= 0);
        rewriteInt(_buf.b.position() - pos - 4, pos);
    }

    /// <summary>
    /// Writes a blob of bytes to the stream.
    /// </summary>
    /// <param name="v">The byte array to be written. All of the bytes in the array are written.</param>
    public void writeBlob(byte[] v)
    {
        if (v == null)
        {
            return;
        }
        expand(v.Length);
        _buf.b.put(v);
    }

    /// <summary>
    /// Writes a blob of bytes to the stream.
    /// </summary>
    /// <param name="v">The byte array to be written. All of the bytes in the array are written.</param>
    /// <param name="off">The offset into the byte array from which to copy.</param>
    /// <param name="len">The number of bytes from the byte array to copy.</param>
    public void writeBlob(byte[] v, int off, int len)
    {
        if (v == null)
        {
            return;
        }
        expand(len);
        _buf.b.put(v, off, len);
    }

    /// <summary>
    /// Write the header information for an optional value.
    /// </summary>
    /// <param name="tag">The numeric tag associated with the value.</param>
    /// <param name="format">The optional format of the value.</param>
    /// <returns><c>true</c> if the encoding supports optional values and the header information was written to the stream;
    /// <c>false</c> otherwise.</returns>
    public bool writeOptional(int tag, OptionalFormat format)
    {
        Debug.Assert(_encapsStack != null);
        if (_encapsStack.encoder != null)
        {
            return _encapsStack.encoder.writeOptional(tag, format);
        }
        else
        {
            return writeOptionalImpl(tag, format);
        }
    }

    /// <summary>
    /// Writes a byte to the stream.
    /// </summary>
    /// <param name="v">The byte to write to the stream.</param>
    public void writeByte(byte v)
    {
        expand(1);
        _buf.b.put(v);
    }

    /// <summary>
    /// Writes an optional byte to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional byte to write to the stream.</param>
    public void writeByte(int tag, byte? v)
    {
        if (v.HasValue)
        {
            writeByte(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional byte to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The byte to write to the stream.</param>
    public void writeByte(int tag, byte v)
    {
        if (writeOptional(tag, OptionalFormat.F1))
        {
            writeByte(v);
        }
    }

    /// <summary>
    /// Writes a byte to the stream at the given position. The current position of the stream is not modified.
    /// </summary>
    /// <param name="v">The byte to write to the stream.</param>
    /// <param name="dest">The position at which to store the byte in the buffer.</param>
    public void rewriteByte(byte v, int dest) => _buf.b.put(dest, v);

    /// <summary>
    /// Writes a byte sequence to the stream.
    /// </summary>
    /// <param name="v">The byte sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeByteSeq(byte[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length);
            _buf.b.put(v);
        }
    }

    /// <summary>
    /// Writes a byte sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeByteSeq(int count, IEnumerable<byte> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<byte> value)
            {
                writeByteSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<byte>)
            {
                writeSize(count);
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
            if (v is Queue<byte> value)
            {
                writeByteSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<byte> value)
            {
                writeByteSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeByteSeq(int tag, byte[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeByteSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional byte sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the byte sequence.</param>
    public void writeByteSeq(int tag, int count, IEnumerable<byte>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeByteSeq(count, v);
        }
    }

    /// <summary>
    /// Writes a boolean to the stream.
    /// </summary>
    /// <param name="v">The boolean to write to the stream.</param>
    public void writeBool(bool v)
    {
        expand(1);
        _buf.b.put(v ? (byte)1 : (byte)0);
    }

    /// <summary>
    /// Writes an optional boolean to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional boolean to write to the stream.</param>
    public void writeBool(int tag, bool? v)
    {
        if (v.HasValue)
        {
            writeBool(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional boolean to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The boolean to write to the stream.</param>
    public void writeBool(int tag, bool v)
    {
        if (writeOptional(tag, OptionalFormat.F1))
        {
            writeBool(v);
        }
    }

    /// <summary>
    /// Writes a boolean to the stream at the given position. The current position of the stream is not modified.
    /// </summary>
    /// <param name="v">The boolean to write to the stream.</param>
    /// <param name="dest">The position at which to store the boolean in the buffer.</param>
    public void rewriteBool(bool v, int dest) => _buf.b.put(dest, v ? (byte)1 : (byte)0);

    /// <summary>
    /// Writes a boolean sequence to the stream.
    /// </summary>
    /// <param name="v">The boolean sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeBoolSeq(bool[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length);
            _buf.b.putBoolSeq(v);
        }
    }

    /// <summary>
    /// Writes a boolean sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeBoolSeq(int count, IEnumerable<bool> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<bool> value)
            {
                writeBoolSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<bool>)
            {
                writeSize(count);
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
            if (v is Queue<bool> value)
            {
                writeBoolSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<bool> value)
            {
                writeBoolSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    /// <param name="v">The boolean sequence to write to the stream.</param>
    public void writeBoolSeq(int tag, bool[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeBoolSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional boolean sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the boolean sequence.</param>
    public void writeBoolSeq(int tag, int count, IEnumerable<bool>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeBoolSeq(count, v);
        }
    }

    /// <summary>
    /// Writes a short to the stream.
    /// </summary>
    /// <param name="v">The short to write to the stream.</param>
    public void writeShort(short v)
    {
        expand(2);
        _buf.b.putShort(v);
    }

    /// <summary>
    /// Writes an optional short to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional short to write to the stream.</param>
    public void writeShort(int tag, short? v)
    {
        if (v.HasValue)
        {
            writeShort(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional short to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The short to write to the stream.</param>
    public void writeShort(int tag, short v)
    {
        if (writeOptional(tag, OptionalFormat.F2))
        {
            writeShort(v);
        }
    }

    /// <summary>
    /// Writes a short sequence to the stream.
    /// </summary>
    /// <param name="v">The short sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeShortSeq(short[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length * 2);
            _buf.b.putShortSeq(v);
        }
    }

    /// <summary>
    /// Writes a short sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeShortSeq(int count, IEnumerable<short> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<short> value)
            {
                writeShortSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<short>)
            {
                writeSize(count);
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
            if (v is Queue<short> value)
            {
                writeShortSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<short> value)
            {
                writeShortSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeShortSeq(int tag, short[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v.Length == 0 ? 1 : (v.Length * 2) + (v.Length > 254 ? 5 : 1));
            writeShortSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional short sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the short sequence.</param>
    public void writeShortSeq(int tag, int count, IEnumerable<short>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(count == 0 ? 1 : (count * 2) + (count > 254 ? 5 : 1));
            writeShortSeq(count, v);
        }
    }

    /// <summary>
    /// Writes an int to the stream.
    /// </summary>
    /// <param name="v">The int to write to the stream.</param>
    public void writeInt(int v)
    {
        expand(4);
        _buf.b.putInt(v);
    }

    /// <summary>
    /// Writes an optional int to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional int to write to the stream.</param>
    public void writeInt(int tag, int? v)
    {
        if (v.HasValue)
        {
            writeInt(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional int to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The int to write to the stream.</param>
    public void writeInt(int tag, int v)
    {
        if (writeOptional(tag, OptionalFormat.F4))
        {
            writeInt(v);
        }
    }

    /// <summary>
    /// Writes an int to the stream at the given position. The current position of the stream is not modified.
    /// </summary>
    /// <param name="v">The int to write to the stream.</param>
    /// <param name="dest">The position at which to store the int in the buffer.</param>
    public void rewriteInt(int v, int dest) => _buf.b.putInt(dest, v);

    /// <summary>
    /// Writes an int sequence to the stream.
    /// </summary>
    /// <param name="v">The int sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeIntSeq(int[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length * 4);
            _buf.b.putIntSeq(v);
        }
    }

    /// <summary>
    /// Writes an int sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeIntSeq(int count, IEnumerable<int> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<int> value)
            {
                writeIntSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<int>)
            {
                writeSize(count);
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
            if (v is Queue<int> value)
            {
                writeIntSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<int> value)
            {
                writeIntSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeIntSeq(int tag, int[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v.Length == 0 ? 1 : (v.Length * 4) + (v.Length > 254 ? 5 : 1));
            writeIntSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional int sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the int sequence.</param>
    public void writeIntSeq(int tag, int count, IEnumerable<int>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(count == 0 ? 1 : (count * 4) + (count > 254 ? 5 : 1));
            writeIntSeq(count, v);
        }
    }

    /// <summary>
    /// Writes a long to the stream.
    /// </summary>
    /// <param name="v">The long to write to the stream.</param>
    public void writeLong(long v)
    {
        expand(8);
        _buf.b.putLong(v);
    }

    /// <summary>
    /// Writes an optional long to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional long to write to the stream.</param>
    public void writeLong(int tag, long? v)
    {
        if (v.HasValue)
        {
            writeLong(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional long to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The long to write to the stream.</param>
    public void writeLong(int tag, long v)
    {
        if (writeOptional(tag, OptionalFormat.F8))
        {
            writeLong(v);
        }
    }

    /// <summary>
    /// Writes a long sequence to the stream.
    /// </summary>
    /// <param name="v">The long sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeLongSeq(long[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length * 8);
            _buf.b.putLongSeq(v);
        }
    }

    /// <summary>
    /// Writes a long sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeLongSeq(int count, IEnumerable<long> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<long> value)
            {
                writeLongSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<long>)
            {
                writeSize(count);
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
            if (v is Queue<long> value)
            {
                writeLongSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<long> value)
            {
                writeLongSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeLongSeq(int tag, long[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v.Length == 0 ? 1 : (v.Length * 8) + (v.Length > 254 ? 5 : 1));
            writeLongSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional long sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the long sequence.</param>
    public void writeLongSeq(int tag, int count, IEnumerable<long>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(count == 0 ? 1 : (count * 8) + (count > 254 ? 5 : 1));
            writeLongSeq(count, v);
        }
    }

    /// <summary>
    /// Writes a float to the stream.
    /// </summary>
    /// <param name="v">The float to write to the stream.</param>
    public void writeFloat(float v)
    {
        expand(4);
        _buf.b.putFloat(v);
    }

    /// <summary>
    /// Writes an optional float to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional float to write to the stream.</param>
    public void writeFloat(int tag, float? v)
    {
        if (v.HasValue)
        {
            writeFloat(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional float to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The float to write to the stream.</param>
    public void writeFloat(int tag, float v)
    {
        if (writeOptional(tag, OptionalFormat.F4))
        {
            writeFloat(v);
        }
    }

    /// <summary>
    /// Writes a float sequence to the stream.
    /// </summary>
    /// <param name="v">The float sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeFloatSeq(float[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length * 4);
            _buf.b.putFloatSeq(v);
        }
    }

    /// <summary>
    /// Writes a float sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeFloatSeq(int count, IEnumerable<float> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<float> value)
            {
                writeFloatSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<float>)
            {
                writeSize(count);
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
            if (v is Queue<float> value)
            {
                writeFloatSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<float> value)
            {
                writeFloatSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeFloatSeq(int tag, float[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v.Length == 0 ? 1 : (v.Length * 4) + (v.Length > 254 ? 5 : 1));
            writeFloatSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional float sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the float sequence.</param>
    public void writeFloatSeq(int tag, int count, IEnumerable<float>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(count == 0 ? 1 : (count * 4) + (count > 254 ? 5 : 1));
            writeFloatSeq(count, v);
        }
    }

    /// <summary>
    /// Writes a double to the stream.
    /// </summary>
    /// <param name="v">The double to write to the stream.</param>
    public void writeDouble(double v)
    {
        expand(8);
        _buf.b.putDouble(v);
    }

    /// <summary>
    /// Writes an optional double to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The optional double to write to the stream.</param>
    public void writeDouble(int tag, double? v)
    {
        if (v.HasValue)
        {
            writeDouble(tag, v.Value);
        }
    }

    /// <summary>
    /// Writes an optional double to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The double to write to the stream.</param>
    public void writeDouble(int tag, double v)
    {
        if (writeOptional(tag, OptionalFormat.F8))
        {
            writeDouble(v);
        }
    }

    /// <summary>
    /// Writes a double sequence to the stream.
    /// </summary>
    /// <param name="v">The double sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeDoubleSeq(double[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            expand(v.Length * 8);
            _buf.b.putDoubleSeq(v);
        }
    }

    /// <summary>
    /// Writes a double sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeDoubleSeq(int count, IEnumerable<double> v)
    {
        if (count == 0)
        {
            writeSize(0);
            return;
        }

        {
            if (v is List<double> value)
            {
                writeDoubleSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is LinkedList<double>)
            {
                writeSize(count);
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
            if (v is Queue<double> value)
            {
                writeDoubleSeq(value.ToArray());
                return;
            }
        }

        {
            if (v is Stack<double> value)
            {
                writeDoubleSeq(value.ToArray());
                return;
            }
        }

        writeSize(count);
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
    public void writeDoubleSeq(int tag, double[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(v.Length == 0 ? 1 : (v.Length * 8) + (v.Length > 254 ? 5 : 1));
            writeDoubleSeq(v);
        }
    }

    /// <summary>
    /// Writes an optional double sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the double sequence.</param>
    public void writeDoubleSeq(int tag, int count, IEnumerable<double>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeSize(count == 0 ? 1 : (count * 8) + (count > 254 ? 5 : 1));
            writeDoubleSeq(count, v);
        }
    }

    private static readonly System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);

    /// <summary>
    /// Writes a string to the stream.
    /// </summary>
    /// <param name="v">The string to write to the stream. Passing null causes
    /// an empty string to be written to the stream.</param>
    public void writeString(string v)
    {
        if (v == null || v.Length == 0)
        {
            writeSize(0);
            return;
        }
        byte[] arr = utf8.GetBytes(v);
        writeSize(arr.Length);
        expand(arr.Length);
        _buf.b.put(arr);
    }

    /// <summary>
    /// Writes an optional string to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The string to write to the stream.</param>
    public void writeString(int tag, string? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.VSize))
        {
            writeString(v);
        }
    }

    /// <summary>
    /// Writes a string sequence to the stream.
    /// </summary>
    /// <param name="v">The string sequence to write to the stream.
    /// Passing null causes an empty sequence to be written to the stream.</param>
    public void writeStringSeq(string[] v)
    {
        if (v == null)
        {
            writeSize(0);
        }
        else
        {
            writeSize(v.Length);
            for (int i = 0; i < v.Length; i++)
            {
                writeString(v[i]);
            }
        }
    }

    /// <summary>
    /// Writes a string sequence to the stream.
    /// </summary>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the container holding the sequence.</param>
    public void writeStringSeq(int count, IEnumerable<string> v)
    {
        writeSize(count);
        if (count != 0)
        {
            foreach (string s in v)
            {
                writeString(s);
            }
        }
    }

    /// <summary>
    /// Writes an optional string sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The string sequence to write to the stream.</param>
    public void writeStringSeq(int tag, string[]? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.FSize))
        {
            int pos = startSize();
            writeStringSeq(v);
            endSize(pos);
        }
    }

    /// <summary>
    /// Writes an optional string sequence to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="count">The number of elements in the sequence.</param>
    /// <param name="v">An enumerator for the string sequence.</param>
    public void writeStringSeq(int tag, int count, IEnumerable<string>? v)
    {
        if (v is not null && writeOptional(tag, OptionalFormat.FSize))
        {
            int pos = startSize();
            writeStringSeq(count, v);
            endSize(pos);
        }
    }

    /// <summary>
    /// Writes a proxy to the stream.
    /// </summary>
    /// <param name="v">The proxy to write.</param>
    public void writeProxy(ObjectPrx? v)
    {
        if (v is not null)
        {
            v.iceWrite(this);
        }
        else
        {
            Identity.ice_write(this, new Identity());
        }
    }

    /// <summary>
    /// Writes an optional proxy to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The proxy to write.</param>
    public void writeProxy(int tag, ObjectPrx? v)
    {
        // We don't write anything for an optional proxy with a null value.
        if (v is not null && writeOptional(tag, OptionalFormat.FSize))
        {
            int pos = startSize();
            writeProxy(v);
            endSize(pos);
        }
    }

    /// <summary>
    /// Writes an enumerated value.
    /// </summary>
    /// <param name="v">The enumerator.</param>
    /// <param name="maxValue">The maximum enumerator value in the definition.</param>
    public void writeEnum(int v, int maxValue)
    {
        if (isEncoding_1_0())
        {
            if (maxValue < 127)
            {
                writeByte((byte)v);
            }
            else if (maxValue < 32767)
            {
                writeShort((short)v);
            }
            else
            {
                writeInt(v);
            }
        }
        else
        {
            writeSize(v);
        }
    }

    /// <summary>
    /// Writes an optional enumerator to the stream.
    /// </summary>
    /// <param name="tag">The optional tag.</param>
    /// <param name="v">The enumerator.</param>
    /// <param name="maxValue">The maximum enumerator value in the definition.</param>
    public void writeEnum(int tag, int v, int maxValue)
    {
        if (writeOptional(tag, OptionalFormat.Size))
        {
            writeEnum(v, maxValue);
        }
    }

    /// <summary>
    /// Writes a class instance to the stream.
    /// </summary>
    /// <param name="v">The value to write. This method writes the index of an instance; the state of the value is
    /// written once writePendingValues() is called.</param>
    public void writeValue(Value? v)
    {
        initEncaps();
        _encapsStack!.encoder!.writeValue(v);
    }

    /// <summary>
    /// Writes a user exception to the stream.
    /// </summary>
    /// <param name="v">The user exception to write.</param>
    public void writeException(UserException v)
    {
        initEncaps();
        // Exceptions are always encoded with the sliced format.
        _encapsStack!.format = FormatType.SlicedFormat;
        _encapsStack!.encoder!.writeException(v);
    }

    private bool writeOptionalImpl(int tag, OptionalFormat format)
    {
        if (isEncoding_1_0())
        {
            return false; // Optional members aren't supported with the 1.0 encoding.
        }

        int v = (int)format;
        if (tag < 30)
        {
            v |= tag << 3;
            writeByte((byte)v);
        }
        else
        {
            v |= 0x0F0; // tag = 30
            writeByte((byte)v);
            writeSize(tag);
        }
        return true;
    }

    /// <summary>
    /// Determines the current position in the stream.
    /// </summary>
    /// <returns>The current position.</returns>
    public int pos() => _buf.b.position();

    /// <summary>
    /// Sets the current position in the stream.
    /// </summary>
    /// <param name="n">The new position.</param>
    public void pos(int n) => _buf.b.position(n);

    /// <summary>
    /// Determines the current size of the stream.
    /// </summary>
    /// <returns>The current size.</returns>
    public int size() => _buf.size();

    /// <summary>
    /// Determines whether the stream is empty.
    /// </summary>
    /// <returns>True if no data has been written yet, false otherwise.</returns>
    public bool isEmpty() => _buf.empty();

    /// <summary>
    /// Expand the stream to accept more data.
    /// </summary>
    /// <param name="n">The number of bytes to accommodate in the stream.</param>
    public void expand(int n) => _buf.expand(n);

    /// <summary>
    /// Prepares the internal data buffer to be written to a socket.
    /// </summary>
    internal Ice.Internal.Buffer prepareWrite()
    {
        _buf.b.limit(_buf.size());
        _buf.b.position(0);
        return _buf;
    }

    /// <summary>
    /// Retrieves the internal data buffer.
    /// </summary>
    /// <returns>The buffer.</returns>
    internal Ice.Internal.Buffer getBuffer() => _buf;

    private Ice.Internal.Buffer _buf;

    private readonly FormatType _format;

    private enum SliceType
    {
        NoSlice,
        ValueSlice,
        ExceptionSlice
    }

    private abstract class EncapsEncoder
    {
        protected EncapsEncoder(OutputStream stream, Encaps encaps)
        {
            _stream = stream;
            _encaps = encaps;
            _typeIdIndex = 0;
            _marshaledMap = new Dictionary<Value, int>();
        }

        internal abstract void writeValue(Value? v);

        internal abstract void writeException(UserException v);

        internal abstract void startInstance(SliceType type, SlicedData? data);

        internal abstract void endInstance();

        internal abstract void startSlice(string typeId, int compactId, bool last);

        internal abstract void endSlice();

        internal virtual bool writeOptional(int tag, OptionalFormat format) => false;

        internal virtual void writePendingValues()
        {
        }

        protected int registerTypeId(string typeId)
        {
            _typeIdMap ??= new Dictionary<string, int>();

            if (_typeIdMap.TryGetValue(typeId, out int p))
            {
                return p;
            }
            else
            {
                _typeIdMap.Add(typeId, ++_typeIdIndex);
                return -1;
            }
        }

        protected readonly OutputStream _stream;

        protected readonly Encaps _encaps;

        // Encapsulation attributes for instance marshaling.
        protected readonly Dictionary<Value, int> _marshaledMap;

        // Encapsulation attributes for instance marshaling.
        private Dictionary<string, int>? _typeIdMap;

        private int _typeIdIndex;
    }

    private sealed class EncapsEncoder10 : EncapsEncoder
    {
        internal EncapsEncoder10(OutputStream stream, Encaps encaps)
            : base(stream, encaps)
        {
            _sliceType = SliceType.NoSlice;
            _valueIdIndex = 0;
            _toBeMarshaledMap = new Dictionary<Value, int>();
        }

        internal override void writeValue(Value? v)
        {
            //
            // Object references are encoded as a negative integer in 1.0.
            //
            if (v != null)
            {
                _stream.writeInt(-registerValue(v));
            }
            else
            {
                _stream.writeInt(0);
            }
        }

        internal override void writeException(UserException v)
        {
            //
            // User exception with the 1.0 encoding start with a bool
            // flag that indicates whether or not the exception uses
            // classes.
            //
            // This allows reading the pending instances even if some part of
            // the exception was sliced.
            //
            bool usesClasses = v.iceUsesClasses();
            _stream.writeBool(usesClasses);
            v.iceWrite(_stream);
            if (usesClasses)
            {
                writePendingValues();
            }
        }

        internal override void startInstance(SliceType sliceType, SlicedData? sliceData) => _sliceType = sliceType;

        internal override void endInstance()
        {
            if (_sliceType == SliceType.ValueSlice)
            {
                //
                // Write the Object slice.
                //
                startSlice(Value.ice_staticId(), -1, true);
                _stream.writeSize(0); // For compatibility with the old AFM.
                endSlice();
            }
            _sliceType = SliceType.NoSlice;
        }

        internal override void startSlice(string typeId, int compactId, bool last)
        {
            //
            // For instance slices, encode a bool to indicate how the type ID
            // is encoded and the type ID either as a string or index. For
            // exception slices, always encode the type ID as a string.
            //
            if (_sliceType == SliceType.ValueSlice)
            {
                int index = registerTypeId(typeId);
                if (index < 0)
                {
                    _stream.writeBool(false);
                    _stream.writeString(typeId);
                }
                else
                {
                    _stream.writeBool(true);
                    _stream.writeSize(index);
                }
            }
            else
            {
                _stream.writeString(typeId);
            }

            _stream.writeInt(0); // Placeholder for the slice length.

            _writeSlice = _stream.pos();
        }

        internal override void endSlice()
        {
            //
            // Write the slice length.
            //
            int sz = _stream.pos() - _writeSlice + 4;
            _stream.rewriteInt(sz, _writeSlice - 4);
        }

        internal override void writePendingValues()
        {
            while (_toBeMarshaledMap.Count > 0)
            {
                //
                // Consider the to be marshaled instances as marshaled now,
                // this is necessary to avoid adding again the "to be
                // marshaled instances" into _toBeMarshaledMap while writing
                // instances.
                //
                foreach (KeyValuePair<Value, int> e in _toBeMarshaledMap)
                {
                    _marshaledMap.Add(e.Key, e.Value);
                }

                Dictionary<Value, int> savedMap = _toBeMarshaledMap;
                _toBeMarshaledMap = new Dictionary<Value, int>();
                _stream.writeSize(savedMap.Count);
                foreach (KeyValuePair<Value, int> p in savedMap)
                {
                    //
                    // Ask the instance to marshal itself. Any new class
                    // instances that are triggered by the classes marshaled
                    // are added to toBeMarshaledMap.
                    //
                    _stream.writeInt(p.Value);

                    p.Key.ice_preMarshal();
                    p.Key.iceWrite(_stream);
                }
            }
            _stream.writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
        }

        private int registerValue(Value v)
        {
            Debug.Assert(v != null);

            //
            // Look for this instance in the to-be-marshaled map.
            //
            if (_toBeMarshaledMap.TryGetValue(v, out int p))
            {
                return p;
            }

            //
            // Didn't find it, try the marshaled map next.
            //
            if (_marshaledMap.TryGetValue(v, out p))
            {
                return p;
            }

            //
            // We haven't seen this instance previously, create a new
            // index, and insert it into the to-be-marshaled map.
            //
            _toBeMarshaledMap.Add(v, ++_valueIdIndex);
            return _valueIdIndex;
        }

        // Instance attributes
        private SliceType _sliceType;

        // Slice attributes
        private int _writeSlice;        // Position of the slice data members

        // Encapsulation attributes for instance marshaling.
        private int _valueIdIndex;
        private Dictionary<Value, int> _toBeMarshaledMap;
    }

    private sealed class EncapsEncoder11 : EncapsEncoder
    {
        internal EncapsEncoder11(OutputStream stream, Encaps encaps)
            : base(stream, encaps)
        {
            _current = null;
            _valueIdIndex = 1;
        }

        internal override void writeValue(Value? v)
        {
            if (v == null)
            {
                _stream.writeSize(0);
            }
            else if (_current != null && _encaps.format == FormatType.SlicedFormat)
            {
                if (_current.indirectionTable == null)
                {
                    _current.indirectionTable = new List<Value>();
                    _current.indirectionMap = new Dictionary<Value, int>();
                }

                //
                // If writing an instance within a slice and using the sliced
                // format, write an index from the instance indirection table.
                //
                if (!_current.indirectionMap!.TryGetValue(v, out int index))
                {
                    _current.indirectionTable.Add(v);
                    int idx = _current.indirectionTable.Count; // Position + 1 (0 is reserved for nil)
                    _current.indirectionMap.Add(v, idx);
                    _stream.writeSize(idx);
                }
                else
                {
                    _stream.writeSize(index);
                }
            }
            else
            {
                writeInstance(v); // Write the instance or a reference if already marshaled.
            }
        }

        internal override void writeException(UserException v) => v.iceWrite(_stream);

        internal override void startInstance(SliceType sliceType, SlicedData? data)
        {
            if (_current == null)
            {
                _current = new InstanceData(null);
            }
            else
            {
                _current = _current.next ?? new InstanceData(_current);
            }
            _current.sliceType = sliceType;
            _current.firstSlice = true;

            if (data != null)
            {
                writeSlicedData(data);
            }
        }

        internal override void endInstance() => _current = _current!.previous;

        internal override void startSlice(string typeId, int compactId, bool last)
        {
            Debug.Assert((_current!.indirectionTable == null || _current.indirectionTable.Count == 0) &&
                         (_current.indirectionMap == null || _current.indirectionMap.Count == 0));

            _current.sliceFlagsPos = _stream.pos();

            _current.sliceFlags = 0;
            if (_encaps.format == FormatType.SlicedFormat)
            {
                //
                // Encode the slice size if using the sliced format.
                //
                _current.sliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE;
            }
            if (last)
            {
                _current.sliceFlags |= Protocol.FLAG_IS_LAST_SLICE; // This is the last slice.
            }

            _stream.writeByte(0); // Placeholder for the slice flags

            //
            // For instance slices, encode the flag and the type ID either as a
            // string or index. For exception slices, always encode the type
            // ID a string.
            //
            if (_current.sliceType == SliceType.ValueSlice)
            {
                //
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                //
                if (_encaps.format == FormatType.SlicedFormat || _current.firstSlice)
                {
                    if (compactId >= 0)
                    {
                        _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT;
                        _stream.writeSize(compactId);
                    }
                    else
                    {
                        int index = registerTypeId(typeId);
                        if (index < 0)
                        {
                            _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING;
                            _stream.writeString(typeId);
                        }
                        else
                        {
                            _current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX;
                            _stream.writeSize(index);
                        }
                    }
                }
            }
            else
            {
                _stream.writeString(typeId);
            }

            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                _stream.writeInt(0); // Placeholder for the slice length.
            }

            _current.writeSlice = _stream.pos();
            _current.firstSlice = false;
        }

        internal override void endSlice()
        {
            //
            // Write the optional member end marker if some optional members
            // were encoded. Note that the optional members are encoded before
            // the indirection table and are included in the slice size.
            //
            if ((_current!.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS) != 0)
            {
                _stream.writeByte(Protocol.OPTIONAL_END_MARKER);
            }

            //
            // Write the slice length if necessary.
            //
            if ((_current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE) != 0)
            {
                int sz = _stream.pos() - _current.writeSlice + 4;
                _stream.rewriteInt(sz, _current.writeSlice - 4);
            }

            //
            // Only write the indirection table if it contains entries.
            //
            if (_current.indirectionTable != null && _current.indirectionTable.Count > 0)
            {
                Debug.Assert(_encaps.format == FormatType.SlicedFormat);
                _current.sliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE;

                //
                // Write the indirect instance table.
                //
                _stream.writeSize(_current.indirectionTable.Count);
                foreach (Value v in _current.indirectionTable)
                {
                    writeInstance(v);
                }
                _current.indirectionTable.Clear();
                _current.indirectionMap!.Clear();
            }

            //
            // Finally, update the slice flags.
            //
            _stream.rewriteByte(_current.sliceFlags, _current.sliceFlagsPos);
        }

        internal override bool writeOptional(int tag, OptionalFormat format)
        {
            if (_current == null)
            {
                return _stream.writeOptionalImpl(tag, format);
            }
            else
            {
                if (_stream.writeOptionalImpl(tag, format))
                {
                    _current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        private void writeSlicedData(SlicedData slicedData)
        {
            Debug.Assert(slicedData != null);

            //
            // We only remarshal preserved slices if we are using the sliced
            // format. Otherwise, we ignore the preserved slices, which
            // essentially "slices" the instance into the most-derived type
            // known by the sender.
            //
            if (_encaps.format != FormatType.SlicedFormat)
            {
                return;
            }

            foreach (SliceInfo info in slicedData.slices)
            {
                startSlice(info.typeId, info.compactId, info.isLastSlice);

                //
                // Write the bytes associated with this slice.
                //
                _stream.writeBlob(info.bytes);

                if (info.hasOptionalMembers)
                {
                    _current!.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS;
                }

                //
                // Make sure to also re-write the instance indirection table.
                //
                if (info.instances != null && info.instances.Length > 0)
                {
                    if (_current!.indirectionTable == null)
                    {
                        _current.indirectionTable = new List<Value>();
                        _current.indirectionMap = new Dictionary<Value, int>();
                    }
                    foreach (Value o in info.instances)
                    {
                        _current.indirectionTable.Add(o);
                    }
                }

                endSlice();
            }
        }

        private void writeInstance(Value v)
        {
            Debug.Assert(v != null);

            //
            // If the instance was already marshaled, just write it's ID.
            //
            if (_marshaledMap.TryGetValue(v, out int p))
            {
                _stream.writeSize(p);
                return;
            }

            //
            // We haven't seen this instance previously, create a new ID,
            // insert it into the marshaled map, and write the instance.
            //
            _marshaledMap.Add(v, ++_valueIdIndex);

            v.ice_preMarshal();
            _stream.writeSize(1); // Object instance marker.
            v.iceWrite(_stream);
        }

        private sealed class InstanceData
        {
            internal InstanceData(InstanceData? previous)
            {
                if (previous != null)
                {
                    previous.next = this;
                }
                this.previous = previous;
                next = null;
            }

            // Instance attributes
            internal SliceType sliceType;
            internal bool firstSlice;

            // Slice attributes
            internal byte sliceFlags;
            internal int writeSlice;    // Position of the slice data members
            internal int sliceFlagsPos; // Position of the slice flags
            internal List<Value>? indirectionTable;
            internal Dictionary<Value, int>? indirectionMap;

            internal InstanceData? previous;
            internal InstanceData? next;
        }

        private InstanceData? _current;

        private int _valueIdIndex; // The ID of the next instance to marshal
    }

    private sealed class Encaps
    {
        internal void reset() => encoder = null;

        internal void setEncoding(EncodingVersion encoding)
        {
            this.encoding = encoding;
            encoding_1_0 = encoding.Equals(Util.Encoding_1_0);
        }

        internal int start;
        internal EncodingVersion encoding;
        internal bool encoding_1_0;
        internal FormatType format;

        internal EncapsEncoder? encoder;

        internal Encaps? next;
    }

    //
    // The encoding version to use when there's no encapsulation to
    // read from or write to. This is for example used to read message
    // headers or when the user is using the streaming API with no
    // encapsulation.
    //
    private EncodingVersion _encoding;

    private bool isEncoding_1_0() =>
        _encapsStack != null ? _encapsStack.encoding_1_0 : _encoding.Equals(Util.Encoding_1_0);

    private Encaps? _encapsStack;
    private Encaps? _encapsCache;

    private void initEncaps()
    {
        if (_encapsStack == null) // Lazy initialization
        {
            _encapsStack = _encapsCache;
            if (_encapsStack != null)
            {
                _encapsCache = _encapsCache!.next;
            }
            else
            {
                _encapsStack = new Encaps();
            }
            _encapsStack.setEncoding(_encoding);
        }

        if (_encapsStack.encoder == null) // Lazy initialization.
        {
            if (_encapsStack.encoding_1_0)
            {
                _encapsStack.encoder = new EncapsEncoder10(this, _encapsStack);
            }
            else
            {
                _encapsStack.encoder = new EncapsEncoder11(this, _encapsStack);
            }
        }
    }
}
