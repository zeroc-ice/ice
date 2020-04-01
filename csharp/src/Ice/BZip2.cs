//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Ice
{
    // Map Bzip2 bz_stream struct to a C# struct for using with the Bzip2 low level API.
    [StructLayout(LayoutKind.Sequential)]
    internal struct BZStream
    {
        internal IntPtr NextIn;
        internal uint AvailIn;
        internal uint TotalInLo32;
        internal uint TotalInHi32;

        internal IntPtr NextOut;
        internal uint AvailOut;
        internal uint TotalOutLo32;
        internal uint TotalOutHi32;

        internal IntPtr State;
        internal IntPtr BzAlloc;
        internal IntPtr BzFree;
        internal IntPtr Opaque;

        internal BZStream(IntPtr nextOut, uint availOut)
        {
            NextIn = IntPtr.Zero;
            AvailIn = 0;
            TotalInLo32 = 0;
            TotalInHi32 = 0;

            NextOut = nextOut;
            AvailOut = availOut;
            TotalOutLo32 = 0;
            TotalOutHi32 = 0;

            State = IntPtr.Zero;
            BzAlloc = IntPtr.Zero;
            BzFree = IntPtr.Zero;
            Opaque = IntPtr.Zero;
        }
    }

    internal static class SafeNativeMethods
    {
        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr WindowsBZ2_bzlibVersion();

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzCompressInit(ref BZStream stream, int blockSize100k, int verbosity,
                                                             int workFactor);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzCompress(ref BZStream stream, int action);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzCompressEnd(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzDecompressInit(ref BZStream stream, int verbosity, int small);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzDecompress(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzDecompressEnd(ref BZStream stream);

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr UnixBZ2_1_bzlibVersion();

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzCompressInit(ref BZStream stream, int blockSize100k, int verbosity,
                                                            int workFactor);

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzCompress(ref BZStream stream, int action);

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzCompressEnd(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzDecompressInit(ref BZStream stream, int verbosity, int small);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzDecompress(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzDecompressEnd(ref BZStream stream);

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr UnixBZ2_10_bzlibVersion();

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzCompressInit(ref BZStream stream, int blockSize100k, int verbosity,
                                                             int workFactor);

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzCompress(ref BZStream stream, int action);

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzCompressEnd(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzDecompressInit(ref BZStream stream, int verbosity, int small);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzDecompress(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzDecompressEnd(ref BZStream stream);

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr MacOSBZ2_bzlibVersion();

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzCompressInit(ref BZStream stream, int blockSize100k, int verbosity,
                                                             int workFactor);

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzCompress(ref BZStream stream, int action);

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzCompressEnd(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzDecompressInit(ref BZStream stream, int verbosity, int small);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzDecompress(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzDecompressEnd(ref BZStream stream);
    }

    internal delegate int BZCompressInit(ref BZStream stream, int blockSize100k, int verbosity, int workFactor);
    internal delegate int BZCompress(ref BZStream stream, int action);
    internal delegate int BZCompressEnd(ref BZStream stream);

    internal delegate int BZDecompressInit(ref BZStream stream, int verbosity, int small);
    internal delegate int BZDeompress(ref BZStream stream);
    internal delegate int BZDeompressEnd(ref BZStream stream);

    internal class BZip2
    {
        private enum BzAction
        {
            Run = 0,
            Flush = 1,
            Finish = 2
        }

        private enum BzStatus
        {
            RunOk = 1,
            FinishOk = 3,
            StreamEnd = 4,

            SequenceError = -1,
            ParamError = -2,
            MemError = -3,
            DataError = -4,
            DataErrorMagic = -5,
            IoError = -6,
            UnexpectedEof = -7,
            OutbuffFull = -8,
            ConfigError = -9
        }

        internal static bool Supported { get; }

        private static readonly BZCompressInit _compressInit;
        private static readonly BZCompress _compress;
        private static readonly BZCompressEnd _compressEnd;

        private static readonly BZDecompressInit _decompressInit;
        private static readonly BZDeompress _decompress;
        private static readonly BZDeompressEnd _decompressEnd;

#pragma warning disable CA1810 // Initialize reference type static fields inline
        static BZip2()
#pragma warning restore CA1810 // Initialize reference type static fields inline
        {
            // Find out whether bzip2 is installed: Call the BZ2_bzlibVersion() function
            // in the library. If we get an exception, the library is not available.
            Supported = false;
            string bzlibName = string.Empty;
            try
            {
                if (IceInternal.AssemblyUtil.IsWindows)
                {
                    bzlibName = "bzip2.dll";
                    SafeNativeMethods.WindowsBZ2_bzlibVersion();
                }
                else if (IceInternal.AssemblyUtil.IsMacOS)
                {
                    bzlibName = "libbz2.dylib";
                    SafeNativeMethods.MacOSBZ2_bzlibVersion();
                }
                else
                {
                    try
                    {
                        bzlibName = "libbz2.so.1.0";
                        SafeNativeMethods.UnixBZ2_10_bzlibVersion();
                    }
                    catch (TypeLoadException)
                    {
                        bzlibName = "libbz2.so.1";
                        SafeNativeMethods.UnixBZ2_1_bzlibVersion();
                    }
                }
                Supported = true;
            }
            catch (EntryPointNotFoundException)
            {
                Console.Error.WriteLine($"warning: found {bzlibName} but entry point BZ2_bzlibVersion is missing.");
            }
            catch (TypeLoadException)
            {
                // Expected -- bzip2 lib not installed or not in PATH.
            }
            catch (BadImageFormatException)
            {
                Console.Error.Write($"warning: {bzlibName} could not be loaded (likely due to 32/64-bit mismatch).");
                if (IntPtr.Size == 8)
                {
                    Console.Error.Write($" Make sure the directory containing the 64-bit {bzlibName} is in your PATH.");
                }
                Console.Error.WriteLine();
            }

            if (IceInternal.AssemblyUtil.IsWindows)
            {
                _decompressInit = (ref BZStream stream, int verbosity, int small) =>
                    SafeNativeMethods.WindowsBZ2_bzDecompressInit(ref stream, verbosity, small);

                _decompress = (ref BZStream stream) => SafeNativeMethods.WindowsBZ2_bzDecompress(ref stream);

                _decompressEnd = (ref BZStream stream) =>
                    SafeNativeMethods.WindowsBZ2_bzDecompressEnd(ref stream);

                _compressInit = (ref BZStream stream, int blockSize100k, int verbosity, int workFactor) =>
                    SafeNativeMethods.WindowsBZ2_bzCompressInit(ref stream, blockSize100k, verbosity, workFactor);

                _compress = (ref BZStream stream, int action) =>
                    SafeNativeMethods.WindowsBZ2_bzCompress(ref stream, action);

                _compressEnd = (ref BZStream stream) => SafeNativeMethods.WindowsBZ2_bzCompressEnd(ref stream);
            }
            else if (IceInternal.AssemblyUtil.IsMacOS)
            {
                _decompressInit = (ref BZStream stream, int verbosity, int small) =>
                    SafeNativeMethods.MacOSBZ2_bzDecompressInit(ref stream, verbosity, small);

                _decompress = (ref BZStream stream) => SafeNativeMethods.MacOSBZ2_bzDecompress(ref stream);

                _decompressEnd = (ref BZStream stream) =>
                    SafeNativeMethods.MacOSBZ2_bzDecompressEnd(ref stream);

                _compressInit = (ref BZStream stream, int blockSize100k, int verbosity, int workFactor) =>
                    SafeNativeMethods.MacOSBZ2_bzCompressInit(ref stream, blockSize100k, verbosity, workFactor);

                _compress = (ref BZStream stream, int action) =>
                    SafeNativeMethods.MacOSBZ2_bzCompress(ref stream, action);

                _compressEnd = (ref BZStream stream) => SafeNativeMethods.MacOSBZ2_bzCompressEnd(ref stream);
            }
            else if (bzlibName == "libbz2.so.1.0")
            {
                _decompressInit = (ref BZStream stream, int verbosity, int small) =>
                    SafeNativeMethods.UnixBZ2_10_bzDecompressInit(ref stream, verbosity, small);

                _decompress = (ref BZStream stream) => SafeNativeMethods.UnixBZ2_10_bzDecompress(ref stream);

                _decompressEnd = (ref BZStream stream) =>
                    SafeNativeMethods.UnixBZ2_10_bzDecompressEnd(ref stream);

                _compressInit = (ref BZStream stream, int blockSize100k, int verbosity, int workFactor) =>
                    SafeNativeMethods.UnixBZ2_10_bzCompressInit(ref stream, blockSize100k, verbosity, workFactor);

                _compress = (ref BZStream stream, int action) =>
                    SafeNativeMethods.UnixBZ2_10_bzCompress(ref stream, action);

                _compressEnd = (ref BZStream stream) => SafeNativeMethods.UnixBZ2_10_bzCompressEnd(ref stream);
            }
            else
            {
                _decompressInit = (ref BZStream stream, int verbosity, int small) =>
                    SafeNativeMethods.UnixBZ2_1_bzDecompressInit(ref stream, verbosity, small);

                _decompress = (ref BZStream stream) => SafeNativeMethods.UnixBZ2_1_bzDecompress(ref stream);

                _decompressEnd = (ref BZStream stream) =>
                    SafeNativeMethods.UnixBZ2_1_bzDecompressEnd(ref stream);

                _compressInit = (ref BZStream stream, int blockSize100k, int verbosity, int workFactor) =>
                    SafeNativeMethods.UnixBZ2_1_bzCompressInit(ref stream, blockSize100k, verbosity, workFactor);

                _compress = (ref BZStream stream, int action) =>
                    SafeNativeMethods.UnixBZ2_1_bzCompress(ref stream, action);

                _compressEnd = (ref BZStream stream) => SafeNativeMethods.UnixBZ2_1_bzCompressEnd(ref stream);
            }
        }

        internal static List<ArraySegment<byte>>? Compress(
            List<ArraySegment<byte>> data, int size, int headerSize, int compressionLevel)
        {
            Debug.Assert(Supported);

            // Compress the message body, but not the header.
            int decompressedLen = size - headerSize;
            // Compress the message body, but not the header.
            byte[] compressed = new byte[(int)((decompressedLen * 1.01) + 600)];

            // Prevent GC from moving the byte array, this allow to take the object address
            // and pass it to bzip2 calls.
            var compressedHandle = GCHandle.Alloc(compressed, GCHandleType.Pinned);
            var bzStream = new BZStream(compressedHandle.AddrOfPinnedObject(), (uint)compressed.Length);

            ArraySegment<byte> headerSegment = data[0];
            try
            {
                _compressInit(ref bzStream, compressionLevel, 0, 0);

                // Slice the first segment to skip the header, the header is never compressed
                Debug.Assert(headerSegment.Offset == 0);
                data[0] = headerSegment.Slice(headerSize);

                BzStatus rc = BzStatus.RunOk;
                for (int i = 0; rc == BzStatus.RunOk && i < data.Count; i++)
                {
                    ArraySegment<byte> segment = data[i];
                    var segmentHandle = GCHandle.Alloc(segment.Array, GCHandleType.Pinned);
                    bzStream.NextIn = segmentHandle.AddrOfPinnedObject() + segment.Offset;
                    bzStream.AvailIn = (uint)segment.Count;
                    Debug.Assert(bzStream.AvailIn > 0);

                    do
                    {
                        rc = (BzStatus)_compress(ref bzStream, (int)BzAction.Run);
                    }
                    while (rc == BzStatus.RunOk && bzStream.AvailIn > 0);
                    segmentHandle.Free();
                }

                if (rc != BzStatus.RunOk)
                {
                    throw new TransportException($"bzip2 compression failed: {rc}");
                }

                do
                {
                    rc = (BzStatus)_compress(ref bzStream, (int)BzAction.Finish);
                }
                while (rc == BzStatus.FinishOk);

                if (rc != BzStatus.StreamEnd)
                {
                    throw new TransportException($"bzip2 compression failed: {rc}");
                }

                int compressedLen = compressed.Length - (int)bzStream.AvailOut;

                // Don't bother if the compressed data is larger than the decompressed data.
                if (compressedLen >= decompressedLen)
                {
                    return null;
                }

                // Copy the header from the decompressed stream to the compressed one,
                // we use headerSize + 4 to ensure there is room for the size of the
                // decompressed stream in the first segment.
                ArraySegment<byte> compressedHeader = new byte[headerSize + 4];
                headerSegment.AsSpan(0, headerSize).CopyTo(compressedHeader);

                int compressedSize = compressedLen + compressedHeader.Count;
                // Write the compression status and the size of the compressed
                // stream into the header.
                compressedHeader[9] = 2;
                OutputStream.WriteInt(compressedSize, compressedHeader.AsSpan(10, 4));

                // Write the compression status and size of the compressed stream
                // into the header of the decompressed stream -- we need this to
                // trace requests correctly.
                headerSegment[9] = 2;
                OutputStream.WriteInt(compressedSize, headerSegment.AsSpan(10, 4));

                // Add the size of the decompressed stream before the message body.
                OutputStream.WriteInt(size, compressedHeader.AsSpan(headerSize, 4));

                return new List<ArraySegment<byte>>(2)
                    {
                        compressedHeader,
                        new ArraySegment<byte>(compressed, 0, compressedLen)
                    };
            }
            finally
            {
                // Restore the first segment that was Sliced above to skip the header
                data[0] = headerSegment;
                _compressEnd(ref bzStream);
                compressedHandle.Free();
            }
        }

        internal static ArraySegment<byte> Decompress(ArraySegment<byte> compressed, int headerSize, int messageSizeMax)
        {
            Debug.Assert(Supported);
            int decompressedSize = InputStream.ReadInt(compressed.AsSpan(headerSize, 4));
            if (decompressedSize <= headerSize)
            {
                throw new InvalidDataException(
                    $"received compressed ice1 frame with a decompressed size of only {decompressedSize} bytes");
            }
            if (decompressedSize > messageSizeMax)
            {
                throw new InvalidDataException(
                    $"decompressed size of {decompressedSize} bytes is greater than Ice.MessageSizeMax value");
            }

            int decompressedLen = decompressedSize - headerSize;
            byte[] decompressed = new byte[decompressedSize];

            // Prevent GC from moving the byte array, this allow to take the object address
            // and pass it to bzip2 calls.
            var decompressedHandle = GCHandle.Alloc(decompressed, GCHandleType.Pinned);
            var compressedHandle = GCHandle.Alloc(compressed.Array, GCHandleType.Pinned);
            var bzStream = new BZStream(decompressedHandle.AddrOfPinnedObject() + headerSize,
                (uint)(decompressedSize - headerSize));

            try
            {
                _decompressInit(ref bzStream, 0, 0);

                bzStream.NextIn = compressedHandle.AddrOfPinnedObject() + compressed.Offset + headerSize + 4;
                bzStream.AvailIn = (uint)(compressed.Count - headerSize - 4);
                var rc = (BzStatus)_decompress(ref bzStream);
                if (rc != BzStatus.StreamEnd)
                {
                    throw new TransportException($"bzip2 decompression failed: {rc}");
                }
            }
            finally
            {
                _decompressEnd(ref bzStream);
                decompressedHandle.Free();
                compressedHandle.Free();
            }
            compressed.AsSpan(0, headerSize).CopyTo(decompressed);
            return decompressed;
        }
    }

}
