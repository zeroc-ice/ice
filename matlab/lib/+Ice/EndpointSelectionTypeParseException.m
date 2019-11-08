% EndpointSelectionTypeParseException   Summary of EndpointSelectionTypeParseException
%
% This exception is raised if there was an error while parsing an
% endpoint selection type.
%
% EndpointSelectionTypeParseException Properties:
%   str - Describes the failure and includes the string that could not be parsed.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef EndpointSelectionTypeParseException < Ice.LocalException
    properties
        % str - Describes the failure and includes the string that could not be parsed.
        str char
    end
    methods
        function obj = EndpointSelectionTypeParseException(ice_exid, ice_exmsg, str)
            if nargin <= 2
                str = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:EndpointSelectionTypeParseException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.EndpointSelectionTypeParseException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.str = str;
        end
        function id = ice_id(~)
            id = '::Ice::EndpointSelectionTypeParseException';
        end
    end
end
