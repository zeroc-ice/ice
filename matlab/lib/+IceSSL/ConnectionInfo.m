classdef ConnectionInfo < Ice.ConnectionInfo
    % ConnectionInfo   Summary of ConnectionInfo
    %
    % Provides access to the connection detaisl of an SSL connection.
    %
    % ConnectionInfo Properties:
    %   cipher - The negotiated cipher suite.
    %   certs - The certificate chain.
    %   verified - The certificate chain verification status.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    methods
        function obj = ConnectionInfo(underlying, incoming, adapterName, connectionId, cipher, ...
                                      certs, verified)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                cipher = '';
                certs = [];
                verified = false;
            end
            obj = obj@Ice.ConnectionInfo(underlying, incoming, adapterName, connectionId);
            obj.cipher = cipher;
            obj.certs = certs;
            obj.verified = verified;
        end
    end
    properties(SetAccess=private)
        % cipher - The negotiated cipher suite.
        cipher char

        % certs - The certificate chain.
        certs

        % verified - The certificate chain verification status.
        verified logical
    end
end
