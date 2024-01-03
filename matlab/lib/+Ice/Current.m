% Current   Summary of Current
%
% Information about the current method invocation for servers. Each operation on the server has a
% Current as its implicit final parameter. Current is mostly used for Ice services. Most
% applications ignore this parameter.
%
% Current Properties:
%   adapter - The object adapter.
%   con - Information about the connection over which the current method invocation was received.
%   id - The Ice object identity.
%   facet - The facet.
%   operation - The operation name.
%   mode - The mode of the operation.
%   ctx - The request context, as received from the client.
%   requestId - The request id unless oneway (0).
%   encoding - The encoding version used to encode the input and output parameters.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Current.ice by slice2matlab version 3.7.10

classdef Current
    properties
        % adapter - The object adapter.
        adapter
        % con - Information about the connection over which the current method invocation was received. If the invocation is
        % direct due to collocation optimization, this value is set to null.
        con
        % id - The Ice object identity.
        id Ice.Identity
        % facet - The facet.
        facet char
        % operation - The operation name.
        operation char
        % mode - The mode of the operation.
        mode Ice.OperationMode
        % ctx - The request context, as received from the client.
        ctx containers.Map
        % requestId - The request id unless oneway (0).
        requestId int32
        % encoding - The encoding version used to encode the input and output parameters.
        encoding Ice.EncodingVersion
    end
    methods
        function obj = Current(adapter, con, id, facet, operation, mode, ctx, requestId, encoding)
            if nargin == 0
                obj.adapter = [];
                obj.con = [];
                obj.id = Ice.Identity();
                obj.facet = '';
                obj.operation = '';
                obj.mode = Ice.OperationMode.Normal;
                obj.ctx = containers.Map('KeyType', 'char', 'ValueType', 'char');
                obj.requestId = 0;
                obj.encoding = Ice.EncodingVersion();
            elseif ne(adapter, IceInternal.NoInit.Instance)
                obj.adapter = adapter;
                obj.con = con;
                obj.id = id;
                obj.facet = facet;
                obj.operation = operation;
                obj.mode = mode;
                obj.ctx = ctx;
                obj.requestId = requestId;
                obj.encoding = encoding;
            end
        end
        function r = eq(obj, other)
            r = isequal(obj, other);
        end
        function r = ne(obj, other)
            r = ~isequal(obj, other);
        end
        function obj = ice_convert(obj)
            obj.adapter = obj.adapter.value;
            obj.con = obj.con.value;
        end
    end
end
