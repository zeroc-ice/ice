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
        typeId
        %
        % The Slice compact type ID for this slice.
        %
        compactId
        %
        % The encoded bytes for this slice, including the leading size integer.
        %
        bytes
        %
        % Whether or not the slice contains optional members.
        %
        hasOptionalMembers
        %
        % Whether or not this is the last slice.
        %
        isLastSlice
    end
    properties
        %
        % The class instances referenced by this slice.
        %
        instances
    end
    methods
        function obj = SliceInfo(typeId, compactId, bytes, hasOptionalMembers, isLastSlice)
            obj.typeId = typeId;
            obj.compactId = compactId;
            obj.bytes = bytes;
            obj.hasOptionalMembers = hasOptionalMembers;
            obj.isLastSlice = isLastSlice;
            obj.instances = [];
        end
    end
end
