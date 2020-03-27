//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// UnknownSlicedClass represents a fully sliced class instance. The local Ice runtime does not known any class
    /// described by this instance.
    /// </summary>
    public sealed class UnknownSlicedClass : AnyClass
    {
        /// <summary>
        /// Returns the Slice type ID associated with this sliced class instance.
        /// </summary>
        /// <value>The type ID.</value>
        public string? TypeId
        {
            get
            {
                // Can only be called after IceRead filled this instance. TypeId can be null for a slice with an
                // unresolved compact ID.
                Debug.Assert(_slicedData.HasValue && _slicedData.Value.Slices.Count > 0);
                return _slicedData.Value.Slices[0].TypeId;
            }
        }
        protected override SlicedData? IceSlicedData => _slicedData;
        private SlicedData? _slicedData;
        protected override void IceRead(InputStream istr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            _slicedData = istr.SlicedData;
            Debug.Assert(_slicedData.HasValue && _slicedData.Value.Slices.Count > 0);
        }
        protected override void IceWrite(OutputStream ostr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            Debug.Assert(_slicedData.HasValue); // Can only be called on an instance previously filled by IceRead.
            ostr.WriteSlicedData(_slicedData.Value);
        }
        internal UnknownSlicedClass()
        {
        }
    }
}
