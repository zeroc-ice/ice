classdef (Sealed) SliceInfo < handle
    %SLICEINFO Encapsulates the details of a class slice with an unknown type.
    %
    %   SliceInfo Properties:
    %     typeId - The Slice type ID for this slice.
    %     compactId - The Slice compact type ID for this slice.
    %     bytes - The encoded bytes for this slice, including the leading size integer.
    %     hasOptionalMembers - Whether or not the slice contains optional members.
    %     isLastSlice - Whether or not this is the last slice.
    %     instances - The class instances referenced by this slice.

    % Copyright (c) ZeroC, Inc.

    properties (SetAccess = immutable)
        %TYPEID The Slice type ID for this slice.
        %   character vector
        typeId (1, :) char

        %COMPACTID The Slice compact type ID for this slice.
        %   int32 scalar
        compactId (1, 1) int32

        %BYTES The encoded bytes for this slice, including the leading size integer.
        %   uint8 vector
        bytes (1, :) uint8

        %HASOPTIONALMEMBERS Whether or not the slice contains optional members.
        %   logical scalar
        hasOptionalMembers (1, 1) logical

        %ISLASTSLICE Whether or not this is the last slice.
        %   logical scalar
        isLastSlice (1, 1) logical
    end
    properties
        %INSTANCES The class instances referenced by this slice.
        %   cell array of Ice.Value
        instances (1, :) cell = {}
    end
    methods (Hidden)
        function obj = SliceInfo(typeId, compactId, bytes, hasOptionalMembers, isLastSlice)
            obj.typeId = typeId;
            obj.compactId = compactId;
            obj.bytes = bytes;
            obj.hasOptionalMembers = hasOptionalMembers;
            obj.isLastSlice = isLastSlice;
        end
    end
end
