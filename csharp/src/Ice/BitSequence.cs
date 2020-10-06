// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Text;

namespace ZeroC.Ice
{
    /// <summary>Presents one or two <see cref="Span{T}"/> of bytes as a continuous sequence of bits.</summary>
    // BitSequence must be a ref struct as it holds two Span<byte>, and Span<T> is a ref struct.
    public readonly ref struct BitSequence
    {
        /// <summary>The first underlying span of bytes.</summary>
        public readonly Span<byte> FirstSpan;

        /// <summary>Gets or sets a bit in the bit sequence.</summary>
        /// <param name="index">The index of the bit to get or set. Must be between 0 and Length - 1.</param>
        /// <value>True when the bit is set, false when the bit is unset.</value>
        public bool this[int index]
        {
            get => (GetByteAsSpan(index)[0] & (1 << (index & 0x7))) != 0;

            set
            {
                Span<byte> span = GetByteAsSpan(index);
                if (value)
                {
                    span[0] = (byte)(span[0] | (1 << (index & 0x7)));
                }
                else
                {
                    span[0] = (byte)(span[0] & ~(1 << (index & 0x7)));
                }
            }
        }

        /// <summary>The length of the bit sequence.</summary>
        public int Length => (FirstSpan.Length + SecondSpan.Length) * 8;

        /// <summary>The second underlying span of bytes.</summary>
        public readonly Span<byte> SecondSpan;

        /// <summary>Constructs a bit sequence over one or two spans of bytes.</summary>
        public BitSequence(Span<byte> firstSpan, Span<byte> secondSpan = default)
        {
            FirstSpan = firstSpan;
            SecondSpan = secondSpan;
        }

        /// <inheritdoc/>
        public override string ToString() =>
            new ReadOnlyBitSequence(FirstSpan).ToString() + "\n" + new ReadOnlyBitSequence(SecondSpan).ToString();

        private Span<byte> GetByteAsSpan(int index)
        {
            int byteIndex = index >> 3;
            try
            {
                return byteIndex < FirstSpan.Length ? FirstSpan.Slice(byteIndex, 1) :
                    SecondSpan.Slice(byteIndex - FirstSpan.Length, 1);
            }
            catch (ArgumentOutOfRangeException)
            {
                throw new IndexOutOfRangeException($"{index} is outside the range 0..{Length - 1}");
            }
        }
    }

    /// <summary>Presents a <see cref="ReadOnlySpan{T}"/> of bytes as a continuous read-only sequence of bits.</summary>
    public readonly ref struct ReadOnlyBitSequence
    {
        /// <summary>Gets a bit in the bit sequence.</summary>
        /// <param name="index">The index of the bit to get. Must be between 0 and Length - 1.</param>
        /// <value>True when the bit is set, false when the bit is unset.</value>
        public bool this[int index] => (Span[index >> 3] & (1 << (index & 0x7))) != 0;

        /// <summary>The length of the bit sequence.</summary>
        public int Length => Span.Length * 8;

        /// <summary>The underlying span of bytes.</summary>
        public readonly ReadOnlySpan<byte> Span;

        /// <summary>Constructs a read-only bit sequence over a read-only span of bytes.</summary>
        public ReadOnlyBitSequence(ReadOnlySpan<byte> span) => Span = span;

        /// <inheritdoc/>
        public override string ToString()
        {
            var sb = new StringBuilder();
            foreach (byte b in Span)
            {
                sb.Append(Convert.ToString(b, 2).PadLeft(8, '0')).Append(' ');
            }
            return sb.ToString();
        }
    }
}
