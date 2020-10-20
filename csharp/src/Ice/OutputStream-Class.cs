// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

namespace ZeroC.Ice
{
    // This partial class provides the class/exception marshaling logic.

    public sealed partial class OutputStream
    {
        /// <summary>Marks the end of a slice for a class instance or user exception. This is an Ice-internal method
        /// marked public because it's called by the generated code.</summary>
        /// <param name="lastSlice">True when it's the last (least derived) slice of the instance; otherwise, false.
        /// </param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice(bool lastSlice)
        {
            Debug.Assert(InEncapsulation && _current.InstanceType != InstanceType.None);

            if (lastSlice)
            {
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.IsLastSlice;
            }

            // Writes the tagged end marker if some tagged members were encoded. Note that tagged members are encoded
            // before the indirection table and are included in the slice size.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                WriteByte(EncodingDefinitions.TaggedEndMarker);
            }

            // Writes the slice size if necessary.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                if (OldEncoding)
                {
                    // Size includes the size length.
                    RewriteFixedLengthSize11(Distance(_current.SliceSizePos), _current.SliceSizePos);
                }
                else
                {
                    // Size does not include the size length.
                    RewriteFixedLengthSize20(Distance(_current.SliceSizePos) - DefaultSizeLength,
                        _current.SliceSizePos);
                }
            }

            if (_current.IndirectionTable?.Count > 0)
            {
                Debug.Assert(_format == FormatType.Sliced);
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasIndirectionTable;

                WriteSize(_current.IndirectionTable.Count);
                foreach (AnyClass v in _current.IndirectionTable)
                {
                    // We cannot use formal type optimization for instances written inline in an indirection table,
                    // as the slice may not be known/unmarshaled by the recipient - therefore the formal type of the
                    // data member is not known.
                    WriteInstance(v, formalTypeId: null);
                }
                _current.IndirectionTable.Clear();
                _current.IndirectionMap?.Clear(); // IndirectionMap is null when writing SlicedData.
            }

