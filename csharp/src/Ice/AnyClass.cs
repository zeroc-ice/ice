// Copyright (c) ZeroC, Inc. All rights reserved.

using System.ComponentModel;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>The base class for classes defined in Slice.</summary>
    public abstract class AnyClass
    {
        /// <summary>An InputStream reader used to read non nullable class instances.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<AnyClass> IceReader =
            istr => istr.ReadClass<AnyClass>(formalTypeId: null);

        /// <summary>An InputStream reader used to read nullable class instances.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<AnyClass?> IceReaderIntoNullable =
            istr => istr.ReadNullableClass<AnyClass>(formalTypeId: null);

        /// <summary>An OutputStream writer used to write non nullable class instances.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<AnyClass> IceWriter = (ostr, value) => ostr.WriteClass(value, null);

        /// <summary>An OutputStream writer used to write nullable class instances.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<AnyClass?> IceWriterFromNullable =
            (ostr, value) => ostr.WriteNullableClass(value, null);

        /// <summary>Returns the sliced data if the class has a preserved-slice base class and has been sliced during
        /// unmarshaling, otherwise <c>null</c>.</summary>
        protected virtual SlicedData? IceSlicedData
        {
            get => null;
            set => Debug.Assert(false);
        }

        internal SlicedData? SlicedData
        {
            get => IceSlicedData;
            set => IceSlicedData = value;
        }

        /// <summary>Unmarshals the current object by reading its data members from the <see cref="InputStream"/>.
        /// </summary>
        /// <param name="istr">The stream to read from.</param>
        /// <param name="firstSlice"><c>True</c> if this is the first Slice otherwise<c>False</c>.</param>
        protected abstract void IceRead(InputStream istr, bool firstSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);

        /// <summary>Marshals the current object by writing its data to from the <see cref="OutputStream"/>.</summary>
        /// <param name="ostr">The stream to write to.</param>
        /// <param name="firstSlice"><c>True</c> if this is the first Slice otherwise<c>False</c>.</param>
        protected abstract void IceWrite(OutputStream ostr, bool firstSlice);
        internal void Write(OutputStream ostr) => IceWrite(ostr, true);
    }

    /// <summary>Provides public extensions methods for AnyClass instances.</summary>
    public static class AnyClassExtensions
    {
        /// <summary>During unmarshaling, Ice can slice off derived slices that it does not know how to read, and it can
        /// optionally preserve those "unknown" slices. See the Slice preserve metadata directive and class
        /// <see cref="UnknownSlicedClass"/>.</summary>
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this AnyClass obj) => obj.SlicedData;
    }
}
