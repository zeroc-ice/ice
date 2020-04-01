//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Ice
{
    // Map Bzip2 bz_stream struct to a C# struct
    // for using with the Bzip2 low level API.
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

        public BZStream(IntPtr nextOut, uint availOut)
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

    internal delegate int BZDeompressInit(ref BZStream stream, int verbosity, int small);
    internal delegate int BZDeompress(ref BZStream stream);
    internal delegate int BZDeompressEnd(ref BZStream stream);

    internal class BZip2
    {
        private const int BzRun = 0;
        private const int BzFinish = 2;

        private const int BzRunOk = 1;
        private const int BzFinishOk = 3;
        private const int BzStreamEnd = 4;

        static BZip2()
        {
            //
            // Simple trick to find out whether bzip2 is installed: Call the BZ2_bzlibVersion() function in the
            // library. If we get an exception, the library is not available.
            //
            _bzlibInstalled = false;
            _bzlibName = string.Empty;
            try
            {
                if (IceInternal.AssemblyUtil.IsWindows)
                {
                    _bzlibName = "bzip2.dll";
                    SafeNativeMethods.WindowsBZ2_bzlibVersion();
                }
                else if (IceInternal.AssemblyUtil.IsMacOS)
                {
                    _bzlibName = "libbz2.dylib";
                    SafeNativeMethods.MacOSBZ2_bzlibVersion();
                }
                else
                {
                    try
                    {
                        _bzlibName = "libbz2.so.1.0";
                        SafeNativeMethods.UnixBZ2_10_bzlibVersion();
                    }
                    catch (TypeLoadException)
                    {
                        _bzlibName = "libbz2.so.1";
                        SafeNativeMethods.UnixBZ2_1_bzlibVersion();
                    }
                }
                _bzlibInstalled = true;
            }
            catch (EntryPointNotFoundException)
            {
                Console.Error.WriteLine($"warning: found {_bzlibName} but entry point BZ2_bzlibVersion is missing.");
            }
            catch (TypeLoadException)
            {
                // Expected -- bzip2 lib not installed or not in PATH.
            }
            catch (BadImageFormatException ex)
            {
                string lib = _bzlibName;
                if (!string.IsNullOrEmpty(ex.FileName))
                {
                    lib = ex.FileName; // Future-proof: we'll do the right thing if the FileName member is non-empty.
                }
                Console.Error.Write($"warning: {lib} could not be loaded (likely due to 32/64-bit mismatch).");
                if (IntPtr.Size == 8)
                {
                    Console.Error.Write($" Make sure the directory containing the 64-bit {lib} is in your PATH.");
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
            else
            {
                if (_bzlibName == "libbz2.so.1.0")
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
        }

        private static string GetBZ2Error(int error)
        {
            string rc;

            switch (error)
            {
                case BzSequenceError:
                    {
                        rc = "BZ_SEQUENCE_ERROR";
                        break;
                    }
                case BzParamError:
                    {
                        rc = "BZ_PARAM_ERROR";
                        break;
                    }
                case BzMemError:
                    {
                        rc = "BZ_MEM_ERROR";
                        break;
                    }
                case BzDataError:
                    {
                        rc = "BZ_DATA_ERROR";
                        break;
                    }
                case BzDataErrorMagic:
                    {
                        rc = "BZ_DATA_ERROR_MAGIC";
                        break;
                    }
                case BzIoError:
                    {
                        rc = "BZ_IO_ERROR";
                        break;
                    }
                case BzUnexpectedEof:
                    {
                        rc = "BZ_UNEXPECTED_EOF";
                        break;
                    }
                case BzOutbuffFull:
                    {
                        rc = "BZ_OUTBUFF_FULL";
                        break;
                    }
                case BzConfigError:
                    {
                        rc = "BZ_CONFIG_ERROR";
                        break;
                    }
                default:
                    {
                        rc = "Unknown bzip2 error: " + error;
                        break;
                    }
            }
            return rc;
        }

        internal static bool Supported() => _bzlibInstalled;

        internal static List<ArraySegment<byte>>? Compress(
            List<ArraySegment<byte>> data, int size, int headerSize, int compressionLevel)
        {
            Debug.Assert(Supported());

            // Compress the message body, but not the header.
            int uncompressedLen = size - headerSize;
            // Compress the message body, but not the header.
            byte[] compressed = new byte[(int)((uncompressedLen * 1.01) + 600)];

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

                int rc = BzRunOk;
                for (int i = 0; rc == BzRunOk && i < data.Count; i++)
                {
                    ArraySegment<byte> segment = data[i];
                    var segmentHandle = GCHandle.Alloc(segment.Array, GCHandleType.Pinned);
                    bzStream.NextIn = segmentHandle.AddrOfPinnedObject() + segment.Offset;
                    bzStream.AvailIn = (uint)segment.Count;
                    Debug.Assert(bzStream.AvailIn > 0);

                    do
                    {
                        rc = _compress(ref bzStream, BzRun);
                    }
                    while (rc == BzRunOk && bzStream.AvailIn > 0);
                    segmentHandle.Free();
                }

                if (rc != BzRunOk)
                {
                    throw new TransportException($"bzip2 compression failed: {GetBZ2Error(rc)}");
                }

                do
                {
                    rc = _compress(ref bzStream, BzFinish);
                }
                while (rc == BzFinishOk);

                if (rc != BzStreamEnd)
                {
                    throw new TransportException($"bzip2 compression failed: {GetBZ2Error(rc)}");
                }

                int compressedLen = compressed.Length - (int)bzStream.AvailOut;

                // Don't bother if the compressed data is larger than the uncompressed data.
                if (compressedLen >= uncompressedLen)
                {
                    return null;
                }

                // Copy the header from the uncompressed stream to the compressed one,
                // we use headerSize + 4 to ensure there is room for the size of the
                // uncompressed stream in the first segment.
                ArraySegment<byte> compressedHeader = new byte[headerSize + 4];
                headerSegment.AsSpan(0, headerSize).CopyTo(compressedHeader);

                int compressedSize = compressedLen + compressedHeader.Count;
                // Write the compression status and the size of the compressed
                // stream into the header.
                compressedHeader[9] = 2;
                OutputStream.WriteInt(compressedSize, compressedHeader.AsSpan(10, 4));

                // Write the compression status and size of the compressed stream
                // into the header of the uncompressed stream -- we need this to
                // trace requests correctly.
                headerSegment[9] = 2;
                OutputStream.WriteInt(compressedSize, headerSegment.AsSpan(10, 4));

                // Add the size of the uncompressed stream before the message body.
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

        public static ArraySegment<byte> Uncompress(ArraySegment<byte> compressed, int headerSize, int messageSizeMax)
        {
            Debug.Assert(Supported());
            int uncompressedSize = InputStream.ReadInt(compressed.AsSpan(headerSize, 4));
            if (uncompressedSize <= headerSize)
            {
                throw new InvalidDataException(
                    $"received compressed ice1 frame with a decompressed size of only {uncompressedSize} bytes");
            }
            if (uncompressedSize > messageSizeMax)
            {
                throw new InvalidDataException(
                    $"uncompressed size of {uncompressedSize} bytes is greater than Ice.MessageSizeMax value");
            }

            int uncompressedLen = uncompressedSize - headerSize;
            byte[] uncompressed = new byte[uncompressedSize];

            // Prevent GC from moving the byte array, this allow to take the object address
            // and pass it to bzip2 calls.
            var uncompressedHandle = GCHandle.Alloc(uncompressed, GCHandleType.Pinned);
            var compressedHandle = GCHandle.Alloc(compressed.Array, GCHandleType.Pinned);
            var bzStream = new BZStream(uncompressedHandle.AddrOfPinnedObject() + headerSize,
                (uint)(uncompressedSize - headerSize));

            try
            {
                _decompressInit(ref bzStream, 0, 0);

                bzStream.NextIn = compressedHandle.AddrOfPinnedObject() + compressed.Offset + headerSize + 4;
                bzStream.AvailIn = (uint)(compressed.Count - headerSize - 4);
                int rc = _decompress(ref bzStream);
                if (rc != BzStreamEnd)
                {
                    throw new TransportException($"bzip2 decompression failed: {GetBZ2Error(rc)}");
                }
            }
            finally
            {
                _decompressEnd(ref bzStream);
                uncompressedHandle.Free();
                compressedHandle.Free();
            }
            compressed.AsSpan(0, headerSize).CopyTo(uncompressed);
            return uncompressed;
        }

        private static readonly bool _bzlibInstalled;
        private static readonly string _bzlibName;

        private static readonly BZCompressInit _compressInit;
        private static readonly BZCompress _compress;
        private static readonly BZCompressEnd _compressEnd;

        private static readonly BZDeompressInit _decompressInit;
        private static readonly BZDeompress _decompress;
        private static readonly BZDeompressEnd _decompressEnd;

        private const int BzSequenceError = -1;
        private const int BzParamError = -2;
        private const int BzMemError = -3;
        private const int BzDataError = -4;
        private const int BzDataErrorMagic = -5;
        private const int BzIoError = -6;
        private const int BzUnexpectedEof = -7;
        private const int BzOutbuffFull = -8;
        private const int BzConfigError = -9;
    }

}
