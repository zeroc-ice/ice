%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

%
% SliceInfo encapsulates the details of a slice for an unknown class or exception type.
%
classdef SliceInfo < handle
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
