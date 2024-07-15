classdef ConnectionInfo < Ice.ConnectionInfo
    % ConnectionInfo   Summary of ConnectionInfo
    %
    % Provides access to the connection detaisl of an SSL connection.
    %
    % ConnectionInfo Properties:
    %   peerCertificate - The peer certificate.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = ConnectionInfo(underlying, incoming, adapterName, connectionId, ...
                                      peerCertificate)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                peerCertificate = '';
            end
            obj@Ice.ConnectionInfo(underlying, incoming, adapterName, connectionId);
            obj.peerCertificate = peerCertificate;
        end
    end
    properties(SetAccess=private)
        % peerCertificate - The peer certificate.
        peerCertificate
    end
end
