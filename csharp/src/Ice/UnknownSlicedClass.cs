//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// UnknownSlicedClass represents a fully sliced class instance. The local Ice runtime does not known any class
    /// described by this instance.
    /// </summary>
    public sealed class UnknownSlicedClass : AnyClass
    {
        private SlicedData? _slicedData;

        /// <summary>
        /// Returns the Slice type ID associated with this object.
        /// </summary>
        /// <returns>The type ID.</returns>
        // TODO: rename this method
        public override string ice_id()
        {
            // Can only be called after IceRead filled this instance. TypeId can be null for a slice with an
            // unresolved compact ID.
            Debug.Assert(_slicedData.HasValue && _slicedData.Value.Slices.Count > 0);
            return _slicedData.Value.Slices[0].TypeId ?? "";
        }

        protected override SlicedData? IceSlicedData => _slicedData;
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
            if (!ostr.WriteSlicedData(_slicedData.Value))
            {
                // Could be for example attempting to marshal this instance into a compact-format encapsulation.
                throw new MarshalException(
                    $"Failed to marshal an {nameof(UnknownSlicedClass)} with type ID {ice_id()}");
            }
        }
        internal UnknownSlicedClass()
        {
        }
    }
}
