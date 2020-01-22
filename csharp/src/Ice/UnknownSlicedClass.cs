//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>
    /// Unknown sliced value holds an instance of an unknown Slice class type.
    /// </summary>
    public sealed class UnknownSlicedClass : AnyClass
    {
        /// <summary>
        /// Represents an instance of a Slice class type having the given Slice type.
        /// </summary>
        /// <param name="unknownTypeId">The Slice type ID of the unknown object.</param>
        public UnknownSlicedClass(string unknownTypeId)
        {
            _unknownTypeId = unknownTypeId;
        }
        protected override SlicedData? IceSlicedData => _slicedData;

        /// <summary>
        /// Returns the Slice type ID associated with this object.
        /// </summary>
        /// <returns>The type ID.</returns>
        public override string ice_id()
        {
            return _unknownTypeId;
        }

        public override void iceWrite(OutputStream ostr)
        {
            ostr.StartClass(_slicedData);
            ostr.EndClass();
        }

        protected override void IceRead(InputStream istr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            _slicedData = istr.SlicedData;
        }

        private string _unknownTypeId;
        private SlicedData? _slicedData;
    }
}
