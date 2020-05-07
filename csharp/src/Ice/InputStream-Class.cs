//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

namespace Ice
{
    // This partial class provides the class/exception unmarshaling logic.
    public sealed partial class InputStream
    {
        /// <summary>Tells the InputStream the end of a class or exception slice was reached. This is an Ice-internal
        /// method marked public because it's called by the generated code.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceEndSlice()
        {
            // Note that IceEndSlice is not called when we call SkipSlice.
            Debug.Assert(InEncapsulation && _current != null);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                SkipTaggedParams();
            }
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                Debug.Assert(_current.PosAfterIndirectionTable.HasValue && _current.IndirectionTable != null);
                _pos = _current.PosAfterIndirectionTable.Value;
                _current.PosAfterIndirectionTable = null;
                _current.IndirectionTable = null;
            }
        }

        /// <summary>Starts reading the first slice of a class instance. This is an Ice-internal method marked public
        /// because it's called by the generated code.</summary>
        /// <param name="typeId">The expected type ID of this slice.</param>
        /// <param name="v">The class instance.</param>
        /// <param name="setSlicedData">When true, the class is "preserved" and this method will set the class
        /// instance's SlicedData.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartFirstSlice(string typeId, AnyClass v, bool setSlicedData = false)
        {
            Debug.Assert(InEncapsulation);
            Debug.Assert(_current != null && (_current.SliceTypeId == null || _current.SliceTypeId == typeId));
            Debug.Assert(_current.InstanceType == InstanceType.Class);

            FirstSliceInit(v, setSlicedData);
            ReadIndirectionTableIntoCurrent();
        }

        /// <summary>Starts reading the first slice of a remote exception. This is an Ice-internal method marked public
        /// because it's called by the generated code.</summary>
        /// <param name="typeId">The expected type ID of this slice.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public SlicedData? IceStartFirstSlice(string typeId)
        {
            Debug.Assert(InEncapsulation);
            Debug.Assert(_current != null && _current.SliceTypeId == typeId);
            Debug.Assert(_current.InstanceType == InstanceType.Exception);
            return GetSlicedData();
        }

        /// <summary>Starts reading a base slice of a class instance or remote exception (any slice except the first
        /// slice). This is an Ice-internal method marked public because it's called by the generated code.</summary>
        /// <param name="typeId">The expected type ID of this slice.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId)
        {
            Debug.Assert(InEncapsulation);
            string? headerTypeId = ReadSliceHeaderIntoCurrent();
            Debug.Assert(headerTypeId == null || headerTypeId == typeId);
            ReadIndirectionTableIntoCurrent();
        }

        /// <summary>Reads a class instance from the stream.</summary>
        /// <returns>The class instance, or null.</returns>
        public T? ReadClass<T>() where T : AnyClass
        {
            AnyClass? obj = ReadAnyClass();
            if (obj == null)
            {
                return null;
            }
            else if (obj is T)
            {
                return (T)obj;
            }
            else
            {
                throw new InvalidDataException(@$"read instance of type `{obj.GetType().FullName
                    }' but expected instance of type `{typeof(T).FullName}'");
            }
        }

        /// <summary>Reads a tagged class instance from the stream.</summary>
        /// <param name="tag">The tag.</param>
        /// <returns>The class instance, or null.</returns>
        public T? ReadTaggedClass<T>(int tag) where T : AnyClass
        {
            AnyClass? obj = ReadTaggedAnyClass(tag);
            if (obj == null)
            {
                return null;
            }
            else if (obj is T)
            {
                return (T)obj;
            }
            else
            {
                throw new InvalidDataException(@$"read instance of type `{obj.GetType().FullName
                    }' but expected instance of type `{typeof(T).FullName}'");
            }
        }

        /// <summary>Reads a remote exception from the stream.</summary>
        /// <returns>The remote exception.</returns>
        public RemoteException ReadException()
        {
            Push(InstanceType.Exception);
            Debug.Assert(_current != null);

            RemoteException? remoteEx = null;
            do
            {
                // Read the slice header; an exception's type ID cannot be null.
                string typeId = ReadSliceHeaderIntoCurrent()!;
                ReadIndirectionTableIntoCurrent(); // we read the indirection table immediately

                if (Communicator.FindRemoteExceptionFactory(typeId) is IRemoteExceptionFactory factory)
                {
                    remoteEx = factory.Read(this);
                }
                else if (SkipSlice()) // Slice off what we don't understand.
                {
                    remoteEx = new RemoteException(GetSlicedData()!.Value);
                }
            }
            while (remoteEx == null);

            Pop(null);
            return remoteEx;
        }

        internal void FirstSliceInit(AnyClass v, bool setSlicedData)
        {
            Debug.Assert(_current != null);

            // Add the instance to the map/list of instances. This must be done before reading the instances (for
            // circular references).
            _instanceMap ??= new List<AnyClass>();
            _instanceMap.Add(v);

            // Read all the deferred indirection tables now that the instance is inserted in _instanceMap.
            if (_current.DeferredIndirectionTableList?.Count > 0)
            {
                int savedPos = _pos;

                Debug.Assert(_current.Slices?.Count == _current.DeferredIndirectionTableList.Count);
                for (int i = 0; i < _current.DeferredIndirectionTableList.Count; ++i)
                {
                    int pos = _current.DeferredIndirectionTableList[i];
                    if (pos > 0)
                    {
                        _pos = pos;
                        _current.Slices[i].Instances = Array.AsReadOnly(ReadIndirectionTable());
                    }
                    // else remains empty
                }
                _pos = savedPos;
            }

            if (setSlicedData)
            {
                v.SlicedData = GetSlicedData();
            }
        }

        // Returns the sliced data held by the current instance
        private SlicedData? GetSlicedData()
        {
            Debug.Assert(_current != null);
            if (_current.Slices == null)
            {
                return null;
            }
            else
            {
                return new SlicedData(Encoding, _current.Slices);
            }
        }

        // Replaces the current instance by savedInstance.
        private void Pop(InstanceData? savedInstance)
        {
            Debug.Assert(_current != null);
            _current = savedInstance;
        }

        // Creates a new current instance of the specified slice type and return the previous current instance, if any.
        private InstanceData? Push(InstanceType instanceType)
        {
            // Can't have a current instance already if we are reading an exception
            Debug.Assert(instanceType == InstanceType.Class || _current == null);
            InstanceData? oldInstance = _current;
            _current = new InstanceData(instanceType);
            return oldInstance;
        }

        // Reads a class instance.
        private AnyClass? ReadAnyClass()
        {
            int index = ReadSize();
            if (index < 0)
            {
                throw new InvalidDataException($"invalid index {index} while reading a class");
            }
            else if (index == 0)
            {
                return null;
            }
            else if (_current != null &&
                (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                // When reading an instance within a slice and there is an indirection table, we have an index within
                // this indirection table.
                // We need to decrement index since position 0 in the indirection table corresponds to index 1.
                index--;
                if (index < _current.IndirectionTable?.Length)
                {
                    return _current.IndirectionTable[index];
                }
                else
                {
                    throw new InvalidDataException("index too big for indirection table");
                }
            }
            else
            {
                return ReadInstance(index);
            }
        }

        // Reads a tagged class instance.
        private AnyClass? ReadTaggedAnyClass(int tag) =>
            ReadTaggedParamHeader(tag, EncodingDefinitions.TagFormat.Class) ? ReadAnyClass() : null;

        private AnyClass[] ReadIndirectionTable()
        {
            int size = ReadAndCheckSeqSize(1);
            if (size == 0)
            {
                throw new InvalidDataException("invalid empty indirection table");
            }
            var indirectionTable = new AnyClass[size];
            for (int i = 0; i < indirectionTable.Length; ++i)
            {
                int index = ReadSize();
                if (index < 1)
                {
                    throw new InvalidDataException($"read invalid index {index} in indirection table");
                }
                indirectionTable[i] = ReadInstance(index);
            }
            return indirectionTable;
        }

        // Reads the indirection table into _current's fields if there is an indirection table.
        // Precondition: called after reading the slice's header.
        // This method does not change Pos.
        private void ReadIndirectionTableIntoCurrent()
        {
            Debug.Assert(_current != null && _current.IndirectionTable == null);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                int savedPos = _pos;
                if (_current.SliceSize < 4)
                {
                    throw new InvalidDataException($"invalid slice size: {_current.SliceSize}");
                }
                _pos = savedPos + _current.SliceSize - 4;
                _current.IndirectionTable = ReadIndirectionTable();
                _current.PosAfterIndirectionTable = _pos;
                _pos = savedPos;
            }
        }

        private AnyClass ReadInstance(int index)
        {
            Debug.Assert(index > 0);

            if (index > 1)
            {
                if (_instanceMap != null && _instanceMap.Count > index - 2)
                {
                    return _instanceMap[index - 2];
                }
                throw new InvalidDataException($"could not find index {index} in {nameof(_instanceMap)}");
            }

            if (++_classGraphDepth > Communicator.ClassGraphDepthMax)
            {
                throw new InvalidDataException("maximum class graph depth reached");
            }

            InstanceData? previousCurrent = Push(InstanceType.Class);
            Debug.Assert(_current != null);

            AnyClass? instance = null;
            do
            {
                // Read the slice header.
                string? typeIdOpt = ReadSliceHeaderIntoCurrent();

                // We cannot read the indirection table at this point as it may reference the new instance that is
                // not created yet.

                IClassFactory? factory = null;
                if (typeIdOpt is string typeId)
                {
                    Debug.Assert(_current.SliceCompactId == null);
                    factory = Communicator.FindClassFactory(typeId);
                }
                else if (_current.SliceCompactId is int compactId)
                {
                    factory = Communicator.FindClassFactory(compactId);
                }

                if (factory != null)
                {
                    instance = factory.Read(this);
                }
                else if (SkipSlice()) // Slice off what we don't understand.
                {
                    instance = new UnknownSlicedClass(this);
                }
            }
            while (instance == null);

            Pop(previousCurrent);
            --_classGraphDepth;
            return instance;
        }

        // Reads a slice header into _current.
        // Returns the type ID of that slice. Null means it's a slice in compact format without a type ID, or a slice
        // with a compact ID we could not resolve.
        private string? ReadSliceHeaderIntoCurrent()
        {
            Debug.Assert(_current != null);

            _current.SliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();

            // Read the type ID. For class slices, the type ID is encoded as a string or as an index or as a compact ID,
            // for exceptions it's always encoded as a string.
            if (_current.InstanceType == InstanceType.Class)
            {
                // TYPE_ID_COMPACT must be checked first!
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdCompact) ==
                    EncodingDefinitions.SliceFlags.HasTypeIdCompact)
                {
                    _current.SliceCompactId = ReadSize();
                    _current.SliceTypeId = null;
                }
                else if ((_current.SliceFlags & (EncodingDefinitions.SliceFlags.HasTypeIdIndex |
                    EncodingDefinitions.SliceFlags.HasTypeIdString)) != 0)
                {
                    _current.SliceTypeId = ReadTypeId(
                        (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdIndex) != 0);
                    _current.SliceCompactId = null;
                }
                else
                {
                    // Slice in compact format, without a type ID or compact ID.
                    Debug.Assert((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0);
                    _current.SliceTypeId = null;
                    _current.SliceCompactId = null;
                }
            }
            else
            {
                _current.SliceTypeId = ReadString();
                Debug.Assert(_current.SliceCompactId == null); // no compact ID for exceptions
            }

            // Read the slice size if necessary.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSize = ReadInt();
                if (_current.SliceSize < 4)
                {
                    throw new InvalidDataException($"invalid slice size: {_current.SliceSize}");
                }
            }
            else
            {
                _current.SliceSize = 0;
            }

            // Clear other per-slice fields:
            _current.IndirectionTable = null;
            _current.PosAfterIndirectionTable = null;

            return _current.SliceTypeId;
        }

        private string ReadTypeId(bool isIndex)
        {
            _typeIdMap ??= new List<string>();

            if (isIndex)
            {
                int index = ReadSize();
                if (index > 0 && index - 1 < _typeIdMap.Count)
                {
                    // The encoded type-id indexes start at 1, not 0.
                    return _typeIdMap[index - 1];
                }
                throw new InvalidDataException($"read invalid type ID index {index}");
            }
            else
            {
                string typeId = ReadString();

                // The typeIds of slices in indirection tables can be read several times: when we skip the indirection
                // table and later on when we read it. We only want to add this typeId to the list and assign it an
                // index when it's the first time we read it, so we save the largest position we read to figure out when
                // to add to the list.
                if (_pos > _posAfterLatestInsertedTypeId)
                {
                    _posAfterLatestInsertedTypeId = _pos;
                    _typeIdMap.Add(typeId);
                }

                return typeId;
            }
        }

        // Skips the indirection table. The caller must save the current stream position before calling
        // SkipIndirectionTable (to read the indirection table at a later point) except when the caller is
        // SkipIndirectionTable itself.
        private void SkipIndirectionTable()
        {
            Debug.Assert(_current != null);
            // We should never skip an exception's indirection table
            Debug.Assert(_current.InstanceType == InstanceType.Class);

            // We use ReadSize and not ReadAndCheckSeqSize here because we don't allocate memory for this sequence, and
            // since we are skipping this sequence to read it later, we don't want to double-count its contribution to
            // _minTotalSeqSize.
            int tableSize = ReadSize();
            for (int i = 0; i < tableSize; ++i)
            {
                int index = ReadSize();
                if (index <= 0)
                {
                    throw new InvalidDataException($"read invalid index {index} in indirection table");
                }
                if (index == 1)
                {
                    if (++_classGraphDepth > Communicator.ClassGraphDepthMax)
                    {
                        throw new InvalidDataException("maximum class graph depth reached");
                    }

                    // Read/skip this instance
                    EncodingDefinitions.SliceFlags sliceFlags;
                    do
                    {
                        sliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdCompact) ==
                            EncodingDefinitions.SliceFlags.HasTypeIdCompact)
                        {
                            ReadSize(); // compact type-id
                        }
                        else if ((sliceFlags & (EncodingDefinitions.SliceFlags.HasTypeIdIndex |
                            EncodingDefinitions.SliceFlags.HasTypeIdString)) != 0)
                        {
                            // This can update the typeIdMap
                            ReadTypeId((sliceFlags & EncodingDefinitions.SliceFlags.HasTypeIdIndex) != 0);
                        }
                        else
                        {
                            throw new InvalidDataException(
                                "indirection table cannot hold an instance without a type ID");
                        }

                        // Read the slice size, then skip the slice
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0)
                        {
                            throw new InvalidDataException("size of slice missing");
                        }
                        int sliceSize = ReadInt();
                        if (sliceSize < 4)
                        {
                            throw new InvalidDataException($"invalid slice size: {sliceSize}");
                        }
                        _pos = _pos + sliceSize - 4;

                        // If this slice has an indirection table, skip it too
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
                        {
                            SkipIndirectionTable();
                        }
                    }
                    while ((sliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) == 0);
                    _classGraphDepth--;
                }
            }
        }

        // Skips the body of the current slice and its indirection table (if any).
        // Returns true when this slice is the last slice; otherwise, false.
        private bool SkipSlice()
        {
            Debug.Assert(_current != null);
            if (Communicator.TraceLevels.Slicing > 0)
            {
                ILogger logger = Communicator.Logger;
                string slicingCat = Communicator.TraceLevels.SlicingCat;
                if (_current.InstanceType == InstanceType.Exception)
                {
                    IceInternal.TraceUtil.TraceSlicing("exception", _current.SliceTypeId ?? "", slicingCat, logger);
                }
                else
                {
                    IceInternal.TraceUtil.TraceSlicing("object", _current.SliceTypeId ?? "", slicingCat, logger);
                }
            }

            int start = _pos;

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                Debug.Assert(_current.SliceSize >= 4);
                Skip(_current.SliceSize - 4);
            }
            else
            {
                if (_current.InstanceType == InstanceType.Class)
                {
                    string typeId = _current.SliceTypeId ?? _current.SliceCompactId!.ToString()!;
                    throw new InvalidDataException(@$"no class found for type ID `{typeId
                        }' and compact format prevents slicing (the sender should use the sliced format instead)");
                }
                else
                {
                    string typeId = _current.SliceTypeId!;
                    throw new InvalidDataException(@$"no exception class found for type ID `{typeId
                        }' and compact format prevents slicing (the sender should use the sliced format instead)");
                }
            }

            // Preserve this slice.
            bool hasOptionalMembers = (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0;
            int end = _pos;
            int dataEnd = end;
            if (hasOptionalMembers)
            {
                // Don't include the tagged end marker. It will be re-written by IceEndSlice when the sliced data is
                // re-written.
                --dataEnd;
            }
            byte[] bytes = new byte[dataEnd - start];
            _buffer.Slice(start, bytes.Length).CopyTo(bytes);

            bool hasIndirectionTable = (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0;

            if (_current.InstanceType == InstanceType.Class)
            {
                _current.DeferredIndirectionTableList ??= new List<int>();
                if (hasIndirectionTable)
                {
                    int savedPos = _pos;
                    SkipIndirectionTable();
                    _current.DeferredIndirectionTableList.Add(savedPos); // we want to later read the deepest first
                }
                else
                {
                    _current.DeferredIndirectionTableList.Add(0); // keep a slot for each slice
                }
            }
            else if (hasIndirectionTable)
            {
                Debug.Assert(_current.PosAfterIndirectionTable != null);
                // Move past indirection table
                _pos = _current.PosAfterIndirectionTable.Value;
                _current.PosAfterIndirectionTable = null;
            }

            bool isLastSlice = (_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0;

            _current.Slices ??= new List<SliceInfo>();
            var info = new SliceInfo(_current.SliceTypeId,
                                     _current.SliceCompactId,
                                     new ReadOnlyMemory<byte>(bytes),
                                     Array.AsReadOnly(_current.IndirectionTable ?? Array.Empty<AnyClass>()),
                                     hasOptionalMembers,
                                     isLastSlice);
            _current.Slices.Add(info);

            // An exception slice may have an indirection table (saved above). We don't need it anymore since we're
            // skipping this slice.
            _current.IndirectionTable = null;

            return isLastSlice;
        }

        private sealed class InstanceData
        {
            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;

            // Instance attributes
            internal readonly InstanceType InstanceType;
            internal List<SliceInfo>? Slices; // Preserved slices.
            internal List<int>? DeferredIndirectionTableList;

            // Slice attributes
            internal EncodingDefinitions.SliceFlags SliceFlags = default;
            internal int SliceSize = 0;
            internal string? SliceTypeId;
            internal int? SliceCompactId;

            // Indirection table of the current slice
            internal AnyClass[]? IndirectionTable;
            internal int? PosAfterIndirectionTable;
        }

        private enum InstanceType
        {
            Class,
            Exception
        }
    }
}
