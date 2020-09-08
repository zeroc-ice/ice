//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.ComponentModel;
using System.Diagnostics;

namespace ZeroC.Ice
{
    public abstract class AnyClass
    {
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<AnyClass> IceReader =
            istr => istr.ReadClass<AnyClass>(formalTypeId: null);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<AnyClass?> IceReaderIntoNullable =
            istr => istr.ReadNullableClass<AnyClass>(formalTypeId: null);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<AnyClass> IceWriter = (ostr, value) => ostr.WriteClass(value, null);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<AnyClass?> IceWriterFromNullable =
            (ostr, value) => ostr.WriteNullableClass(value, null);

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

        // See InputStream
        protected abstract void IceRead(InputStream istr, bool firtSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);

        // See OutputStream.
        protected abstract void IceWrite(OutputStream ostr, bool firstSlice);
        internal void Write(OutputStream ostr) => IceWrite(ostr, true);
    }

    public static class AnyClassExtensions
    {
        /// <summary>During unmarshaling, Ice can slice off derived slices that it does not know how to read, and it can
        /// optionally preserve those "unknown" slices. See the Slice preserve metadata directive and class
        /// <see cref="UnknownSlicedClass"/>.</summary>
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this AnyClass obj) => obj.SlicedData;
    }
}
