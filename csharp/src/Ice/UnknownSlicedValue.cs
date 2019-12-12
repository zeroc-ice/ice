//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>
    /// Unknown sliced value holds an instance of an unknown Slice class type.
    /// </summary>
    public sealed class UnknownSlicedValue : Value
    {
        /// <summary>
        /// Represents an instance of a Slice class type having the given Slice type.
        /// </summary>
        /// <param name="unknownTypeId">The Slice type ID of the unknown object.</param>
        public UnknownSlicedValue(string unknownTypeId)
        {
            _unknownTypeId = unknownTypeId;
        }

        /// <summary>
        /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
        /// un-marshaling of the value, null is returned otherwise.
        /// </summary>
        /// <returns>The sliced data or null.</returns>
        public override SlicedData? ice_getSlicedData()
        {
            return _slicedData;
        }

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
            ostr.StartValue(_slicedData);
            ostr.EndValue();
        }

        public override void iceRead(InputStream istr)
        {
            istr.StartValue();
            _slicedData = istr.EndValue(true);
        }

        private string _unknownTypeId;
        private SlicedData? _slicedData;
    }
}
