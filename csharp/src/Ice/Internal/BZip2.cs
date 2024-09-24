// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;
using System.Reflection;
using System.Runtime.ExceptionServices;
using System.Runtime.InteropServices;

namespace Ice.Internal;

// Map Bzip2 bz_stream struct to a C# struct for using with the Bzip2 low level API.
[StructLayout(LayoutKind.Sequential)]
internal struct BZStream(IntPtr nextOut, uint availOut)
{
    internal IntPtr NextIn = IntPtr.Zero;
    internal uint AvailIn = 0;
    internal uint TotalInLo32 = 0;
    internal uint TotalInHi32 = 0;

    internal IntPtr NextOut = nextOut;
    internal uint AvailOut = availOut;
    internal uint TotalOutLo32 = 0;
    internal uint TotalOutHi32 = 0;

    internal IntPtr State = IntPtr.Zero;
    internal IntPtr BzAlloc = IntPtr.Zero;
    internal IntPtr BzFree = IntPtr.Zero;
    internal IntPtr Opaque = IntPtr.Zero;
}

public static class BZip2
{
    private enum BzAction
    {
        Run = 0,
        Flush = 1,
        Finish = 2
    }

    private enum BzStatus
    {
        Ok = 0,
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

    public static bool isLoaded(Ice.Logger logger)
    {
        lock (_mutex)
        {
            if (_isLoaded is null)
            {
                // Register a delegate to load native libraries used by Ice assembly.
                NativeLibrary.SetDllImportResolver(Assembly.GetAssembly(typeof(BZip2))!, dllImportResolver);
                string libNames = string.Join(", ", getPlatformNativeLibraryNames()).TrimEnd();
                bool loaded = false;
                try
                {
                    BZ2_bzLibVersion();
                    loaded = true;
                }
                catch (EntryPointNotFoundException)
                {
                    Console.Error.WriteLine($"warning: found {libNames} but entry point BZ2_bzlibVersion is missing.");
                }
                catch (TypeLoadException)
                {
                    // Expected -- bzip2 lib not installed or not in PATH.
                }
                catch (BadImageFormatException)
                {
                    Console.Error.Write(
                        $"warning: {libNames} could not be loaded (likely due to 32/64-bit mismatch).");
                    if (IntPtr.Size == 8)
                    {
                        Console.Error.Write(
                            $" Make sure the directory containing the 64-bit {libNames} is in your PATH.");
                    }
                    Console.Error.WriteLine();
                }
                _isLoaded = loaded;
            }
            return _isLoaded.Value;
        }
    }

    private static readonly object _mutex = new object();
    private static bool? _isLoaded;

    internal static Buffer? compress(Buffer buf, int headerSize, int compressionLevel)
    {
        Debug.Assert(_isLoaded is true);
        // In the worst case, the compressed buffer will be 1% larger than the decompressed buffer plus 600 bytes
        // for the bzip2 header, plus 4 bytes for the decompressed size added by Ice protocol.
        int compressedLenMax = (int)((buf.size() * 1.01) + 600 + 4);

        byte[] compressed = new byte[compressedLenMax];

        // Prevent GC from moving the byte array, this allow to take the object address and pass it to bzip2 calls.
        var compressedHandle = GCHandle.Alloc(compressed, GCHandleType.Pinned);
        var bzStream = new BZStream(
            compressedHandle.AddrOfPinnedObject() + headerSize + 4,
            (uint)(compressed.Length - headerSize - 4));

        BzStatus rc;
        try
        {
            rc = (BzStatus)BZ2_bzCompressInit(ref bzStream, compressionLevel, 0, 0);
            if (rc != BzStatus.Ok)
            {
                throw new ProtocolException($"Bzip2 compress init failed {rc}");
            }

            // Compress the message body, but not the header.
            int decompressedLen = buf.size() - headerSize;
            byte[] decompressed = buf.b.rawBytes();
            var payload = new ArraySegment<byte>(decompressed, headerSize, decompressedLen);

            var payloadHandle = GCHandle.Alloc(payload.Array, GCHandleType.Pinned);
            bzStream.NextIn = payloadHandle.AddrOfPinnedObject() + payload.Offset;
            bzStream.AvailIn = (uint)payload.Count;
            Debug.Assert(bzStream.AvailIn > 0);

            do
            {
                rc = (BzStatus)BZ2_bzCompress(ref bzStream, (int)BzAction.Run);
            }
            while (rc == BzStatus.RunOk && bzStream.AvailIn > 0);
            payloadHandle.Free();

            if (rc != BzStatus.RunOk)
            {
                throw new ProtocolException($"Bzip2 compress run failed {rc}");
            }

            do
            {
                rc = (BzStatus)BZ2_bzCompress(ref bzStream, (int)BzAction.Finish);
            }
            while (rc == BzStatus.FinishOk);

            if (rc != BzStatus.StreamEnd)
            {
                throw new ProtocolException($"Bzip2 compress finish failed {rc}");
            }

            int compressedLen = compressed.Length - (int)bzStream.AvailOut;

            // Don't bother if the compressed data is larger than the decompressed data.
            if (compressedLen >= decompressedLen)
            {
                return null;
            }

            var r = new Buffer(compressed);
            r.resize(compressedLen, false);
            r.b.position(0);

            // Copy the header from the decompressed buffer to the compressed one.
            r.b.put(decompressed, 0, headerSize);

            // Add the size of the decompressed stream before the message body
            r.b.putInt(buf.size());

            r.b.position(compressedLen);
            return r;
        }
        finally
        {
            rc = (BzStatus)BZ2_bzCompressEnd(ref bzStream);
            Debug.Assert(rc == BzStatus.Ok);
            compressedHandle.Free();
        }
    }

