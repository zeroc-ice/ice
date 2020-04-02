//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;

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

        internal static string LibName { get; private set; }
        internal static bool Supported { get; }

#pragma warning disable CA1810 // Initialize reference type static fields inline
        static BZip2()
#pragma warning restore CA1810 // Initialize reference type static fields inline
        {
            LibName = "";
            // Find out whether bzip2 is installed: Call the BZ2_bzlibVersion() function
            // in the library. If we get an exception, the library is not available.
            // Register a dll import delegate to load native libraries used by Ice assembly.
            NativeLibrary.SetDllImportResolver(Assembly.GetAssembly(typeof(BZip2))!, DllImportResolver);
            Supported = false;
            try
            {
                BZ2_bzLibVersion();
                Supported = true;
            }
            catch (EntryPointNotFoundException)
            {
                Console.Error.WriteLine($"warning: found {LibName} but entry point BZ2_bzlibVersion is missing.");
            }
            catch (TypeLoadException)
            {
                // Expected -- bzip2 lib not installed or not in PATH.
            }
            catch (BadImageFormatException)
            {
                Console.Error.Write($"warning: {LibName} could not be loaded (likely due to 32/64-bit mismatch).");
                if (IntPtr.Size == 8)
                {
                    Console.Error.Write($" Make sure the directory containing the 64-bit {LibName} is in your PATH.");
                }
                Console.Error.WriteLine();
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
                BZ2_bzCompressInit(ref bzStream, compressionLevel, 0, 0);

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
                        rc = (BzStatus)BZ2_bzCompress(ref bzStream, (int)BzAction.Run);
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
                    rc = (BzStatus)BZ2_bzCompress(ref bzStream, (int)BzAction.Finish);
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
                BZ2_bzCompressEnd(ref bzStream);
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

            byte[] decompressed = new byte[decompressedSize];

            // Prevent GC from moving the byte array, this allow to take the object address
            // and pass it to bzip2 calls.
            var decompressedHandle = GCHandle.Alloc(decompressed, GCHandleType.Pinned);
            var compressedHandle = GCHandle.Alloc(compressed.Array, GCHandleType.Pinned);
            var bzStream = new BZStream(decompressedHandle.AddrOfPinnedObject() + headerSize,
                (uint)(decompressedSize - headerSize));

            try
            {
                BZ2_bzDecompressInit(ref bzStream, 0, 0);

                bzStream.NextIn = compressedHandle.AddrOfPinnedObject() + compressed.Offset + headerSize + 4;
                bzStream.AvailIn = (uint)(compressed.Count - headerSize - 4);
                var rc = (BzStatus)BZ2_bzDecompress(ref bzStream);
                if (rc != BzStatus.StreamEnd)
                {
                    throw new TransportException($"bzip2 decompression failed: {rc}");
                }
            }
            finally
            {
                BZ2_bzDecompressEnd(ref bzStream);
                decompressedHandle.Free();
                compressedHandle.Free();
            }
            compressed.AsSpan(0, headerSize).CopyTo(decompressed);
            return decompressed;
        }

        private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            Debug.Assert(libraryName == "bzip2.dll");
            if (IceInternal.AssemblyUtil.IsWindows)
            {
                LibName = libraryName;
                return NativeLibrary.Load(libraryName, assembly, searchPath);
            }
            else if (IceInternal.AssemblyUtil.IsMacOS)
            {
                LibName = Path.ChangeExtension(libraryName, ".dylib");
                return NativeLibrary.Load(LibName, assembly, searchPath);
            }
            else
            {
                try
                {
                    LibName = Path.ChangeExtension(libraryName, ".so.1.0");
                    return NativeLibrary.Load(LibName, assembly, searchPath);
                }
                catch (DllNotFoundException)
                {
                    LibName = Path.ChangeExtension(libraryName, ".so.1");
                    return NativeLibrary.Load(LibName, assembly, searchPath);
                }
            }
        }

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        private static extern IntPtr BZ2_bzLibVersion();

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
        private static extern int BZ2_bzCompressInit(ref BZStream stream, int blockSize100k, int verbosity,
            int workFactor);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
        private static extern int BZ2_bzCompress(ref BZStream stream, int action);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
        private static extern int BZ2_bzCompressEnd(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
        private static extern int BZ2_bzDecompressInit(ref BZStream stream, int verbosity, int small);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
        private static extern int BZ2_bzDecompress(ref BZStream stream);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
        private static extern int BZ2_bzDecompressEnd(ref BZStream stream);
    }
}
