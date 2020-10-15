// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace ZeroC.Ice
{
    /// <summary>Provides extension methods for byte buffers (such as <c>ReadOnlyMemory{byte}</c>) to read and write
    /// data encoded using the Ice encoding.</summary>
    public static class ByteBufferExtensions
    {
        private static readonly System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);

        /// <summary>Reads a value from the buffer.</summary>
        /// <typeparam name="T">The type of the value.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of the data in the buffer.</param>
        /// <param name="communicator">The communicator, which is mandatory only when reading proxies.</param>
        /// <param name="reader">The <see cref="InputStreamReader{T}"/> that reads the value from the buffer using an
        /// <see cref="InputStream"/>.</param>
        /// <returns>The value read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>reader</c> finds invalid data or <c>reader</c> leaves
        /// unread data in the buffer.</exception>
        public static T Read<T>(
            this ReadOnlyMemory<byte> buffer,
            Encoding encoding,
            Communicator? communicator,
            InputStreamReader<T> reader)
        {
            var istr = new InputStream(buffer, encoding, communicator);
            T result = reader(istr);
            istr.CheckEndOfBuffer(skipTaggedParams: false);
            return result;
        }

        /// <summary>Reads a value from the buffer that uses the Ice 2.0 encoding.</summary>
        /// <typeparam name="T">The type of the value.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="communicator">The communicator, which is mandatory only when reading proxies.</param>
        /// <param name="reader">The <see cref="InputStreamReader{T}"/> that reads the value from the buffer using an
        /// <see cref="InputStream"/>.</param>
        /// <returns>The value read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>reader</c> finds invalid data or <c>reader</c> leaves
        /// unread data in the buffer.</exception>
        public static T Read<T>(
            this ReadOnlyMemory<byte> buffer,
            Communicator communicator,
            InputStreamReader<T> reader) => buffer.Read(Encoding.V20, communicator, reader);

        /// <summary>Reads a value from the buffer. Value cannot contain any proxy.</summary>
        /// <typeparam name="T">The type of the value.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of the data in the buffer.</param>
        /// <param name="reader">The <see cref="InputStreamReader{T}"/> that reads the value from the buffer using an
        /// <see cref="InputStream"/>.</param>
        /// <returns>The value read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>reader</c> finds invalid data or <c>reader</c> leaves
        /// unread data in the buffer.</exception>
        public static T Read<T>(
            this ReadOnlyMemory<byte> buffer,
            Encoding encoding,
            InputStreamReader<T> reader) => buffer.Read(encoding, null, reader);

        /// <summary>Reads a value from the buffer that uses the 2.0 encoding. Value cannot contain any proxy.
        /// </summary>
        /// <typeparam name="T">The type of the value.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="reader">The <see cref="InputStreamReader{T}"/> that reads the value from the buffer using an
        /// <see cref="InputStream"/>.</param>
        /// <returns>The value read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>reader</c> finds invalid data or <c>reader</c> leaves
        /// unread data in the buffer.</exception>
        public static T Read<T>(this ReadOnlyMemory<byte> buffer, InputStreamReader<T> reader) =>
            buffer.Read(Encoding.V20, null, reader);

        /// <summary>Reads an empty encapsulation from the buffer.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of encapsulation header.</param>
        /// <exception name="InvalidDataException">Thrown when the buffer is not an empty encapsulation, for example
        /// when buffer contains an encapsulation that does not have only tagged parameters.</exception>
        public static void ReadEmptyEncapsulation(this ReadOnlyMemory<byte> buffer, Encoding encoding) =>
            new InputStream(buffer,
                            encoding,
                            communicator: null,
                            startEncapsulation: true).CheckEndOfBuffer(skipTaggedParams: true);

        /// <summary>Reads an empty encapsulation from the buffer, with the encapsulation header encoded using the 2.0
        /// encoding.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <exception name="InvalidDataException">Thrown when the buffer is not an empty encapsulation, for example
        /// when buffer contains an encapsulation that does not have only tagged parameters.</exception>
        public static void ReadEmptyEncapsulation(this ReadOnlyMemory<byte> buffer) =>
            buffer.ReadEmptyEncapsulation(Encoding.V20);

        /// <summary>Reads the contents of an encapsulation from the buffer.</summary>
        /// <typeparam name="T">The type of the contents.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of encapsulation header in the buffer.</param>
        /// <param name="communicator">The communicator.</param>
        /// <param name="payloadReader">The <see cref="InputStreamReader{T}"/> that reads the payload of the
        /// encapsulation using an <see cref="InputStream"/>.</param>
        /// <returns>The contents of the encapsulation read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>buffer</c> is not a valid encapsulation or
        /// <c>payloadReader</c> finds invalid data.</exception>
        public static T ReadEncapsulation<T>(
            this ReadOnlyMemory<byte> buffer,
            Encoding encoding,
            Communicator communicator,
            InputStreamReader<T> payloadReader)
        {
            var istr = new InputStream(buffer, encoding, communicator, startEncapsulation: true);
            T result = payloadReader(istr);
            istr.CheckEndOfBuffer(skipTaggedParams: true);
            return result;
        }

        /// <summary>Reads the contents of an encapsulation from the buffer, with the encapsulation header encoded
        /// using the 2.0 encoding.</summary>
        /// <typeparam name="T">The type of the contents.</typeparam>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="communicator">The communicator.</param>
        /// <param name="payloadReader">The <see cref="InputStreamReader{T}"/> that reads the payload of the
        /// encapsulation using an <see cref="InputStream"/>.</param>
        /// <returns>The contents of the encapsulation read from the buffer.</returns>
        /// <exception name="InvalidDataException">Thrown when <c>buffer</c> is not a valid encapsulation or
        /// <c>payloadReader</c> finds invalid data.</exception>
        public static T ReadEncapsulation<T>(
            this ReadOnlyMemory<byte> buffer,
            Communicator communicator,
            InputStreamReader<T> payloadReader) =>
            buffer.ReadEncapsulation(Encoding.V20, communicator, payloadReader);

        internal static ReadOnlyMemory<T> AsReadOnlyMemory<T>(this ArraySegment<T> segment) => segment;

        internal static ReadOnlyMemory<T> AsReadOnlyMemory<T>(this ArraySegment<T> segment, int start) =>
            segment.AsMemory(start);

        internal static ReadOnlyMemory<T> AsReadOnlyMemory<T>(this ArraySegment<T> segment, int start, int length) =>
            segment.AsMemory(start, length);

        internal static ReadOnlySpan<T> AsReadOnlySpan<T>(this ArraySegment<T> segment) => segment;

        internal static ReadOnlySpan<T> AsReadOnlySpan<T>(this ArraySegment<T> segment, int start) =>
            segment.AsSpan(start);

        internal static ReadOnlySpan<T> AsReadOnlySpan<T>(this ArraySegment<T> segment, int start, int length) =>
            segment.AsSpan(start, length);

        internal static (int Size, int SizeLength, Encoding Encoding) ReadEncapsulationHeader(
            this ReadOnlySpan<byte> buffer,
            Encoding encoding)
        {
            int sizeLength;
            int size;

            if (encoding == Encoding.V11)
            {
                sizeLength = 4;
                size = buffer.ReadInt() - sizeLength; // Remove the size length which is included with the 1.1 encoding.
                if (size < 0)
                {
                    throw new InvalidDataException(
                        $"the 1.1 encapsulation's size ({size + sizeLength}) is too small");
                }
            }
            else
            {
                (size, sizeLength) = buffer.ReadSize20();
            }

            if (sizeLength + size > buffer.Length)
            {
                throw new InvalidDataException(
                    $"the encapsulation's size ({size}) extends beyond the end of the buffer");
            }

            return (size, sizeLength, new Encoding(buffer[sizeLength], buffer[sizeLength + 1]));
        }

        internal static int ReadFixedLengthSize(this ReadOnlySpan<byte> buffer, Encoding encoding)
        {
            if (encoding == Encoding.V11)
            {
                return buffer.ReadInt();
            }
            else
            {
                return buffer.ReadSize20().Size;
            }
        }

        internal static int ReadInt(this ReadOnlySpan<byte> buffer) => BitConverter.ToInt32(buffer);
        internal static long ReadLong(this ReadOnlySpan<byte> buffer) => BitConverter.ToInt64(buffer);
        internal static short ReadShort(this ReadOnlySpan<byte> buffer) => BitConverter.ToInt16(buffer);
        internal static ushort ReadUShort(this ReadOnlySpan<byte> buffer) => BitConverter.ToUInt16(buffer);

        internal static (int Size, int SizeLength) ReadSize(this ReadOnlySpan<byte> buffer, Encoding encoding) =>
            encoding == Encoding.V11 ? buffer.ReadSize11() : buffer.ReadSize20();

        /// <summary>Reads a string from a UTF-8 byte buffer. The size of the byte buffer corresponds to the number of
        /// UTF-8 code points in the string.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <returns>The string read from the buffer.</returns>
        internal static string ReadString(this ReadOnlySpan<byte> buffer) =>
            buffer.IsEmpty ? "" : _utf8.GetString(buffer);

        private static (int Size, int SizeLength) ReadSize11(this ReadOnlySpan<byte> buffer)
        {
            byte b = buffer[0];
            if (b < 255)
            {
                return (b, 1);
            }

            int size = buffer.Slice(1, 4).ReadInt();
            if (size < 0)
            {
                throw new InvalidDataException($"read invalid size: {size}");
            }
            return (size, 5);
        }

        internal static (int Size, int SizeLength) ReadSize20(this ReadOnlySpan<byte> buffer)
        {
            ulong size = (buffer[0] & 0x03) switch
            {
                0 => (uint)buffer[0] >> 2,
                1 => (uint)BitConverter.ToUInt16(buffer) >> 2,
                2 => BitConverter.ToUInt32(buffer) >> 2,
                _ => BitConverter.ToUInt64(buffer) >> 2
            };

            checked // make sure we don't overflow
            {
                return ((int)size, buffer[0].ReadSizeLength20());
            }
        }

        internal static int ReadSizeLength20(this byte b) => b.ReadVarLongLength();

        internal static (long Value, int ValueLength) ReadVarLong(this ReadOnlySpan<byte> buffer)
        {
            long value = (buffer[0] & 0x03) switch
            {
                0 => buffer[0] >> 2,
                1 => BitConverter.ToInt16(buffer) >> 2,
                2 => BitConverter.ToInt32(buffer) >> 2,
                _ => BitConverter.ToInt64(buffer) >> 2
            };

            return (value, buffer[0].ReadVarLongLength());
        }

        internal static int ReadVarLongLength(this byte b) => 1 << (b & 0x03);

        internal static (ulong Value, int ValueLength) ReadVarULong(this ReadOnlySpan<byte> buffer)
        {
            ulong value = (buffer[0] & 0x03) switch
            {
                0 => (uint)buffer[0] >> 2,
                1 => (uint)BitConverter.ToUInt16(buffer) >> 2,
                2 => BitConverter.ToUInt32(buffer) >> 2,
                _ => BitConverter.ToUInt64(buffer) >> 2
            };

            return (value, buffer[0].ReadVarLongLength());
        }

        internal static void WriteEncapsulationSize(this Span<byte> buffer, int size, Encoding encoding)
        {
            if (encoding == Encoding.V20)
            {
                buffer.WriteFixedLengthSize20(size);
            }
            else
            {
                buffer.WriteInt(size + 4);
            }
        }

        /// <summary>Writes a size into a span of bytes using a fixed number of bytes.</summary>
        /// <param name="buffer">The destination byte buffer, which must be 1, 2 or 4 bytes long.</param>
        /// <param name="size">The size to write.</param>
        internal static void WriteFixedLengthSize20(this Span<byte> buffer, int size)
        {
            int sizeLength = buffer.Length;
            Debug.Assert(sizeLength == 1 || sizeLength == 2 || sizeLength == 4);

            (uint encodedSize, uint maxSize) = sizeLength switch
            {
                1 => (0x00u, 63u), // 2^6 - 1
                2 => (0x01u, 16_383u), // 2^14 - 1
                _ => (0x02u, 1_073_741_823u) // 2^30 - 1
            };

            if (size < 0 || size > maxSize)
            {
                throw new ArgumentOutOfRangeException("size is out of range", nameof(size));
            }

            Span<byte> uintBuf = stackalloc byte[4];
            uint v = (uint)size;
            v <<= 2;

            v |= encodedSize;
            MemoryMarshal.Write(uintBuf, ref v);
            uintBuf.Slice(0, sizeLength).CopyTo(buffer);
        }

        /// <summary>Writes a varlong into a span of bytes using a fixed number of bytes.</summary>
        /// <param name="buffer">The destination byte buffer, which must be 1, 2, 4 or 8 bytes long.</param>
        /// <param name="value">The value to write.</param>
        internal static void WriteFixedLengthVarLong(this Span<byte> buffer, long value)
        {
            Debug.Assert(buffer.Length >= OutputStream.GetVarLongLength(value));

            Span<byte> longBuf = stackalloc byte[8];
            long v = value;
            v <<= 2;
            uint encodedSize = buffer.Length switch
            {
                1 => 0x00,
                2 => 0x01,
                4 => 0x02,
                _ => 0x03
            };
            v |= encodedSize;

            MemoryMarshal.Write(longBuf, ref v);
            longBuf.Slice(0, buffer.Length).CopyTo(buffer);
        }

        internal static void WriteInt(this Span<byte> buffer, int v) => MemoryMarshal.Write(buffer, ref v);
    }
}
