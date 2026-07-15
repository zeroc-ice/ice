classdef EndpointSelectionType < uint8
    %ENDPOINTSELECTIONTYPE Determines how the Ice runtime sorts proxy endpoints when establishing a connection.
    %
    %   EndpointSelectionType Properties:
    %     Random - The Ice runtime shuffles the endpoints in a random order.
    %     Ordered - The Ice runtime uses the endpoints in the order they appear in the proxy.

    % Copyright (c) ZeroC, Inc.

    enumeration
        %RANDOM The Ice runtime shuffles the endpoints in a random order.
        Random (0)

        %ORDERED The Ice runtime uses the endpoints in the order they appear in the proxy.
        Ordered (1)
    end
    methods (Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.EndpointSelectionType.Random;
                case 1
                    r = Ice.EndpointSelectionType.Ordered;
                otherwise
                    throw(Ice.MarshalException(sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
