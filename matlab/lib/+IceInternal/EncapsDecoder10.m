%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EncapsDecoder10 < IceInternal.EncapsDecoder
    methods
        function obj = EncapsDecoder10(is, encaps)
            obj = obj@IceInternal.EncapsDecoder(is, encaps);
            obj.sliceType = IceInternal.SliceType.NoSlice;
        end
        function throwException(obj)
            assert(obj.sliceType == IceInternal.SliceType.NoSlice);

            %
            % User exception with the 1.0 encoding start with a boolean flag
            % that indicates whether or not the exception has classes.
            %
            % This allows reading the pending values even if some part of
            % the exception was sliced.
            %
            usesClasses = obj.is.readBool();

            obj.sliceType = IceInternal.SliceType.ExceptionSlice;
            obj.skipFirstSlice = false;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.typeId;
            while true
                %
                % Translate the type ID into a class name.
                %
                cls = Ice.Util.idToClass(obj.typeId);

                %
                % Try to instantiate the class.
                %
                ex = [];
                try
                    ex = eval(cls);
                catch e
                    %
                    % Instantiation failed.
                    %
                end

                if ~isempty(ex)
                    ex = ex.read_(obj.is);
                    if usesClasses
                        obj.readPendingValues();
                    end
                    throw(ex);
                else
                    %
                    % Slice off what we don't understand.
                    %
                    obj.skipSlice();
                    try
                        obj.startSlice();
                    catch ex
                        if isa(ex, 'Ice.UnmarshalOutOfBoundsException')
                            %
                            % An oversight in the 1.0 encoding means there is no marker to indicate
                            % the last slice of an exception. As a result, we just try to read the
                            % next type ID, which raises UnmarshalOutOfBoundsException when the
                            % input buffer underflows.
                            %
                            % Set the reason member to a more helpful message.
                            %
                            ex.reason = ['unknown exception type `', mostDerivedId, ''''];
                        end
                        rethrow(ex);
                    end
                end
            end
        end
        function startInstance(obj, sliceType)
            assert(obj.sliceType == sliceType);
            obj.skipFirstSlice = true;
        end
        function endInstance(obj) % TODO: SlicedData
            %
            % Read the Ice::Value slice.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                obj.startSlice();
                sz = obj.is.readSize(); % For compatibility with the old AFM.
                if sz != 0
                    throw(Ice.MarshalException('', '', "invalid Object slice"));
                end
                obj.endSlice();
            end
            obj.sliceType = IceInternal.SliceType.NoSlice;
        end
        function r = startSlice(obj)
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the type.
            %
            if obj.skipFirstSlice
                obj.skipFirstSlice = false;
                r = obj.typeId;
                return;
            end

            %
            % For values, first read the type ID boolean which indicates
            % whether or not the type ID is encoded as a string or as an
            % index. For exceptions, the type ID is always encoded as a
            % string.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                isIndex = obj.is.readBool();
                obj.typeId = obj.readTypeId(isIndex);
            else
                obj.typeId = obj.is.readString();
            end

            obj.sliceSize = obj.is.readInt();
            if obj.sliceSize < 4
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = obj.typeId;
        end
        function endSlice(obj)
            % Nothing to do
        end
        function skipSlice(obj)
            %obj.is.traceSkipSlice(obj.typeId, obj.sliceType);
            assert(obj.sliceSize >= 4);
            obj.is.skip(obj.sliceSize - 4);
        end
        function r = readOptional(obj, tag, format)
            r = false;
        end
    end
    properties(Access=private)
        sliceType
        skipFirstSlice
        sliceSize
        typeId
    end
end
