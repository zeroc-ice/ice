% ParseException   Summary of ParseException
%
% This exception is raised if there was an error while parsing a string.
%
% ParseException Properties:
%   str - Describes the failure and includes the string that could not be parsed.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ParseException < Ice.LocalException
    properties
        % str - Describes the failure and includes the string that could not be parsed.
        str char
    end
    methods
        function obj = ParseException(ice_exid, ice_exmsg, str)
            if nargin <= 2
                str = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ParseException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ParseException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.str = str;
        end
        function id = ice_id(~)
            id = '::Ice::ParseException';
        end
    end
end
