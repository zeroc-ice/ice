// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Diagnostics;
    using System.Runtime.InteropServices;

    internal static class SafeNativeMethods
    {
        [DllImport("bzip2.dll", EntryPoint="BZ2_bzlibVersion")]
        internal static extern IntPtr windowsBZ2_bzlibVersion();

        [DllImport("bzip2.dll", EntryPoint="BZ2_bzBuffToBuffCompress")]
        internal static extern int windowsBZ2_bzBuffToBuffCompress(byte[] dest,
                                                                   ref int destLen,
                                                                   byte[] source,
                                                                   int sourceLen,
                                                                   int blockSize100k,
                                                                   int verbosity,
                                                                   int workFactor);

        [DllImport("bzip2.dll", EntryPoint="BZ2_bzBuffToBuffDecompress")]
        internal static extern int windowsBZ2_bzBuffToBuffDecompress(byte[] dest,
                                                                     ref int destLen,
                                                                     byte[] source,
                                                                     int sourceLen,
                                                                     int small,
                                                                     int verbosity);

        [DllImport("libbz2.so", EntryPoint="BZ2_bzlibVersion")]
        internal static extern IntPtr unixBZ2_bzlibVersion();

        [DllImport("libbz2.so", EntryPoint="BZ2_bzBuffToBuffCompress")]
        internal static extern int unixBZ2_bzBuffToBuffCompress(byte[] dest,
                                                                ref int destLen,
                                                                byte[] source,
                                                                int sourceLen,
                                                                int blockSize100k,
                                                                int verbosity,
                                                                int workFactor);

        [DllImport("libbz2.so", EntryPoint="BZ2_bzBuffToBuffDecompress")]
        internal static extern int unixBZ2_bzBuffToBuffDecompress(byte[] dest,
                                                                  ref int destLen,
                                                                  byte[] source,
                                                                  int sourceLen,
                                                                  int small,
                                                                  int verbosity);

        [DllImport("libbz2.dylib", EntryPoint="BZ2_bzlibVersion")]
        internal static extern IntPtr macOSBZ2_bzlibVersion();

        [DllImport("libbz2.dylib", EntryPoint="BZ2_bzBuffToBuffCompress")]
        internal static extern int macOSBZ2_bzBuffToBuffCompress(byte[] dest,
                                                                 ref int destLen,
                                                                 byte[] source,
                                                                 int sourceLen,
                                                                 int blockSize100k,
                                                                 int verbosity,
                                                                 int workFactor);

        [DllImport("libbz2.dylib", EntryPoint="BZ2_bzBuffToBuffDecompress")]
        internal static extern int macOSBZ2_bzBuffToBuffDecompress(byte[] dest,
                                                                   ref int destLen,
                                                                   byte[] source,
                                                                   int sourceLen,
                                                                   int small,
                                                                   int verbosity);
    }

    delegate int CompressBuffer(byte[] dest,
                                ref int destLen,
                                byte[] source,
                                int sourceLen,
                                int blockSize100k,
                                int verbosity,
                                int workFactor);

    delegate int DecompressBuffer(byte[] dest,
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

            //
            // We are setting the library name here because, under Mono, we don't know the exact library name.
            // In addition, the FileName member of the BadImageFormatException is the empty string, even though
            // it should provide the name of the library.
            //
            try
            {
                if(AssemblyUtil.isWindows)
                {
                    SafeNativeMethods.windowsBZ2_bzlibVersion();
                }
                else if(AssemblyUtil.isMacOS)
                {
                    SafeNativeMethods.macOSBZ2_bzlibVersion();
                }
                else
                {
                    SafeNativeMethods.unixBZ2_bzlibVersion();
                }
                _bzlibInstalled = true;
            }
            catch(DllNotFoundException)
            {
                // Expected -- bzip2 lib not installed or not in PATH.
            }
            catch(EntryPointNotFoundException)
            {
                Console.Error.WriteLine("warning: found bzip2.dll but entry point BZ2_bzlibVersion is missing.");
            }
            catch(BadImageFormatException ex)
            {
                string lib = AssemblyUtil.isWindows ? "bzip2.dll" : "bzip2.so";
                if(!String.IsNullOrEmpty(ex.FileName))
                {
                    lib = ex.FileName; // Future-proof: we'll do the right thing if the FileName member is non-empty.
                }
                Console.Error.Write("warning: " + lib + " could not be loaded (likely due to 32/64-bit mismatch).");
                if(IntPtr.Size == 8)
                {
                    Console.Error.Write(" Make sure the directory containing the 64-bit " + lib + " is in your PATH.");
                }
                Console.Error.WriteLine();
            }

            if(AssemblyUtil.isWindows)
            {
                _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                   int verbosity, int workFactor) =>
                    {
                        return SafeNativeMethods.windowsBZ2_bzBuffToBuffCompress(dest, ref destLen, source, sourceLen,
                                                                                 blockSize100k, verbosity, workFactor);
                    };

                _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                     int verbosity) =>
                    {
                        return SafeNativeMethods.windowsBZ2_bzBuffToBuffDecompress(dest, ref destLen, source, sourceLen,
                                                                                   small, verbosity);
                    };
            }
            else if(AssemblyUtil.isMacOS)
            {
                _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                   int verbosity, int workFactor) =>
                    {
                        return SafeNativeMethods.macOSBZ2_bzBuffToBuffCompress(dest, ref destLen, source, sourceLen,
                                                                               blockSize100k, verbosity, workFactor);
                    };

                _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                     int verbosity) =>
                    {
                        return SafeNativeMethods.macOSBZ2_bzBuffToBuffDecompress(dest, ref destLen, source, sourceLen,
                                                                                 small, verbosity);
                    };
            }
            else
            {
                _compressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int blockSize100k,
                                   int verbosity, int workFactor) =>
                    {
                        return SafeNativeMethods.unixBZ2_bzBuffToBuffCompress(dest, ref destLen, source, sourceLen,
                                                                              blockSize100k, verbosity, workFactor);
                    };

                _decompressBuffer = (byte[] dest, ref int destLen, byte[] source, int sourceLen, int small,
                                     int verbosity) =>
                    {
                        return SafeNativeMethods.unixBZ2_bzBuffToBuffDecompress(dest, ref destLen, source, sourceLen,
                                                                                   small, verbosity);
                    };
            }
        }

        static string getBZ2Error(int error)
        {
            string rc;

            switch(error)
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

        public static bool supported()
        {
            return _bzlibInstalled;
        }

        public static Buffer compress(Buffer buf, int headerSize, int compressionLevel)
        {
            Debug.Assert(supported());
            //
            // Compress the message body, but not the header.
            //
            int uncompressedLen = buf.size() - headerSize;
            byte[] data = buf.b.rawBytes(headerSize, uncompressedLen);
            int compressedLen = (int)(uncompressedLen * 1.01 + 600);
            byte[] compressed = new byte[compressedLen];

            int rc = _compressBuffer(compressed, ref compressedLen, data, uncompressedLen, compressionLevel, 0, 0);
            if(rc == BZ_OUTBUFF_FULL)
            {
                return null;
            }
            else if(rc < 0)
            {
                Ice.CompressionException ex = new Ice.CompressionException("BZ2_bzBuffToBuffCompress failed");
                ex.reason = getBZ2Error(rc);
                throw ex;
            }

            //
            // Don't bother if the compressed data is larger than the
            // uncompressed data.
            //
            if(compressedLen >= uncompressedLen)
            {
                return null;
            }

            Buffer r = new Buffer();
            r.resize(headerSize + 4 + compressedLen, false);
            r.b.position(0);

            //
            // Copy the header from the uncompressed stream to the
            // compressed one.
            //
            r.b.put(buf.b.rawBytes(0, headerSize));

            //
            // Add the size of the uncompressed stream before the
            // message body.
            //
            r.b.putInt(buf.size());

            //
            // Add the compressed message body.
            //
            r.b.put(compressed, 0, compressedLen);

            return r;
        }

        public static Buffer uncompress(Buffer buf, int headerSize, int messageSizeMax)
        {
            Debug.Assert(supported());

            buf.b.position(headerSize);
            int uncompressedSize = buf.b.getInt();
            if(uncompressedSize <= headerSize)
            {
                throw new Ice.IllegalMessageSizeException("compressed size <= header size");
            }
            if(uncompressedSize > messageSizeMax)
            {
                IceInternal.Ex.throwMemoryLimitException(uncompressedSize, messageSizeMax);
            }

            int compressedLen = buf.size() - headerSize - 4;
            byte[] compressed = buf.b.rawBytes(headerSize + 4, compressedLen);
            int uncompressedLen = uncompressedSize - headerSize;

            byte[] uncompressed = new byte[uncompressedLen];
            int rc = _decompressBuffer(uncompressed, ref uncompressedLen, compressed, compressedLen, 0, 0);
            if(rc < 0)
            {
                Ice.CompressionException ex = new Ice.CompressionException("BZ2_bzBuffToBuffDecompress failed");
                ex.reason = getBZ2Error(rc);
                throw ex;
            }

            Buffer r = new Buffer();
            r.resize(uncompressedSize, false);

            //
            // Copy the header from the compressed buffer to the uncompressed one.
            //
            r.b.position(0);
            r.b.put(buf.b.rawBytes(), 0, headerSize);
            r.b.put(uncompressed);

            return r;
        }

        private static bool _bzlibInstalled;

        private static CompressBuffer _compressBuffer;
        private static DecompressBuffer _decompressBuffer;

        const int BZ_SEQUENCE_ERROR = -1;
        const int BZ_PARAM_ERROR = -2;
        const int BZ_MEM_ERROR = -3;
        const int BZ_DATA_ERROR = -4;
        const int BZ_DATA_ERROR_MAGIC = -5;
        const int BZ_IO_ERROR = -6;
        const int BZ_UNEXPECTED_EOF = -7;
        const int BZ_OUTBUFF_FULL = -8;
        const int BZ_CONFIG_ERROR = -9;
    }

}
