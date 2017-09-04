%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function [communicator, args] = initialize(varargin)
    if length(varargin) >= 1 && ~isempty(varargin{1})
        args = varargin{1};
    else
        args = {};
    end

    if length(varargin) >= 2 && ~isempty(varargin{2})
        if ~isa(varargin{2}, 'Ice.InitializationData')
            throw(MException('Ice:ArgumentException', 'initData argument must be an Ice.InitializationData object'));
        end
        initData = varargin{2};
    else
        initData = [];
    end

    %
    % Implementation note: We need to extract and pass the libpointer object for properties to the C function.
    % Passing the wrapper (Ice.Properties) object won't work because the C code has no way to obtain the
    % inner pointer.
    %
    props = libpointer('voidPtr');
    if ~isempty(initData)
        if ~isempty(initData.properties_)
            if ~isa(initData.properties_, 'Ice.Properties')
                throw(MException('Ice:ArgumentException', 'invalid value for properties_ member'));
            else
                props = initData.properties_.impl;
            end
        end
    end

    impl = libpointer('voidPtr');
    args = Ice.Util.callWithResult('Ice_initialize', args, props, impl);
    communicator = Ice.Communicator(impl);
end
