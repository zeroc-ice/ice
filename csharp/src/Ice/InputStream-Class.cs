// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

namespace ZeroC.Ice
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
            Debug.Assert(_inEncapsulation && _current.InstanceType != InstanceType.None);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0)
            {
                SkipTaggedParams();
            }
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                Debug.Assert(_current.PosAfterIndirectionTable != null && _current.IndirectionTable != null);
                Pos = _current.PosAfterIndirectionTable.Value;
                _current.PosAfterIndirectionTable = null;
                _current.IndirectionTable = null;
            }
        }

        /// <summary>Starts reading the first slice of a class or exception. This is an Ice-internal method marked
        /// public because it's called by the generated code.</summary>
        /// <returns>The sliced-off slices, if any.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public SlicedData? IceStartFirstSlice() => SlicedData;

        /// <summary>Starts reading a base slice of a class instance or remote exception (any slice except the first
        /// slice). This is an Ice-internal method marked public because it's called by the generated code.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceStartNextSlice()
        {
            Debug.Assert(_inEncapsulation);
            ReadNextSliceHeaderIntoCurrent();
            ReadIndirectionTableIntoCurrent();
        }

        /// <summary>Reads a class instance from the stream.</summary>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being read.
        /// It's T.IceTypeId for generated classes. Use null when the type of the parameter/data member is AnyClass.
        /// </param>
        /// <returns>The class instance read from the stream.</returns>
        public T ReadClass<T>(string? formalTypeId) where T : AnyClass =>
            ReadNullableClass<T>(formalTypeId) ??
                throw new InvalidDataException("read a null class instance, but expected a non-null instance");

        /// <summary>Reads a remote exception from the stream.</summary>
        /// <returns>The remote exception.</returns>
        public RemoteException ReadException()
        {
            if (!_inEncapsulation)
            {
                throw new InvalidOperationException("cannot read an exception outside an encapsulation");
            }
            if (Communicator == null)
            {
                throw new InvalidOperationException(
                    "cannot read an exception from an InputStream with a null communicator");
            }

            Debug.Assert(_current.InstanceType == InstanceType.None);
            _current.InstanceType = InstanceType.Exception;

            RemoteException? remoteEx = null;
            string? errorMessage = null;
            RemoteExceptionOrigin? origin = null;

            // The unmarshaling of remote exceptions is similar with the 1.1 and 2.0 encodings, in particular we can
            // read the indirection table (if there is one) immediately after reading each slice header because the
            // indirection table cannot reference the exception itself.
            // With the 1.1 encoding, each slice contains its type ID (as a string), while with the 2.0 encoding the
            // first slice contains all the type IDs.

            if (OldEncoding)
            {
                do
                {
                    // The type ID is always read for an exception and cannot be null.
                    (string? typeId, _) = ReadSliceHeaderIntoCurrent11();
                    Debug.Assert(typeId != null);

                    ReadIndirectionTableIntoCurrent(); // we read the indirection table immediately.

                    if (Communicator.FindRemoteExceptionFactory(typeId) is
                        Func<string?, RemoteExceptionOrigin?, RemoteException> factory)
                    {
                        // The 1.1 encoding does not carry the error message or origin so they are always null.
                        remoteEx = factory(errorMessage, origin);
                    }
                    else if (SkipSlice(typeId)) // Slice off what we don't understand.
                    {
                        break;
                    }
                }
                while (remoteEx == null);
            }
            else
            {
                // The type IDs are always read and cannot be null or empty.
                string[]? allTypeIds;
                (allTypeIds, errorMessage, origin) = ReadFirstSliceHeaderIntoCurrent20();
                Debug.Assert(allTypeIds != null && allTypeIds.Length > 0 && errorMessage != null);
                bool firstSlice = true;

                foreach (string typeId in allTypeIds)
                {
                    if (firstSlice)
                    {
                        firstSlice = false;
                    }
                    else
                    {
                        ReadNextSliceHeaderIntoCurrent();
                    }
                    ReadIndirectionTableIntoCurrent(); // we read the indirection table immediately.

                    Func<string?, RemoteExceptionOrigin?, RemoteException>? factory =
                        Communicator.FindRemoteExceptionFactory(typeId);
                    if (factory != null)
                    {
                        remoteEx = factory(errorMessage, origin);
                        break; // foreach
                    }
                    else if (SkipSlice(typeId))
                    {
                        // It should be the last element of allTypeIds; if it's not, we'll fail when reading the slices.
                        break;
                    }
                    // else, loop.
                }
            }

            remoteEx ??= new RemoteException(errorMessage);
            remoteEx.Read(this);

            _current = default;
            return remoteEx;
        }

        /// <summary>Reads a nullable class instance from the stream.</summary>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being read.
        /// Use null when the type of the parameter/data member is AnyClass.</param>
        /// <returns>The class instance read from the stream, or null.</returns>
        public T? ReadNullableClass<T>(string? formalTypeId) where T : AnyClass
        {
            AnyClass? obj = ReadAnyClass(formalTypeId);
            if (obj is T result)
            {
                return result;
            }
            else if (obj == null)
            {
                return null;
            }
            else
            {
                throw new InvalidDataException(@$"read instance of type `{obj.GetType().FullName
                    }' but expected instance of type `{typeof(T).FullName}'");
            }
        }

        /// <summary>Reads a class instance from the stream.</summary>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being read.
        /// </param>
        /// <returns>The class instance read from the stream. Can be null.</returns>
        private AnyClass? ReadAnyClass(string? formalTypeId)
        {
            if (!_inEncapsulation)
            {
                throw new InvalidOperationException("cannot read a class outside an encapsulation");
            }
            if (Communicator == null)
            {
                throw new InvalidOperationException(
                    "cannot read a class from an InputStream with a null communicator");
            }

            int index = ReadSize();
            if (index < 0)
            {
                throw new InvalidDataException($"invalid index {index} while reading a class");
            }
            else if (index == 0)
            {
                return null;
            }
            else if (_current.InstanceType != InstanceType.None &&
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
                return ReadInstance(index, formalTypeId);
            }
        }

        /// <summary>Reads the header of the first (and current) slice of a class/exception instance into _current.
        /// </summary>
        /// <returns>Null when no type ID was encoded (because of formal type optimization) or a non-empty array of type
        /// IDs. With the compact format, this array contains a single element. Also returns an error message for remote
        /// exceptions.</returns>
        private (string[]? AllTypeIds,
                 string? ErrorMessage,
                 RemoteExceptionOrigin? Origin) ReadFirstSliceHeaderIntoCurrent20()
        {
            string[]? typeIds;
            string? errorMessage = null;
            RemoteExceptionOrigin? origin = null;

            _current.SliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();
            EncodingDefinitions.TypeIdKind typeIdKind = _current.SliceFlags.GetTypeIdKind();

            if (_current.InstanceType == InstanceType.Class)
            {
                _typeIdMap20 ??= new List<string[]>();

                switch (typeIdKind)
                {
                    case EncodingDefinitions.TypeIdKind.Index:
                        int index = ReadSize();
                        if (index > 0 && index - 1 < _typeIdMap20.Count)
                        {
                            // The encoded type-id indexes start at 1, not 0.
                            typeIds = _typeIdMap20[index - 1];
                        }
                        else
                        {
                            throw new InvalidDataException($"read invalid type ID index {index}");
                        }
                        break;

                    case EncodingDefinitions.TypeIdKind.String:
                        typeIds = new string[] { ReadString() };
                        _typeIdMap20.Add(typeIds);
                        break;

                    case EncodingDefinitions.TypeIdKind.Sequence20:
                        typeIds = ReadArray(1, IceReaderIntoString);
                        if (typeIds.Length == 0)
                        {
                            throw new InvalidDataException("received empty type ID sequence");
                        }
                        _typeIdMap20.Add(typeIds);
                        break;

                    default:
                        // TypeIdKind has only 4 possible values.
                        Debug.Assert(typeIdKind == EncodingDefinitions.TypeIdKind.None);
                        typeIds = null;
                        break;
                }
            }
            else
            {
                // Exception
                if (typeIdKind == EncodingDefinitions.TypeIdKind.Sequence20)
                {
                    typeIds = ReadArray(1, IceReaderIntoString);
                    if (typeIds.Length == 0)
                    {
                        throw new InvalidDataException("received empty type ID sequence");
                    }
                }
                else
                {
                    throw new InvalidDataException(
                        $"the type IDs for an exception cannot be encoded using {typeIdKind}");
                }
                errorMessage = ReadString();
                origin = new RemoteExceptionOrigin(this);
            }

            // Read the slice size if available.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSize = ReadSize();
            }
            else
            {
                _current.SliceSize = 0;
            }
            return (typeIds, errorMessage, origin);
        }

        /// <summary>Reads an indirection table from the stream, without updating _current.</summary>
        /// <returns>The indirection table.</returns>
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
                indirectionTable[i] = ReadInstance(index, formalTypeId: null);
            }
            return indirectionTable;
        }

        /// <summary>Reads the indirection table into _current's fields if there is an indirection table.
        /// Precondition: called after reading the header of the current slice. This method does not change _pos.
        /// </summary>
        private void ReadIndirectionTableIntoCurrent()
        {
            Debug.Assert(_current.IndirectionTable == null);
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
            {
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0)
                {
                    throw new InvalidDataException("slice has indirection table but does not have a size");
                }

                int savedPos = Pos;
                Pos = savedPos + _current.SliceSize;
                _current.IndirectionTable = ReadIndirectionTable();
                _current.PosAfterIndirectionTable = Pos;
                Pos = savedPos;
            }
        }

        /// <summary>Reads a class instance from the stream.</summary>
        /// <param name="index">The index of the class instance. If greater than 1, it's a reference to a previously
        /// seen class; if 1, the class's bytes are next on the stream. Cannot be 0 or less.</param>
        /// <param name="formalTypeId">The type ID of the formal type of the parameter or data member being read.
        /// </param>
        private AnyClass ReadInstance(int index, string? formalTypeId)
        {
            Debug.Assert(Communicator != null);
            Debug.Assert(index > 0);

            if (index > 1)
            {
                if (_instanceMap != null && _instanceMap.Count > index - 2)
                {
                    return _instanceMap[index - 2];
                }
                throw new InvalidDataException($"could not find index {index} in {nameof(_instanceMap)}");
            }

            if (++_classGraphDepth > Communicator.ClassGraphMaxDepth)
            {
                throw new InvalidDataException("maximum class graph depth reached");
            }

            // Save current in case we're reading a nested instance.
            InstanceData previousCurrent = _current;
            _current = default;
            _current.InstanceType = InstanceType.Class;

            AnyClass? instance = null;
            _instanceMap ??= new List<AnyClass>();

            if (OldEncoding)
            {
                bool readIndirectionTable = true;
                do
                {
                    // Read the slice header.
                    (string? typeId, int? compactId) = ReadSliceHeaderIntoCurrent11();

                    // We cannot read the indirection table at this point as it may reference the new instance that is
                    // not created yet.

                    Func<AnyClass>? factory = null;
                    if (typeId != null)
                    {
                        factory = Communicator.FindClassFactory(typeId);
                    }
                    else if (compactId is int compactIdValue)
                    {
                        factory = Communicator.FindClassFactory(compactIdValue);
                    }

                    if (factory != null)
                    {
                        instance = factory();
                    }
                    else if (SkipSlice(typeId, compactId)) // Slice off what we don't understand.
                    {
                        instance = new UnknownSlicedClass();
                        // Don't read the indirection table as it's the last entry in DeferredIndirectionTableList11.
                        readIndirectionTable = false;
                    }
                }
                while (instance == null);

                // Add the instance to the map/list of instances. This must be done before reading the instances (for
                // circular references).
                _instanceMap.Add(instance);

                // Read all the deferred indirection tables now that the instance is inserted in _instanceMap.
                if (_current.DeferredIndirectionTableList11?.Count > 0)
                {
                    int savedPos = Pos;

                    Debug.Assert(_current.Slices?.Count == _current.DeferredIndirectionTableList11.Count);
                    for (int i = 0; i < _current.DeferredIndirectionTableList11.Count; ++i)
                    {
                        int pos = _current.DeferredIndirectionTableList11[i];
                        if (pos > 0)
                        {
                            Pos = pos;
                            _current.Slices[i].Instances = Array.AsReadOnly(ReadIndirectionTable());
                        }
                        // else remains empty
                    }

                    Pos = savedPos;
                }

                if (readIndirectionTable)
                {
                    ReadIndirectionTableIntoCurrent();
                }
            }
            else
            {
                // With the 2.0 encoding, we don't need a DeferredIndirectionTableList because all the type IDs are
                // provided by the first slice (when using the sliced format).

                (string[]? allTypeIds, _, _) = ReadFirstSliceHeaderIntoCurrent20();
                if (allTypeIds != null)
                {
                    int skipCount = 0;
                    foreach (string typeId in allTypeIds)
                    {
                        if (Communicator.FindClassFactory(typeId) is Func<AnyClass> factory)
                        {
                            instance = factory();
                            break; // foreach
                        }
                        else
                        {
                            skipCount++;
                        }
                    }

                    instance ??= new UnknownSlicedClass();

                    _instanceMap.Add(instance);
                    ReadIndirectionTableIntoCurrent(); // read the indirection table immediately

                    for (int i = 0; i < skipCount; ++i)
                    {
                        // SkipSlice saves the slice data including the current indirection table, if any.
                        if (SkipSlice(allTypeIds[i]))
                        {
                            if (i != skipCount - 1)
                            {
                                throw new InvalidDataException(
                                    "class slice marked as the last slice is not the last slice");
                            }
                            break;
                        }
                        else
                        {
                            ReadNextSliceHeaderIntoCurrent();
                            ReadIndirectionTableIntoCurrent();
                        }
                    }
                }
                else if (formalTypeId != null)
                {
                    // received null and formalTypeId is not null, apply formal type optimization.
                    if (Communicator.FindClassFactory(formalTypeId) is Func<AnyClass> factory)
                    {
                        instance = factory();
                        _instanceMap.Add(instance);
                        ReadIndirectionTableIntoCurrent();
                        // Nothing to skip
                    }
                    else
                    {
                        throw new ArgumentException($"cannot find class factory for `{formalTypeId}'",
                            nameof(formalTypeId));
                    }
                }
                else
                {
                    throw new InvalidDataException(
                        @$"cannot read a class instance with no type ID; did you forget to specify the {
                            nameof(formalTypeId)}?");
                }
            }

            instance.Read(this);

            _current = previousCurrent;
            --_classGraphDepth;
            return instance;
        }

        /// <summary>Reads the header of the current slice into _current; this method is used when the current slice
        /// is not the first (most derived) slice.</summary>
        private void ReadNextSliceHeaderIntoCurrent()
        {
            if (OldEncoding)
            {
                // With the 1.1 encoding, each slice header in sliced format contains a type ID - we read it and
                // ignore it.
                _ = ReadSliceHeaderIntoCurrent11();
            }
            else
            {
                _current.SliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();
                if (_current.SliceFlags.GetTypeIdKind() != EncodingDefinitions.TypeIdKind.None)
                {
                    throw new InvalidDataException(
                        $"invalid type ID kind `{_current.SliceFlags.GetTypeIdKind()}' for next slice");
                }

                // Read the slice size if available.
                if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
                {
                    _current.SliceSize = ReadSize();
                }
                else
                {
                    _current.SliceSize = 0;
                }
            }
        }

        /// <summary>Reads the header of the current slice into _current.</summary>
        /// <returns>The type ID or the compact ID of the current slice.</returns>
        private (string? TypeId, int? CompactId) ReadSliceHeaderIntoCurrent11()
        {
            _current.SliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();

            string? typeId;
            int? compactId = null;

            // Read the type ID. For class slices, the type ID is encoded as a string or as an index or as a compact ID,
            // for exceptions it's always encoded as a string.
            if (_current.InstanceType == InstanceType.Class)
            {
                (typeId, compactId) = ReadTypeId11(_current.SliceFlags.GetTypeIdKind());

                if (typeId == null && compactId == null)
                {
                    if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
                    {
                        // A slice in compact format cannot carry a size.
                        throw new InvalidDataException("inconsistent slice flags");
                    }
                }
            }
            else
            {
                // Exception slices always include the type ID, even when using the compact format.
                typeId = ReadString();
            }

            // Read the slice size if available.
            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) != 0)
            {
                _current.SliceSize = ReadSliceSize11();
            }
            else
            {
                _current.SliceSize = 0;
            }

            // Clear other per-slice fields:
            _current.IndirectionTable = null;
            _current.PosAfterIndirectionTable = null;

            return (typeId, compactId);
        }

        /// <summary>Reads the size of the current slice.</summary>
        /// <returns>The slice of the current slice, not including the size length.</returns>
        private int ReadSliceSize11()
        {
            int size = ReadInt();
            if (size < 4)
            {
                throw new InvalidDataException($"invalid slice size: {size}");
            }
            // With the 1.1 encoding, the encoded size includes the size length.
            return size - 4;
        }

        /// <summary>Reads the type ID of a class instance.</summary>
        /// <param name="typeIdKind">The kind of type ID to read.</param>
        /// <returns>The type ID and the compact ID, if any.</returns>
        private (string? TypeId, int? CompactId) ReadTypeId11(EncodingDefinitions.TypeIdKind typeIdKind)
        {
            _typeIdMap11 ??= new List<string>();

            switch (typeIdKind)
            {
                case EncodingDefinitions.TypeIdKind.Index:
                    int index = ReadSize();
                    if (index > 0 && index - 1 < _typeIdMap11.Count)
                    {
                        // The encoded type-id indexes start at 1, not 0.
                        return (_typeIdMap11[index - 1], null);
                    }
                    throw new InvalidDataException($"read invalid type ID index {index}");

                case EncodingDefinitions.TypeIdKind.String:
                    string typeId = ReadString();

                    // The typeIds of slices in indirection tables can be read several times: when we skip the
                    // indirection table and later on when we read it. We only want to add this typeId to the list and
                    // assign it an index when it's the first time we read it, so we save the largest position we read
                    // to figure out when to add to the list.
                    if (Pos > _posAfterLatestInsertedTypeId11)
                    {
                        _posAfterLatestInsertedTypeId11 = Pos;
                        _typeIdMap11.Add(typeId);
                    }
                    return (typeId, null);

                case EncodingDefinitions.TypeIdKind.CompactId11:
                    return (null, ReadSize());

                default:
                    // TypeIdKind has only 4 possible values.
                    Debug.Assert(typeIdKind == EncodingDefinitions.TypeIdKind.None);
                    return (null, null);
            }
        }

        /// <summary>Skips the indirection table. The caller must save the current stream position before calling
        /// SkipIndirectionTable11 (to read the indirection table at a later point) except when the caller is
        /// SkipIndirectionTable11 itself.</summary>
        private void SkipIndirectionTable11()
        {
            Debug.Assert(Communicator != null);
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
                    if (++_classGraphDepth > Communicator.ClassGraphMaxDepth)
                    {
                        throw new InvalidDataException("maximum class graph depth reached");
                    }

                    // Read/skip this instance
                    EncodingDefinitions.SliceFlags sliceFlags;
                    do
                    {
                        sliceFlags = (EncodingDefinitions.SliceFlags)ReadByte();

                        // Skip type ID - can update _typeIdMap11
                        _ = ReadTypeId11(sliceFlags.GetTypeIdKind());

                        // Read the slice size, then skip the slice
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0)
                        {
                            throw new InvalidDataException("size of slice missing");
                        }
                        int sliceSize = ReadSliceSize11();
                        Pos += sliceSize; // we need a temporary sliceSize because ReadSliceSize11 updates _pos.

                        // If this slice has an indirection table, skip it too.
                        if ((sliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0)
                        {
                            SkipIndirectionTable11();
                        }
                    }
                    while ((sliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) == 0);
                    _classGraphDepth--;
                }
            }
        }

        /// <summary>Skips and saves the body of the current slice; also skips and save the indirection table (if any).
        /// </summary>
        /// <param name="typeId">The type ID of the current slice.</param>
        /// <param name="compactId">The compact ID of the current slice.</param>
        /// <returns>True when the current slice is the last slice; otherwise, false.</returns>
        private bool SkipSlice(string? typeId, int? compactId = null)
        {
            Debug.Assert(Communicator != null);

            // With the 2.0 encoding, typeId is not null and compactId is always null.
            // With the 1.1 encoding, they are potentially both null (but this will result in an exception below).
            Debug.Assert(OldEncoding || (typeId != null && compactId == null));

            if (typeId == null && compactId == null)
            {
                throw new InvalidDataException("cannot skip a class slice with no type ID");
            }

            if ((_current.SliceFlags & EncodingDefinitions.SliceFlags.HasSliceSize) == 0)
            {
                string printableId = typeId ?? compactId?.ToString() ?? "(none)";
                string kind = _current.InstanceType.ToString().ToLowerInvariant();
                throw new InvalidDataException(@$"no {kind} found for type ID `{printableId
                        }' and compact format prevents slicing (the sender should use the sliced format instead)");
            }

            if (Communicator.TraceLevels.Slicing > 0)
            {
                string printableId = typeId ?? compactId?.ToString() ?? "(none)";
                string kind = _current.InstanceType.ToString().ToLowerInvariant();
                Communicator.Logger.Trace(TraceLevels.SlicingCategory,
                    $"slicing unknown {kind} type `{printableId}'");
            }

            bool hasTaggedMembers = (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasTaggedMembers) != 0;

            byte[] bytes;
            int bytesCopied;
            if (hasTaggedMembers)
            {
                // Don't include the tag end marker. It will be re-written by IceEndSlice when the sliced data is
                // re-written.
                bytes = new byte[_current.SliceSize - 1];
                bytesCopied = ReadSpan(bytes);
                Skip(1);
            }
            else
            {
                bytes = new byte[_current.SliceSize];
                bytesCopied = ReadSpan(bytes);
            }

            if (bytesCopied != bytes.Length)
            {
                throw new InvalidDataException("the slice size extends beyond the end of the buffer");
            }

            bool hasIndirectionTable = (_current.SliceFlags & EncodingDefinitions.SliceFlags.HasIndirectionTable) != 0;

            // With the 1.1 encoding, SkipSlice for a class skips the indirection table and preserves its position in
            // _current.DeferredIndirectionTableList11 for later reading.
            // For exceptions and with the 2.0 encoding, we always read the indirection table before calling SkipSlice
            // (if there is an indirection table), hence no need for a DeferredIndirectionTableList.
            if (OldEncoding && _current.InstanceType == InstanceType.Class)
            {
                _current.DeferredIndirectionTableList11 ??= new List<int>();
                if (hasIndirectionTable)
                {
                    int savedPos = Pos;
                    SkipIndirectionTable11();
                    _current.DeferredIndirectionTableList11.Add(savedPos); // we want to later read the deepest first
                }
                else
                {
                    _current.DeferredIndirectionTableList11.Add(0); // keep a slot for each slice
                }
            }
            else if (hasIndirectionTable)
            {
                Debug.Assert(_current.PosAfterIndirectionTable != null);
                // Move past indirection table
                Pos = _current.PosAfterIndirectionTable.Value;
                _current.PosAfterIndirectionTable = null;
            }

            _current.Slices ??= new List<SliceInfo>();
            var info = new SliceInfo(typeId ?? "",
                                     compactId,
                                     new ReadOnlyMemory<byte>(bytes),
                                     Array.AsReadOnly(_current.IndirectionTable ?? Array.Empty<AnyClass>()),
                                     hasTaggedMembers);
            _current.Slices.Add(info);

            // If we read the indirection table previously, we don't need it anymore since we're skipping this slice.
            _current.IndirectionTable = null;

            return (_current.SliceFlags & EncodingDefinitions.SliceFlags.IsLastSlice) != 0;
        }

        private struct InstanceData
        {
            // Instance fields

            internal List<int>? DeferredIndirectionTableList11;
            internal InstanceType InstanceType;
            internal List<SliceInfo>? Slices; // Preserved slices.

            // Slice fields

            internal AnyClass[]? IndirectionTable; // Indirection table of the current slice
            internal int? PosAfterIndirectionTable;

            internal EncodingDefinitions.SliceFlags SliceFlags;
            internal int SliceSize;
        }

        private enum InstanceType : byte
        {
            None = 0,
            Class,
            Exception
        }
    }
}
