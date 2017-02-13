// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        /// Determine the Slice type ID associated with this object.
        /// </summary>
        /// <returns>The type ID.</returns>
        public string getUnknownTypeId()
        {
            return _unknownTypeId;
        }

        public override void iceWrite(OutputStream ostr)
        {
            ostr.startValue(_slicedData);
            ostr.endValue();
        }

        public override void iceRead(InputStream istr)
        {
            istr.startValue();
            _slicedData = istr.endValue(true);
        }

        private string _unknownTypeId;
        private SlicedData _slicedData;
    }
}
