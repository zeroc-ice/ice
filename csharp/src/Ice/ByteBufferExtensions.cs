//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

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
            InputStreamReader<T> reader) => buffer.Read(Encoding.V2_0, communicator, reader);

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
            buffer.Read(Encoding.V2_0, null, reader);

        /// <summary>Reads an empty encapsulation from the buffer.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="encoding">The encoding of encapsulation header.</param>
        /// <param name="communicator">The communicator.</param>
        /// <exception name="InvalidDataException">Thrown when the buffer is not an empty encapsulation, for example
        /// when buffer contains an encapsulation that does not have only tagged parameters.</exception>
        // TODO: eliminate communicator parameter once tagged classes are gone
        public static void ReadEmptyEncapsulation(
            this ReadOnlyMemory<byte> buffer,
            Encoding encoding,
            Communicator communicator) =>
            new InputStream(buffer,
                            encoding,
                            communicator,
                            startEncapsulation: true).CheckEndOfBuffer(skipTaggedParams: true);

        /// <summary>Reads an empty encapsulation from the buffer, with the encapsulation header encoded using the 2.0
        /// encoding.</summary>
        /// <param name="buffer">The byte buffer.</param>
        /// <param name="communicator">The communicator.</param>
        /// <exception name="InvalidDataException">Thrown when the buffer is not an empty encapsulation, for example
        /// when buffer contains an encapsulation that does not have only tagged parameters.</exception>
        // TODO: eliminate communicator parameter once tagged classes are gone
        public static void ReadEmptyEncapsulation(this ReadOnlyMemory<byte> buffer, Communicator communicator) =>
            buffer.ReadEmptyEncapsulation(Encoding.V2_0, communicator);

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
            buffer.ReadEncapsulation(Encoding.V2_0, communicator, payloadReader);

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

        internal static (int Size, Encoding Encoding) ReadEncapsulationHeader(
            this ReadOnlySpan<byte> buffer,
            Encoding encoding)
        {
            int sizeLength;
            int size;

            if (encoding == Encoding.V1_1)
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

            return (size, new Encoding(buffer[sizeLength], buffer[sizeLength + 1]));
        }

        internal static int ReadFixedLengthSize(this ReadOnlySpan<byte> buffer, Encoding encoding)
        {
            if (encoding == Encoding.V1_1)
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

        internal static (int Size, int SizeLength) ReadSize(this ReadOnlySpan<byte> buffer, Encoding encoding) =>
            encoding == Encoding.V1_1 ? buffer.ReadSize11() : buffer.ReadSize20();

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

        private static (int Size, int SizeLength) ReadSize20(this ReadOnlySpan<byte> buffer)
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
                return ((int)size, 1 << (buffer[0] & 0x03));
            }
        }
    }
}
