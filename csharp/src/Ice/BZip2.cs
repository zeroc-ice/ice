//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace IceInternal
{
    internal static class SafeNativeMethods
    {
        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr WindowsBZ2_bzlibVersion();

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzBuffToBuffCompress", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzBuffToBuffCompress(byte[] dest,
                                                                   ref int destLen,
                                                                   byte[] source,
                                                                   int sourceLen,
                                                                   int blockSize100k,
                                                                   int verbosity,
                                                                   int workFactor);

        [DllImport("bzip2.dll", EntryPoint = "BZ2_bzBuffToBuffDecompress", ExactSpelling = true)]
        internal static extern int WindowsBZ2_bzBuffToBuffDecompress(byte[] dest,
                                                                     ref int destLen,
                                                                     byte[] source,
                                                                     int sourceLen,
                                                                     int small,
                                                                     int verbosity);

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr UnixBZ2_1_bzlibVersion();

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzBuffToBuffCompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzBuffToBuffCompress(byte[] dest,
                                                                  ref int destLen,
                                                                  byte[] source,
                                                                  int sourceLen,
                                                                  int blockSize100k,
                                                                  int verbosity,
                                                                  int workFactor);

        [DllImport("libbz2.so.1", EntryPoint = "BZ2_bzBuffToBuffDecompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_1_bzBuffToBuffDecompress(byte[] dest,
                                                                    ref int destLen,
                                                                    byte[] source,
                                                                    int sourceLen,
                                                                    int small,
                                                                    int verbosity);

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr UnixBZ2_10_bzlibVersion();

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzBuffToBuffCompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzBuffToBuffCompress(byte[] dest,
                                                                   ref int destLen,
                                                                   byte[] source,
                                                                   int sourceLen,
                                                                   int blockSize100k,
                                                                   int verbosity,
                                                                   int workFactor);

        [DllImport("libbz2.so.1.0", EntryPoint = "BZ2_bzBuffToBuffDecompress", ExactSpelling = true)]
        internal static extern int UnixBZ2_10_bzBuffToBuffDecompress(byte[] dest,
                                                                     ref int destLen,
                                                                     byte[] source,
                                                                     int sourceLen,
                                                                     int small,
                                                                     int verbosity);

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzlibVersion", ExactSpelling = true)]
        internal static extern IntPtr MacOSBZ2_bzlibVersion();

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzBuffToBuffCompress", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzBuffToBuffCompress(byte[] dest,
                                                                 ref int destLen,
                                                                 byte[] source,
                                                                 int sourceLen,
                                                                 int blockSize100k,
                                                                 int verbosity,
                                                                 int workFactor);

        [DllImport("libbz2.dylib", EntryPoint = "BZ2_bzBuffToBuffDecompress", ExactSpelling = true)]
        internal static extern int MacOSBZ2_bzBuffToBuffDecompress(byte[] dest,
                                                                   ref int destLen,
                                                                   byte[] source,
                                                                   int sourceLen,
                                                                   int small,
                                                                   int verbosity);
    }

    internal delegate int CompressBuffer(byte[] dest,
                                         ref int destLen,
                                         byte[] source,
                                         int sourceLen,
                                         int blockSize100k,
                                         int verbosity,
                                         int workFactor);

    internal delegate int DecompressBuffer(byte[] dest,
                                           ref int destLen,
                                           byte[] source,
                                           int sourceLen,
                                           int small,
                                           int verbosity);
    public class BZip2
    {
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
                if (AssemblyUtil.IsWindows)
                {
                    _bzlibName = "bzip2.dll";
                    SafeNativeMethods.WindowsBZ2_bzlibVersion();
                }
                else if (AssemblyUtil.IsMacOS)
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
                Console.Error.WriteLine("warning: found " + _bzlibName + " but entry point BZ2_bzlibVersion is missing.");
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
                Console.Error.Write("warning: " + lib + " could not be loaded (likely due to 32/64-bit mismatch).");
                if (IntPtr.Size == 8)
                {
                    Console.Error.Write(" Make sure the directory containing the 64-bit " + lib + " is in your PATH.");
                }
                Console.Error.WriteLine();
            }

            if (AssemblyUtil.IsWindows)
            {
                _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                   int verbosity, int workFactor) =>
                    {
                        return SafeNativeMethods.WindowsBZ2_bzBuffToBuffCompress(dest, ref destLen, source, sourceLen,
                                                                                 blockSize100k, verbosity, workFactor);
                    };

                _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                     int verbosity) =>
                    {
                        return SafeNativeMethods.WindowsBZ2_bzBuffToBuffDecompress(dest, ref destLen, source, sourceLen,
                                                                                   small, verbosity);
                    };
            }
            else if (AssemblyUtil.IsMacOS)
            {
                _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                   int verbosity, int workFactor) =>
                    {
                        return SafeNativeMethods.MacOSBZ2_bzBuffToBuffCompress(dest, ref destLen, source, sourceLen,
                                                                               blockSize100k, verbosity, workFactor);
                    };

                _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                     int verbosity) =>
                    {
                        return SafeNativeMethods.MacOSBZ2_bzBuffToBuffDecompress(dest, ref destLen, source, sourceLen,
                                                                                 small, verbosity);
                    };
            }
            else
            {
                if (_bzlibName == "libbz2.so.1.0")
                {
                    _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                       int verbosity, int workFactor) =>
                        {
                            return SafeNativeMethods.UnixBZ2_10_bzBuffToBuffCompress(dest, ref destLen, source,
                                                                                     sourceLen, blockSize100k,
                                                                                     verbosity, workFactor);
                        };

                    _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                         int verbosity) =>
                        {
                            return SafeNativeMethods.UnixBZ2_10_bzBuffToBuffDecompress(dest, ref destLen, source,
                                                                                       sourceLen, small, verbosity);
                        };
                }
                else
                {
                    _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                       int verbosity, int workFactor) =>
                        {
                            return SafeNativeMethods.UnixBZ2_1_bzBuffToBuffCompress(dest, ref destLen, source,
                                                                                    sourceLen, blockSize100k, verbosity,
                                                                                    workFactor);
                        };

                    _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                         int verbosity) =>
                        {
                            return SafeNativeMethods.UnixBZ2_1_bzBuffToBuffDecompress(dest, ref destLen, source,
                                                                                      sourceLen, small, verbosity);
                        };
                }
            }
        }

        private static string GetBZ2Error(int error)
        {
            string rc;

            switch (error)
            {
                case BZ_SEQUENCE_ERROR:
                    {
                        rc = "BZ_SEQUENCE_ERROR";
                        break;
                    }
                case BZ_PARAM_ERROR:
                    {
                        rc = "BZ_PARAM_ERROR";
                        break;
                    }
                case BZ_MEM_ERROR:
                    {
                        rc = "BZ_MEM_ERROR";
                        break;
                    }
                case BZ_DATA_ERROR:
                    {
                        rc = "BZ_DATA_ERROR";
                        break;
                    }
                case BZ_DATA_ERROR_MAGIC:
                    {
                        rc = "BZ_DATA_ERROR_MAGIC";
                        break;
                    }
                case BZ_IO_ERROR:
                    {
                        rc = "BZ_IO_ERROR";
                        break;
                    }
                case BZ_UNEXPECTED_EOF:
                    {
                        rc = "BZ_UNEXPECTED_EOF";
                        break;
                    }
                case BZ_OUTBUFF_FULL:
                    {
                        rc = "BZ_OUTBUFF_FULL";
                        break;
                    }
                case BZ_CONFIG_ERROR:
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

        public static bool Supported() => _bzlibInstalled;

        public static Ice.OutputStream? Compress(Ice.OutputStream stream, int headerSize, int compressionLevel)
        {
            Debug.Assert(Supported());
            //
            // Compress the message body, but not the header.
            //
            int uncompressedLen = stream.Size - headerSize;

            // TODO: Avoid copy the data using bzip2 low level API, feeding segments instead of allocating
            // a new array to hold the whole input.
            byte[] data = stream.GetBytes(headerSize, uncompressedLen);

            int compressedLen = (int)((uncompressedLen * 1.01) + 600);
            byte[] compressed = new byte[compressedLen];

            int rc = _compressBuffer(compressed, ref compressedLen, data, uncompressedLen, compressionLevel, 0, 0);
            if (rc == BZ_OUTBUFF_FULL)
            {
                return null;
            }
            else if (rc < 0)
            {
                throw new Ice.CompressionException($"BZ2_bzBuffToBuffCompress failed {GetBZ2Error(rc)}");
            }

            //
            // Don't bother if the compressed data is larger than the
            // uncompressed data.
            //
            if (compressedLen >= uncompressedLen)
            {
                return null;
            }

            //
            // Copy the header from the uncompressed stream to the
            // compressed one.
            //
            byte[] header = stream.GetBytes(0, headerSize + 4);
            var r = new Ice.OutputStream(stream.Communicator, stream.Encoding, header);
            //
            // Add the size of the uncompressed stream before the
            // message body.
            //
            r.RewriteInt(stream.Size, new Ice.OutputStream.Position(0, headerSize));
            r.WriteSpan(compressed.AsSpan(0, compressedLen));
            return r;
        }

        public static Ice.InputStream Uncompress(Ice.InputStream compressed, int headerSize, int messageSizeMax)
        {
            Debug.Assert(Supported());
            int uncompressedSize = Ice.InputStream.ReadInt(compressed.Buffer.Slice(headerSize, 4));
            if (uncompressedSize <= headerSize)
            {
                throw new Ice.IllegalMessageSizeException("compressed size <= header size");
            }
            if (uncompressedSize > messageSizeMax)
            {
                Ex.ThrowMemoryLimitException(uncompressedSize, messageSizeMax);
            }

            // TODO this is not optimal we copy the compressed data to feed Bzip2 with a single array,
            // that doesn't include the uncompressed header, then we allocate an array to hold the
            // uncompressed data and copy the uncompressed data to the uncompressed stream after the
            // header.
            int compressedLen = compressed.Size - headerSize - 4;
            byte[] compressedData = new byte[compressedLen];
            compressed.Buffer.Slice(headerSize + 4).CopyTo(compressedData);
            int uncompressedLen = uncompressedSize - headerSize;

            byte[] uncompressedData = new byte[uncompressedLen];
            int rc = _decompressBuffer(uncompressedData, ref uncompressedLen, compressedData, compressedLen, 0, 0);
            if (rc < 0)
            {
                throw new Ice.CompressionException($"BZ2_bzBuffToBuffDecompress failed\n{GetBZ2Error(rc)}");
            }

            // Copy the header from the compressed buffer to the uncompressed one. We should
            // avoid the copy see comment above.
            byte[] uncompressed = new byte[uncompressedSize];
            compressed.Buffer.Slice(0, headerSize).CopyTo(uncompressed);
            uncompressedData.AsSpan().CopyTo(uncompressed.AsSpan(headerSize));
            return new Ice.InputStream(compressed.Communicator, compressed.Encoding, uncompressed);
        }

        private static readonly bool _bzlibInstalled;
        private static readonly string _bzlibName;

        private static readonly CompressBuffer _compressBuffer;
        private static readonly DecompressBuffer _decompressBuffer;

        private const int BZ_SEQUENCE_ERROR = -1;
        private const int BZ_PARAM_ERROR = -2;
        private const int BZ_MEM_ERROR = -3;
        private const int BZ_DATA_ERROR = -4;
        private const int BZ_DATA_ERROR_MAGIC = -5;
        private const int BZ_IO_ERROR = -6;
        private const int BZ_UNEXPECTED_EOF = -7;
        private const int BZ_OUTBUFF_FULL = -8;
        private const int BZ_CONFIG_ERROR = -9;
    }

}