    internal static Buffer decompress(Buffer buf, int headerSize, int messageSizeMax)
    {
        Debug.Assert(_isLoaded is true);

        buf.b.position(headerSize);
        int decompressedSize = buf.b.getInt();
        if (decompressedSize <= headerSize)
        {
            throw new MarshalException("compressed size <= header size");
        }
        if (decompressedSize > messageSizeMax)
        {
            Ex.throwMemoryLimitException(decompressedSize, messageSizeMax);
        }

        byte[] compressed = buf.b.rawBytes();

        byte[] decompressed = new byte[decompressedSize];
        // Prevent GC from moving the byte array, this allow to take the object address and pass it to bzip2 calls.
        var decompressedHandle = GCHandle.Alloc(decompressed, GCHandleType.Pinned);
        var compressedHandle = GCHandle.Alloc(compressed, GCHandleType.Pinned);
        var bzStream = new BZStream(
            decompressedHandle.AddrOfPinnedObject() + headerSize,
            (uint)(decompressedSize - headerSize));

        BzStatus rc;
        try
        {
            rc = (BzStatus)BZ2_bzDecompressInit(ref bzStream, 0, 0);
            if (rc != BzStatus.Ok)
            {
                throw new ProtocolException($"bzip2 decompression failed: {rc}");
            }

            bzStream.NextIn = compressedHandle.AddrOfPinnedObject() + headerSize + 4;
            bzStream.AvailIn = (uint)(compressed.Length - headerSize - 4);
            rc = (BzStatus)BZ2_bzDecompress(ref bzStream);
            if (rc != BzStatus.StreamEnd)
            {
                throw new ProtocolException($"bzip2 decompression failed: {rc}");
            }
        }
        finally
        {
            rc = (BzStatus)BZ2_bzDecompressEnd(ref bzStream);
            Debug.Assert(rc == BzStatus.Ok);
            decompressedHandle.Free();
            compressedHandle.Free();
        }
        compressed.AsSpan(0, headerSize).CopyTo(decompressed);
        return new Buffer(decompressed);
    }

    [DllImport("bzip2", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
    private static extern IntPtr BZ2_bzLibVersion();

    [DllImport("bzip2", EntryPoint = "BZ2_bzCompressInit", ExactSpelling = true)]
    private static extern int BZ2_bzCompressInit(
        ref BZStream stream,
        int blockSize100k,
        int verbosity,
        int workFactor);

    [DllImport("bzip2", EntryPoint = "BZ2_bzCompress", ExactSpelling = true)]
    private static extern int BZ2_bzCompress(ref BZStream stream, int action);

    [DllImport("bzip2", EntryPoint = "BZ2_bzCompressEnd", ExactSpelling = true)]
    private static extern int BZ2_bzCompressEnd(ref BZStream stream);

    [DllImport("bzip2", EntryPoint = "BZ2_bzDecompressInit", ExactSpelling = true)]
    private static extern int BZ2_bzDecompressInit(ref BZStream stream, int verbosity, int small);

    [DllImport("bzip2", EntryPoint = "BZ2_bzDecompress", ExactSpelling = true)]
    private static extern int BZ2_bzDecompress(ref BZStream stream);

    [DllImport("bzip2", EntryPoint = "BZ2_bzDecompressEnd", ExactSpelling = true)]
    private static extern int BZ2_bzDecompressEnd(ref BZStream stream);

    private static IntPtr dllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        DllNotFoundException? failure = null;
        Debug.Assert(libraryName == "bzip2");
        foreach (string name in getPlatformNativeLibraryNames())
        {
            try
            {
                return NativeLibrary.Load(name, assembly, searchPath);
            }
            catch (DllNotFoundException ex)
            {
                failure = ex;
            }
        }
        Debug.Assert(failure is not null);
        ExceptionDispatchInfo.Throw(failure);
        Debug.Assert(false);
        return IntPtr.Zero;
    }

    private static string[] getPlatformNativeLibraryNames()
    {
        if (AssemblyUtil.isWindows)
        {
            return ["bzip2.dll", "bzip2d.dll"];
        }
        else if (AssemblyUtil.isMacOS)
        {
            return ["libbz2.dylib"];
        }
        else
        {
            return ["libbz2.so.1.0", "libbz2.so.1", "libbz2.so"];
        }
    }
}
