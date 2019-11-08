% ProtocolVersion   Summary of ProtocolVersion
%
% A version structure for the protocol version.
%
% ProtocolVersion Properties:
%   major
%   minor

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Version.ice by slice2matlab version 3.7.3

classdef ProtocolVersion
    properties
        major uint8
        minor uint8
    end
    methods
        function obj = ProtocolVersion(major, minor)
            if nargin > 0
                obj.major = major;
                obj.minor = minor;
            else
                obj.major = 0;
                obj.minor = 0;
            end
        end
        function r = eq(obj, other)
            r = isequal(obj, other);
        end
        function r = ne(obj, other)
            r = ~isequal(obj, other);
        end
    end
    methods(Static)
        function r = ice_read(is)
            r = Ice.ProtocolVersion();
            r.major = is.readByte();
            r.minor = is.readByte();
        end
        function r = ice_readOpt(is, tag)
            if is.readOptional(tag, Ice.OptionalFormat.VSize)
                is.skipSize();
                r = Ice.ProtocolVersion.ice_read(is);
            else
                r = Ice.Unset;
            end
        end
        function ice_write(os, v)
            if isempty(v)
                v = Ice.ProtocolVersion();
            end
            os.writeByte(v.major);
            os.writeByte(v.minor);
        end
        function ice_writeOpt(os, tag, v)
            if v ~= Ice.Unset && os.writeOptional(tag, Ice.OptionalFormat.VSize)
                os.writeSize(2);
                Ice.ProtocolVersion.ice_write(os, v);
            end
        end
    end
end