            // Update SliceFlags in case they were updated.
            RewriteByte((byte)_current.SliceFlags, _current.SliceFlagsPos);
        }

        /// <summary>Starts writing the first slice of a class or exception instance. This is an Ice-internal method
        /// marked public because it's called by the generated code.</summary>
        /// <param name="allTypeIds">The type IDs of all slices of the instance (excluding sliced-off slices), from
        /// most derived to least derived.</param>
        /// <param name="slicedData">The preserved sliced-off slices, if any.</param>
        /// <param name="errorMessage">The exception error message (provided only by exceptions).</param>
        /// <param name="origin">The exception origin (provided only by exceptions).</param>
        /// <param name="compactId">The compact ID of this slice, if any. Used by the 1.1 encoding.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartFirstSlice(
            string[] allTypeIds,
            SlicedData? slicedData = null,
            string? errorMessage = null,
            RemoteExceptionOrigin? origin = null,
            int? compactId = null)
        {
            Debug.Assert(InEncapsulation && _current.InstanceType != InstanceType.None);

            if (slicedData is SlicedData slicedDataValue)
            {
                bool firstSliceWritten = false;
                try
                {
                    // WriteSlicedData calls IceStartFirstSlice.
                    WriteSlicedData(slicedDataValue, allTypeIds, errorMessage, origin);
                    firstSliceWritten = true;
                }
                catch (NotSupportedException)
                {
                    // For some reason we could not remarshal the sliced data; firstSliceWritten remains false.
                }
                if (firstSliceWritten)
                {
                    IceStartNextSlice(allTypeIds[0], compactId);
                    return;
                }
                // else keep going, we're still writing the first slice and we're ignoring slicedData.
            }

            if (OldEncoding && _format == FormatType.Sliced)
            {
                // With the 1.1 encoding in sliced format, all the slice headers are the same.
                IceStartNextSlice(allTypeIds[0], compactId);
            }
            else
            {
                _current.SliceFlags = default;
                _current.SliceFlagsPos = _tail;
                WriteByte(0); // Placeholder for the slice flags

                if (OldEncoding)
                {
                    WriteTypeId11(allTypeIds[0], compactId);
                }
                else
                {
                    WriteTypeId20(allTypeIds, errorMessage, origin);
                    if (_format == FormatType.Sliced)
                    {
                        // Encode the slice size if using the sliced format.
                        _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasSliceSize;
                        _current.SliceSizePos = StartFixedLengthSize();
                    }
                }
            }
        }

        /// <summary>Starts writing the next (i.e. not first) slice of a class or exception instance. This is an
        /// Ice-internal method marked public because it's called by the generated code.</summary>
        /// <param name="typeId">The type ID of this slice.</param>
        /// <param name="compactId">The compact ID of this slice, if any. Used by the 1.1 encoding.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartNextSlice(string typeId, int? compactId = null)
        {
            Debug.Assert(InEncapsulation && _current.InstanceType != InstanceType.None);

            _current.SliceFlags = default;
            _current.SliceFlagsPos = _tail;
            WriteByte(0); // Placeholder for the slice flags

            if (OldEncoding && _format == FormatType.Sliced)
            {
                WriteTypeId11(typeId, compactId);
            }

            if (_format == FormatType.Sliced)
            {
                // Encode the slice size if using the sliced format.
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasSliceSize;
                _current.SliceSizePos = StartFixedLengthSize();
            }
        }

        /// <summary>Writes a class instance to the stream.</summary>
        /// <param name="v">The class instance to write. This instance cannot be null.</param>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being written.
        /// Use null when the type of the parameter/data member is AnyClass.</param>
        public void WriteClass(AnyClass v, string? formalTypeId)
        {
            Debug.Assert(InEncapsulation);

            if (_current.InstanceType != InstanceType.None && _format == FormatType.Sliced)
            {
                // If writing an instance within a slice and using the sliced format, write an index of that slice's
                // indirection table.
                if (_current.IndirectionMap != null && _current.IndirectionMap.TryGetValue(v, out int index))
                {
                    // Found, index is position in indirection table + 1
                    Debug.Assert(index > 0);
                }
                else
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    _current.IndirectionMap ??= new Dictionary<AnyClass, int>();
                    _current.IndirectionTable.Add(v);
                    index = _current.IndirectionTable.Count; // Position + 1 (0 is reserved for null)
                    _current.IndirectionMap.Add(v, index);
                }
                WriteSize(index);
            }
            else
            {
                WriteInstance(v, formalTypeId); // Writes the instance or a reference if already marshaled.
            }
        }

        /// <summary>Writes a remote exception to the stream.</summary>
        /// <param name="v">The remote exception to write.</param>
        public void WriteException(RemoteException v)
        {
            Debug.Assert(InEncapsulation && _current.InstanceType == InstanceType.None);
            Debug.Assert(_format == FormatType.Sliced);
            _current.InstanceType = InstanceType.Exception;
            v.Write(this);
            _current = default;
        }

        /// <summary>Writes a class instance to the stream, or null.</summary>
        /// <param name="v">The class instance to write, or null.</param>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being written.
        /// Use null when the type of the parameter/data member is AnyClass.</param>
        public void WriteNullableClass(AnyClass? v, string? formalTypeId)
        {
            Debug.Assert(InEncapsulation);
            if (v == null)
            {
                WriteSize(0);
            }
            else
            {
                WriteClass(v, formalTypeId);
            }
        }

        /// <summary>Writes sliced-off slices to the stream.</summary>
        /// <param name="slicedData">The sliced-off slices to write.</param>
        /// <param name="baseTypeIds">The type IDs of less derived slices.</param>
        /// <param name="errorMessage">For exceptions, the exception's error message.</param>
        /// <param name="origin">For exceptions, the exception's origin.</param>
        internal void WriteSlicedData(
            SlicedData slicedData,
            string[] baseTypeIds,
            string? errorMessage = null,
            RemoteExceptionOrigin? origin = null)
        {
            Debug.Assert(_current.InstanceType != InstanceType.None);

            // We only remarshal preserved slices if we are using the sliced format. Otherwise, we ignore the preserved
            // slices, which essentially "slices" the instance into the most-derived type known by the sender.
            if (_format != FormatType.Sliced)
            {
                throw new NotSupportedException($"cannot write sliced data into payload using {_format} format");
            }
            if (Encoding != slicedData.Encoding)
            {
                throw new NotSupportedException(@$"cannot write sliced data encoded with encoding {slicedData.Encoding
                    } into payload encoded with encoding {Encoding}");
            }

            bool firstSliceWithNewEncoding = !OldEncoding;

            for (int i = 0; i < slicedData.Slices.Count; ++i)
            {
                SliceInfo sliceInfo = slicedData.Slices[i];

                if (firstSliceWithNewEncoding)
                {
                    firstSliceWithNewEncoding = false;

                    string[] allTypeIds = new string[slicedData.Slices.Count + baseTypeIds.Length];
                    for (int j = 0; j < slicedData.Slices.Count; ++j)
                    {
                        allTypeIds[j] = slicedData.Slices[j].TypeId;
                    }
                    if (baseTypeIds.Length > 0)
                    {
                        baseTypeIds.CopyTo(allTypeIds, slicedData.Slices.Count);
                    }

                    IceStartFirstSlice(allTypeIds, errorMessage: errorMessage, origin: origin);
                }
                else
                {
                    // With the 1.1 encoding in sliced format, IceStartNextSlice is the same as IceStartFirstSlice.
                    IceStartNextSlice(sliceInfo.TypeId, sliceInfo.CompactId);
                }

                // Writes the bytes associated with this slice.
                WriteByteSpan(sliceInfo.Bytes.Span);

                if (sliceInfo.HasTaggedMembers)
                {
                    _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTaggedMembers;
                }

                // Make sure to also re-write the instance indirection table.
                // These instances will be marshaled (and assigned instance IDs) in IceEndSlice.
                if (sliceInfo.Instances.Count > 0)
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    Debug.Assert(_current.IndirectionTable.Count == 0);
                    _current.IndirectionTable.AddRange(sliceInfo.Instances);
                }
                IceEndSlice(lastSlice: baseTypeIds.Length == 0 && (i == slicedData.Slices.Count - 1));
            }
        }

        /// <summary>Registers or looks up a type ID in the the _typeIdMap.</summary>
        /// <param name="typeId">The type ID to register or lookup.</param>
        /// <returns>The index in _typeIdMap if this type ID was previously registered; otherwise, -1.</returns>
        private int RegisterTypeId(string typeId)
        {
            _typeIdMap ??= new Dictionary<string, int>();

            if (_typeIdMap.TryGetValue(typeId, out int index))
            {
                return index;
            }
            else
            {
                index = _typeIdMap.Count + 1;
                _typeIdMap.Add(typeId, index);
                return -1;
            }
        }

        /// <summary>Writes this class instance inline if not previously marshaled, otherwise just write its instance
        /// ID.</summary>
        /// <param name="v">The class instance.</param>
        /// <param name="formalTypeId">The type ID of the formal parameter or data member being marshaled.</param>
        private void WriteInstance(AnyClass v, string? formalTypeId)
        {
            // If the instance was already marshaled, just write its instance ID.
            if (_instanceMap != null && _instanceMap.TryGetValue(v, out int instanceId))
            {
                WriteSize(instanceId);
            }
            else
            {
                _instanceMap ??= new Dictionary<AnyClass, int>();

                // We haven't seen this instance previously, so we create a new instance ID and insert the instance
                // and its ID in the marshaled map, before writing the instance inline.
                // The instance IDs start at 2 (0 means null and 1 means the instance is written immediately after).
                instanceId = _instanceMap.Count + 2;
                _instanceMap.Add(v, instanceId);

                WriteSize(1); // Class instance marker.

                // Save _current in case we're writing a nested instance.
                InstanceData previousCurrent = _current;
                _current = default;
                _current.InstanceType = InstanceType.Class;
                _current.FormalTypeId20 = formalTypeId;

                v.Write(this);

                // Restore previous _current.
                _current = previousCurrent;
            }
        }

        /// <summary>Writes the type ID or compact ID immediately after the slice flags byte, and updates the slice
        /// flags byte as needed.</summary>
        /// <param name="typeId">The type ID of the current slice.</param>
        /// <param name="compactId">The compact ID of the current slice.</param>
        private void WriteTypeId11(string typeId, int? compactId)
        {
            Debug.Assert(_current.InstanceType != InstanceType.None);

            EncodingDefinitions.TypeIdKind typeIdKind = EncodingDefinitions.TypeIdKind.None;

            if (_current.InstanceType == InstanceType.Class)
            {
                if (compactId is int compactIdValue)
                {
                    typeIdKind = EncodingDefinitions.TypeIdKind.CompactId11;
                    WriteSize(compactIdValue);
                }
                else
                {
                    int index = RegisterTypeId(typeId);
                    if (index < 0)
                    {
                        typeIdKind = EncodingDefinitions.TypeIdKind.String;
                        WriteString(typeId);
                    }
                    else
                    {
                        typeIdKind = EncodingDefinitions.TypeIdKind.Index;
                        WriteSize(index);
                    }
                }
            }
            else
            {
                Debug.Assert(compactId == null);
                // With the 1.1 encoding, we always write a string and don't set a type ID kind in SliceFlags.
                WriteString(typeId);
            }

            _current.SliceFlags |= (EncodingDefinitions.SliceFlags)typeIdKind;
        }

        /// <summary>Writes the type ID or type ID sequence immediately after the slice flags byte of the first slice,
        /// and updates the slice flags byte as needed. Applies formal type optimization (class only), if possible.
        /// </summary>
        /// <param name="allTypeIds">The type IDs of all slices of this class or exception instance.</param>
        /// <param name="errorMessage">The exception's error message. Provided only for exceptions.</param>
        /// <param name="origin">The exception's origin. Provided only for exceptions.</param>
        private void WriteTypeId20(string[] allTypeIds, string? errorMessage, RemoteExceptionOrigin? origin)
        {
            Debug.Assert(_current.InstanceType != InstanceType.None);

            EncodingDefinitions.TypeIdKind typeIdKind = EncodingDefinitions.TypeIdKind.None;

            if (_current.InstanceType == InstanceType.Class)
            {
                string typeId = allTypeIds[0];
                if (typeId != _current.FormalTypeId20)
                {
                    int index = RegisterTypeId(typeId);
                    if (index < 0)
                    {
                        if (_format == FormatType.Sliced)
                        {
                            typeIdKind = EncodingDefinitions.TypeIdKind.Sequence20;
                            WriteSequence(allTypeIds, IceWriterFromString);
                        }
                        else
                        {
                            typeIdKind = EncodingDefinitions.TypeIdKind.String;
                            WriteString(typeId);
                        }
                    }
                    else
                    {
                        typeIdKind = EncodingDefinitions.TypeIdKind.Index;
                        WriteSize(index);
                    }
                }
                // else, don't write anything (formal type optimization)
            }
            else
            {
                typeIdKind = EncodingDefinitions.TypeIdKind.Sequence20;
                WriteSequence(allTypeIds, IceWriterFromString);

                Debug.Assert(errorMessage != null);
                WriteString(errorMessage);
                Debug.Assert(origin != null);
                origin.Value.IceWrite(this);
            }

            _current.SliceFlags |= (EncodingDefinitions.SliceFlags)typeIdKind;
        }

        private struct InstanceData
        {
            // The following fields are used and reused for all the slices of a class or exception instance.

            // (Class only) The type ID associated with the formal type of the parameter or data member being written.
            // We use this formalTypeId to skip the marshaling of type IDs when there is a match.
            internal string? FormalTypeId20;

            internal InstanceType InstanceType;

            // The following fields are used for the current slice:

            // The indirection map and indirection table are only used for the sliced format.
            internal Dictionary<AnyClass, int>? IndirectionMap;
            internal List<AnyClass>? IndirectionTable;

            internal EncodingDefinitions.SliceFlags SliceFlags;

            // Position of the slice flags.
            internal Position SliceFlagsPos;

            // Position of the slice size. Used only for the sliced format.
            internal Position SliceSizePos;
        }

        private enum InstanceType : byte
        {
            None = 0,
            Class,
            Exception
        }
    }
}
