classdef (Sealed) ConnectionInfo < Ice.ConnectionInfo
    %CONNECTIONINFO Provides access to the connection details of an SSL connection.
    %
    %   ConnectionInfo Properties:
    %     peerCertificate - The peer certificate.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = ConnectionInfo(underlying, peerCertificate)
            assert(nargin == 2, 'Invalid number of arguments');
            obj@Ice.ConnectionInfo(underlying);
            obj.peerCertificate = peerCertificate;
        end
    end
    properties (SetAccess = immutable)
        %PEERCERTIFICATE The peer certificate.
        %   character vector
        peerCertificate (1, :) char
    end
end
