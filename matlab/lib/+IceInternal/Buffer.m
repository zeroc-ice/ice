%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Buffer < matlab.mixin.Copyable
    methods
        function obj = Buffer(data)
            if nargin == 1
                %
                % Caller provided a uint8 array to be used for unmarshaling.
                %
                obj.buf = data;
                obj.size = length(data);
                obj.capacity = obj.size;
            else
                %
                % This buffer will be used for marshaling and grows as necessary.
                %
                % Use zeros() here. Using "uint8(0)" isn't sufficient.
                %
                obj.buf = zeros(1, 1024, 'uint8');
                obj.size = 0;
                obj.capacity = length(obj.buf);
            end
        end
        function reset(obj, data)
            if nargin == 2
                obj.buf = data;
                obj.size = length(data);
                obj.capacity = obj.size;
            else
                %
                % Reuse the existing buffer.
                %
                obj.size = 0;
            end
        end
        function expand(obj, n)
            obj.resize(obj.size + n);
        end
        function resize(obj, n)
            if n > obj.capacity
                obj.capacity = max(n, 2 * obj.capacity);
                obj.buf(obj.capacity) = uint8(0); % Expand the array
            end
            obj.size = n;
        end
        function pushByte(obj, b)
            s = obj.size + 1;
            if s <= obj.capacity
                obj.buf(s) = b;
                obj.size = s;
            else
                obj.resize(s);
                obj.buf(s) = b;
            end
        end
        function push(obj, b)
            n = length(b);
            if n == 1
                obj.pushByte(b);
            else
                sz = obj.size;
                if sz + n <= obj.capacity
                    obj.buf(sz + 1:sz + n) = b;
                    obj.size = obj.size + n;
                else
                    obj.resize(sz + n);
                    obj.buf(sz + 1:sz + n) = b;
                end
            end
        end
    end
    properties
        %
        % Public properties to reduce method calls
        %
        buf
        size uint32
        capacity uint32
    end
end
