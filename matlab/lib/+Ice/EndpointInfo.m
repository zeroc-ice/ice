%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EndpointInfo < handle
    methods
        function obj = EndpointInfo(type, datagram, secure, underlying, timeout, compress)
            if nargin == 3
                underlying = [];
                timeout = 0;
                compress = false;
            end
            obj.type_ = type;
            obj.datagram_ = datagram;
            obj.secure_ = secure;
            obj.underlying = underlying;
            obj.timeout = timeout;
            obj.compress = compress;
        end
        function r = type(obj)
            if ~isempty(obj.underlying)
                r = obj.underlying.type();
            else
                r = obj.type_;
            end
        end
        function r = datagram(obj)
            if ~isempty(obj.underlying)
                r = obj.underlying.datagram();
            else
                r = obj.datagram_;
            end
        end
        function r = secure(obj)
            if ~isempty(obj.underlying)
                r = obj.underlying.secure();
            else
                r = obj.secure_;
            end
        end
    end
    properties(SetAccess=private)
        underlying
        timeout int32
        compress logical
    end
    properties(Access=protected)
        type_
        datagram_
        secure_
    end
end
