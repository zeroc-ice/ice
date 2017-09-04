%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function [properties, remArgs] = createProperties(varargin)
    if length(varargin) >= 1 && ~isempty(varargin{1})
        args = varargin{1};
    else
        args = {};
    end
    if length(varargin) >= 2 && ~isempty(varargin{2})
        if ~isa(varargin{2}, 'Ice.Properties')
            throw(MException('Ice:ArgumentException', 'expecting Ice.Properties object'));
        end
        defaults = varargin{2}.impl;
    else
        defaults = libpointer('voidPtr');
    end
    impl = libpointer('voidPtr');
    remArgs = Ice.Util.callWithResult('Ice_createProperties', args, defaults, impl);
    properties = Ice.Properties(impl);
end
