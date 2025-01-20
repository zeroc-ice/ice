classdef (Sealed) ConnectionInfo < Ice.ConnectionInfo
    % ConnectionInfo   Summary of ConnectionInfo
    %
    % Provides access to the connection details of an SSL connection.
    %
    % ConnectionInfo Properties:
    %   peerCertificate - The peer certificate.

    %  Copyright (c) ZeroC, Inc.

    methods
        function obj = ConnectionInfo(underlying, peerCertificate)
            assert(nargin == 2, 'Invalid number of arguments');
            obj@Ice.ConnectionInfo(underlying);
            obj.peerCertificate = peerCertificate;
        end
    end
    properties(SetAccess=immutable)
        % peerCertificate - The peer certificate.
        peerCertificate
    end
end
