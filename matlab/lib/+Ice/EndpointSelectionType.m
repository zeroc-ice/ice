% EndpointSelectionType   Summary of EndpointSelectionType
%
% Determines the order in which the Ice run time uses the endpoints
% in a proxy when establishing a connection.
%
% EndpointSelectionType Properties:
%   Random - Random causes the endpoints to be arranged in a random order.
%   Ordered - Ordered forces the Ice run time to use the endpoints in the order they appeared in the proxy.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from EndpointTypes.ice by slice2matlab version 3.7.3

classdef EndpointSelectionType < uint8
    enumeration
        % Random causes the endpoints to be arranged in a random order.
        Random (0)
        % Ordered forces the Ice run time to use the endpoints in the
        % order they appeared in the proxy.
        Ordered (1)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.EndpointSelectionType.Random;
                case 1
                    r = Ice.EndpointSelectionType.Ordered;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
