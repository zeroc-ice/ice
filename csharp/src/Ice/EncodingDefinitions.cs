// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    internal static class EncodingDefinitions
    {
        internal const long VarLongMinValue = -2_305_843_009_213_693_952; // -2^61
        internal const long VarLongMaxValue = 2_305_843_009_213_693_951; // 2^61 - 1
        internal const ulong VarULongMinValue = 0;
        internal const ulong VarULongMaxValue = 4_611_686_018_427_387_903; // 2^62 - 1

        internal const byte TaggedEndMarker = 0xFF;

        /// <summary>The first byte of each encoded class or exception slice.</summary>
        [Flags]
        internal enum SliceFlags : byte
        {
            /// <summary>The first 2 bits of SliceFlags represent the TypeIdKind, which can be extracted using
            /// GetTypeIdKind.</summary>
            TypeIdMask = 3,
            HasTaggedMembers = 4,
            HasIndirectionTable = 8,
            HasSliceSize = 16,
            IsLastSlice = 32
        }

        /// <summary>The first 2 bits of the SliceFlags.</summary>
        internal enum TypeIdKind : byte
        {
            None = 0,
            String = 1,
            Index = 2,
            CompactId11 = 3,
#pragma warning disable CA1069 // Enums values should not be duplicated
            Sequence20 = 3,
#pragma warning restore CA1069 // Enums values should not be duplicated
        }

        /// <summary>Each tagged parameter has a specific tag format. This tag format describes how the data is encoded
        /// and how it can be skipped by the unmarshaling code if the tagged parameter is present in the input stream
        /// but is not known to the receiver.</summary>
        internal enum TagFormat
        {
            F1 = 0,
            F2 = 1,
            F4 = 2,
            F8 = 3,
            Size = 4,
            VSize = 5,
            FSize = 6,
            Class = 7, // no longer written or accepted as of Ice 4.0

            /// <summary>VInt is a special value that is never marshaled and that means "one of F1, F2, F4 or F8".
            /// </summary>
            VInt = 8
        }

        internal enum ProxyType : byte
        {
            Null,
            Direct,
            Indirect,
            // TODO: more proxy types like fixed and relative
        }

        internal ref struct DirectProxyData
        {
            internal Identity Identity;

            internal Protocol Protocol; // Protocol is marshaled like an optional data member.

            internal Endpoint[] Endpoints;

            internal string[]? Location;
            internal Encoding? Encoding;

            internal InvocationMode? Ice1InvocationMode;

            internal string? Facet;

            internal DirectProxyData(InputStream istr)
            {
                var bitSequence = istr.ReadBitSequence(5);

                Identity = new Identity(istr);
                if (Identity.Name.Length == 0)
                {
                    throw new InvalidDataException("received proxy with empty name");
                }

                Protocol = bitSequence[0] ? (Protocol)istr.ReadByte() : Protocol.Ice2;
                Endpoints = istr.ReadEndpointArray(Protocol);

                Location = bitSequence[1] ? istr.ReadArray(1, InputStream.IceReaderIntoString) : null;
                Encoding = bitSequence[2] ? new Encoding(istr) : null;
                Ice1InvocationMode = bitSequence[3] ? istr.ReadByte().AsInvocationMode() : null;

                if (Protocol != Protocol.Ice1 && Ice1InvocationMode != null)
                {
                    throw new InvalidDataException(
                        $"received endpoint with protocol {Protocol.GetName()} and ice1 invocation mode");
                }

                Facet = bitSequence[4] ? istr.ReadString() : null;
            }
        }

        internal ref struct IndirectProxyData
        {
        }
    }

    internal static class SliceFlagsExtensions
    {
        /// <summary>Extracts the TypeIdKind of a SliceFlags value.</summary>
        /// <param name="sliceFlags">The SliceFlags value.</param>
        /// <returns>The TypeIdKind encoded in sliceFlags.</returns>
        internal static EncodingDefinitions.TypeIdKind GetTypeIdKind(this EncodingDefinitions.SliceFlags sliceFlags) =>
            (EncodingDefinitions.TypeIdKind)(sliceFlags & EncodingDefinitions.SliceFlags.TypeIdMask);
    }
}
