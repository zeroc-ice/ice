//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

namespace Ice
{
    // This partial class provides the class/exception marshaling logic.

    public sealed partial class OutputStream
    {
        // Marks the end of a slice for a class instance or user exception.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // lastSlice is true when it's the last (least derived) slice of the instance.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice(bool lastSlice)
        {
            Debug.Assert(InEncapsulation && _current != null);

            if (lastSlice)
            {
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.IsLastSlice;
            }

            // Writes the tagged member end marker if some tagged members were encoded. Note that the optional members
            // are encoded before the indirection table and are included in the slice size.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                WriteByte(EncodingDefinitions.TaggedEndMarker);
            }

            // Writes the slice size if necessary.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                RewriteInt(Distance(_current.SliceSizePos), _current.SliceSizePos);
            }

            if (_current.IndirectionTable?.Count > 0)
            {
                Debug.Assert(_format == FormatType.Sliced);
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasIndirectionTable;

                WriteSize(_current.IndirectionTable.Count);
                foreach (AnyClass v in _current.IndirectionTable)
                {
                    WriteInstance(v);
                }
                _current.IndirectionTable.Clear();
                _current.IndirectionMap?.Clear(); // IndirectionMap is null when writing SlicedData.
            }

            // Update SliceFlags in case they were updated.
            RewriteByte((byte)_current.SliceFlags, _current.SliceFlagsPos);
        }

        // Start writing a slice of a class or exception instance.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the type ID of this slice.
        // firstSlice is true when writing the first (most derived) slice of an instance.
        // slicedData is the preserved sliced-off slices, if any. Can only be provided when firstSlice is true.
        // compactId is the compact type ID of this slice, if specified.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId, bool firstSlice, SlicedData? slicedData = null, int? compactId = null)
        {
            Debug.Assert(InEncapsulation && _current != null);
            if (slicedData.HasValue)
            {
                Debug.Assert(firstSlice);
                try
                {
                    WriteSlicedData(slicedData.Value);
                    firstSlice = false;
                }
                catch (NotSupportedException)
                {
                    // Ignored: for some reason we could not remarshal the sliced data and firstSlice remains true
                }
            }

            _current.SliceFlags = default;

            if (_format == FormatType.Sliced)
            {
                // Encode the slice size if using the sliced format.
                _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasSliceSize;
            }

            _current.SliceFlagsPos = _tail;
            WriteByte(0); // Placeholder for the slice flags

            // For instance slices, encode the flag and the type ID either as a string or index. For exception slices,
            // always encode the type ID a string.
            if (_current.InstanceType == InstanceType.Class)
            {
                // Encode the type ID (only in the first slice for the compact
                // encoding).
                // This  also shows that the firstSlice is currently useful/used only for class instances in
                // compact format.
                if (_format == FormatType.Sliced || firstSlice)
                {
                    if (compactId.HasValue)
                    {
                        _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdCompact;
                        WriteSize(compactId.Value);
                    }
                    else
                    {
                        int index = RegisterTypeId(typeId);
                        if (index < 0)
                        {
                            _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdString;
                            WriteString(typeId);
                        }
                        else
                        {
                            _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTypeIdIndex;
                            WriteSize(index);
                        }
                    }
                }
            }
            else
            {
                WriteString(typeId);
            }

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSizePos = _tail;
                WriteInt(0); // Placeholder for the slice length.
            }
            _current.SliceFirstMemberPos = _tail;
        }

        /// <summary>Writes a class instance to the stream.</summary>
        /// <param name="v">The class instance to write.</param>
        public void WriteClass(AnyClass? v)
        {
            Debug.Assert(InEncapsulation);
            if (v == null)
            {
                WriteSize(0);
            }
            else if (_current != null && _format == FormatType.Sliced)
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
                WriteInstance(v); // Writes the instance or a reference if already marshaled.
            }
        }

        /// <summary>Writes a remote exception to the stream.</summary>
        /// <param name="v">The remote exception to write.</param>
        public void WriteException(RemoteException v)
        {
            Debug.Assert(InEncapsulation && _current == null);
            Debug.Assert(_format == FormatType.Sliced);
            Debug.Assert(!(v is ObjectNotExistException)); // temporary
            Debug.Assert(!(v is OperationNotExistException)); // temporary
            Debug.Assert(!(v is UnhandledException)); // temporary
            Push(InstanceType.Exception);

            v.Write(this);
            Pop(null);
        }

        /// <summary>Writes a tagged class instance to the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <param name="v">The class instance to write.</param>
        public void WriteTaggedClass(int tag, AnyClass? v)
        {
            if (v != null)
            {
                WriteTaggedParamHeader(tag, EncodingDefinitions.TagFormat.Class);
                WriteClass(v);
            }
        }

        internal void WriteSlicedData(SlicedData slicedData)
        {
            Debug.Assert(_current != null);
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

            bool firstSlice = true;
            // There is always at least one slice. See SlicedData constructor.
            foreach (SliceInfo info in slicedData.Slices)
            {
                IceStartSlice(info.TypeId ?? "", firstSlice, null, info.CompactId);
                firstSlice = false;

                // Writes the bytes associated with this slice.
                WriteByteSpan(info.Bytes.Span);

                if (info.HasOptionalMembers)
                {
                    _current.SliceFlags |= EncodingDefinitions.SliceFlags.HasTaggedMembers;
                }

                // Make sure to also re-write the instance indirection table.
                // These instances will be marshaled (and assigned instance IDs) in IceEndSlice.
                if (info.Instances.Count > 0)
                {
                    _current.IndirectionTable ??= new List<AnyClass>();
                    Debug.Assert(_current.IndirectionTable.Count == 0);
                    _current.IndirectionTable.AddRange(info.Instances);
                }
                IceEndSlice(info.IsLastSlice); // TODO: can we check it's indeed the last slice?
            }
            Debug.Assert(!firstSlice);
        }

        private void Pop(InstanceData? savedInstanceData) => _current = savedInstanceData;

        private InstanceData? Push(InstanceType instanceType)
        {
            InstanceData? savedInstanceData = _current;
            _current = new InstanceData(instanceType);
            return savedInstanceData;
        }

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

        // Write this class instance inline if not previously marshaled, otherwise just write its instance ID.
        private void WriteInstance(AnyClass v)
        {
            Debug.Assert(v != null);

            // If the instance was already marshaled, just write its instance ID.
            if (_instanceMap != null && _instanceMap.TryGetValue(v, out int instanceId))
            {
                WriteSize(instanceId);
                return;
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

                InstanceData? savedInstanceData = Push(InstanceType.Class);
                v.Write(this);
                Pop(savedInstanceData);
            }
        }

        private sealed class InstanceData
        {
            internal readonly InstanceType InstanceType;

            // The following fields are used and reused for all the slices of a class or exception instance.
            internal EncodingDefinitions.SliceFlags SliceFlags = default;

            // Position of the optional slice size.
            internal Position SliceSizePos = new Position(0, 0);

            // Position of the first data member in the slice, just after the optional slice size.
            internal Position SliceFirstMemberPos = new Position(0, 0);

            // Position of the slice flags.
            internal Position SliceFlagsPos = new Position(0, 0);

            // The indirection table and indirection map are only used for the sliced format.
            internal List<AnyClass>? IndirectionTable;
            internal Dictionary<AnyClass, int>? IndirectionMap;

            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;
        }

        private enum InstanceType
        {
            Class,
            Exception
        }
    }
}
