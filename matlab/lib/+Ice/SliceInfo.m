classdef SliceInfo < handle
    % SliceInfo   Summary of SliceInfo
    %
    % SliceInfo encapsulates the details of a slice for an unknown class or
    % exception type.
    %
    % SliceInfo Properties:
    %   typeId - The Slice type ID for this slice.
    %   compactId - The Slice compact type ID for this slice.
    %   bytes - The encoded bytes for this slice, including the leading size
    %     integer.
    %   instances - The class instances referenced by this slice.
    %   hasOptionalMembers - Whether or not the slice contains optional members.
    %   isLastSlice - Whether or not this is the last slice.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    properties
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
        % The class instances referenced by this slice.
        %
        instances
        %
        % Whether or not the slice contains optional members.
        %
        hasOptionalMembers
        %
        % Whether or not this is the last slice.
        %
        isLastSlice
    end
end
