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
        // Tells the InputStream the end of a class or exception slice was reached.
        // This is an Ice-internal method marked public because it's called by the generated code.
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

        // Starts reading a slice of a class or exception instance.
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the expected type ID of this slice.
        // firstSlice is true when reading the first (most derived) slice of an instance.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartSlice(string typeId, bool firstSlice)
        {
            Debug.Assert(InEncapsulation);
            if (firstSlice)
            {
                Debug.Assert(_current != null && (_current.SliceTypeId == null || _current.SliceTypeId == typeId));
                if (_current.InstanceType == InstanceType.Class)
                {
                    // For exceptions, we read it for the first slice in ThrowException.
                    ReadIndirectionTableIntoCurrent();
                }

                // We can discard all the unknown slices: the generated code calls IceStartSliceAndGetSlicedData to
                // preserve them and it just called IceStartSlice instead.
                _current.Slices = null;
            }
            else
            {
                string? headerTypeId = ReadSliceHeaderIntoCurrent();
                Debug.Assert(headerTypeId == null || headerTypeId == typeId);
                ReadIndirectionTableIntoCurrent();
            }
        }

        // Starts reading the first slice of an instance and get the unknown slices for this instances that were
        // previously saved (if any).
        // This is an Ice-internal method marked public because it's called by the generated code.
        // typeId is the expected typeId of this slice.
        [EditorBrowsable(EditorBrowsableState.Never)]
        public SlicedData? IceStartSliceAndGetSlicedData(string typeId)
        {
            Debug.Assert(InEncapsulation);
            // Called by generated code for first slice instead of IceStartSlice
            Debug.Assert(_current != null && (_current.SliceTypeId == null || _current.SliceTypeId == typeId));
            if (_current.InstanceType == InstanceType.Class)
            {
                    // For exceptions, we read it for the first slice in ThrowException.
                    ReadIndirectionTableIntoCurrent();
            }
            return SlicedData;
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

            // Read the first slice header, and exception's type ID cannot be null.
            string typeId = ReadSliceHeaderIntoCurrent()!;
            ReadIndirectionTableIntoCurrent(); // we read the indirection table immediately

            while (true)
            {
                RemoteException? remoteEx = null;
                Type? type = Communicator.ResolveClass(typeId);
                if (type != null)
                {
                    try
                    {
                        remoteEx = (RemoteException?)Activator.CreateInstance(type);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidDataException(
                            @$"failed to create an instance of type `{type.Name
                            }' while reading a remote exception with type ID `{typeId}'", ex);
                    }
                }

                // We found the exception.
                if (remoteEx != null)
                {
                    remoteEx.ConvertToUnhandled = true;
                    remoteEx.Read(this);
                    Pop(null);
                    return remoteEx;
                }

                // Slice off what we don't understand.
                SkipSlice();

                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0)
                {
                    // Create and throw a plain RemoteException with the SlicedData.
                    Debug.Assert(SlicedData != null);
                    remoteEx = new RemoteException(SlicedData.Value);
                    remoteEx.ConvertToUnhandled = true;
                    return remoteEx;
                }

                typeId = ReadSliceHeaderIntoCurrent()!;
                ReadIndirectionTableIntoCurrent();
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

            InstanceData? previousCurrent = Push(InstanceType.Class);
            Debug.Assert(_current != null);

            // Read the first slice header.
            string? mostDerivedId = ReadSliceHeaderIntoCurrent();
            string? typeId = mostDerivedId;
            // We cannot read the indirection table at this point as it may reference the new instance that is not
            // created yet.

            AnyClass? v = null;
            List<int>? deferredIndirectionTableList = null;

            while (true)
            {
                Type? cls = null;
                if (typeId != null)
                {
                    Debug.Assert(_current.SliceCompactId == null);
                    cls = Communicator.ResolveClass(typeId);
                }
                else if (_current.SliceCompactId.HasValue)
                {
                    cls = Communicator.ResolveCompactId(_current.SliceCompactId.Value);
                }

                if (cls != null)
                {
                    try
                    {
                        Debug.Assert(!cls.IsAbstract && !cls.IsInterface);
                        v = (AnyClass?)Activator.CreateInstance(cls);
                    }
                    catch (Exception ex)
                    {
                        string typeIdString = typeId ?? _current.SliceCompactId!.ToString()!;
                        throw new InvalidDataException(@$"failed to create an instance of type `{cls.Name
                            } while reading a class with type ID {typeIdString}", ex);
                    }
                }

                if (v != null)
                {
                    // We have an instance, get out of this loop.
                    break;
                }

                // Slice off what we don't understand, and save the indirection table (if any) in
                // deferredIndirectionTableList.
                deferredIndirectionTableList ??= new List<int>();
                deferredIndirectionTableList.Add(SkipSlice());

                // If this is the last slice, keep the instance as an opaque UnknownSlicedClass object.
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0)
                {
                    v = new UnknownSlicedClass();
                    break;
                }

                typeId = ReadSliceHeaderIntoCurrent(); // Read next Slice header for next iteration.
            }

            if (++_classGraphDepth > Communicator.ClassGraphDepthMax)
            {
                throw new InvalidDataException("maximum class graph depth reached");
            }

            // Add the instance to the map/list of instances. This must be done before reading the instances (for
            // circular references).
            _instanceMap ??= new List<AnyClass>();
            _instanceMap.Add(v);

            // Read all the deferred indirection tables now that the instance is inserted in _instanceMap.
            if (deferredIndirectionTableList?.Count > 0)
            {
                int savedPos = _pos;

                Debug.Assert(_current.Slices?.Count == deferredIndirectionTableList.Count);
                for (int i = 0; i < deferredIndirectionTableList.Count; ++i)
                {
                    int pos = deferredIndirectionTableList[i];
                    if (pos > 0)
                    {
                        _pos = pos;
                        _current.Slices[i].Instances = Array.AsReadOnly(ReadIndirectionTable());
                    }
                    // else remains empty
                }
                _pos = savedPos;
            }

            // Read the instance.
            v.Read(this);
            Pop(previousCurrent);

            --_classGraphDepth;
            return v;
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
                    } while ((sliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) == 0);
                    _classGraphDepth--;
                }
            }
        }

        // Skips the body of the current slice and it indirection table (if any).
        // When it's a class instance and there is an indirection table, it returns the starting position of that
        // indirection table; otherwise, it return 0.
        private int SkipSlice()
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

            int startOfIndirectionTable = 0;

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                if (_current.InstanceType == InstanceType.Class)
                {
                    startOfIndirectionTable = _pos;
                    SkipIndirectionTable();
                }
                else
                {
                    Debug.Assert(_current.PosAfterIndirectionTable != null);
                    // Move past indirection table
                    _pos = _current.PosAfterIndirectionTable.Value;
                    _current.PosAfterIndirectionTable = null;
                }
            }
            _current.Slices ??= new List<SliceInfo>();
            var info = new SliceInfo(_current.SliceTypeId,
                                     _current.SliceCompactId,
                                     new ReadOnlyMemory<byte>(bytes),
                                     Array.AsReadOnly(_current.IndirectionTable ?? Array.Empty<AnyClass>()),
                                     hasOptionalMembers,
                                     (_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0);
            _current.Slices.Add(info);

            // An exception slice may have an indirection table (saved above). We don't need it anymore since we're
            // skipping this slice.
            _current.IndirectionTable = null;
            return startOfIndirectionTable;
        }

        private sealed class InstanceData
        {
            internal InstanceData(InstanceType instanceType) => InstanceType = instanceType;

            // Instance attributes
            internal readonly InstanceType InstanceType;
            internal List<SliceInfo>? Slices; // Preserved slices.

            // Slice attributes
            internal EncodingDefinitions.SliceFlags SliceFlags = default;
            internal int SliceSize = 0;
            internal string? SliceTypeId;
            internal int? SliceCompactId;
            // Indirection table of the current slice
            internal AnyClass[]? IndirectionTable;
            internal int? PosAfterIndirectionTable;
        }

        private enum InstanceType { Class, Exception }
    }
}
