classdef SliceInfo < handle
    % SliceInfo   Summary of SliceInfo
    %
    % Encapsulates the details of a slice with an unknown type.
    %
    % SliceInfo Properties:
    %   typeId - The Slice type ID for this slice.
    %   compactId - The Slice compact type ID for this slice.
    %   bytes - The encoded bytes for this slice, including the leading size
    %     integer.
    %   instances - The class instances referenced by this slice.
    %   hasOptionalMembers - Whether or not the slice contains optional members.
    %   isLastSlice - Whether or not this is the last slice.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        %
        % The Slice type ID for this slice.
        %
        typeId (1, :) char
        %
        % The Slice compact type ID for this slice.
        %
        compactId (1, 1) int32
        %
        % The encoded bytes for this slice, including the leading size integer.
        %
        bytes (1, :) uint8
        %
        % Whether or not the slice contains optional members.
        %
        hasOptionalMembers (1, 1) logical
        %
        % Whether or not this is the last slice.
        %
        isLastSlice (1, 1) logical
    end
    properties
        %
        % The class instances referenced by this slice.
        %
        instances (1, :) cell = {}
    end
    methods
        function obj = SliceInfo(typeId, compactId, bytes, hasOptionalMembers, isLastSlice)
            obj.typeId = typeId;
            obj.compactId = compactId;
            obj.bytes = bytes;
            obj.hasOptionalMembers = hasOptionalMembers;
            obj.isLastSlice = isLastSlice;
        end
    end
end
