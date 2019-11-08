% NotRegisteredException   Summary of NotRegisteredException
%
% An attempt was made to find or deregister something that is not
% registered with the Ice run time or Ice locator.
%
% This exception is raised if an attempt is made to remove a servant,
% servant locator, facet, value factory, plug-in, object adapter,
% object, or user exception factory that is not currently registered.
%
% It's also raised if the Ice locator can't find an object or object
% adapter when resolving an indirect proxy or when an object adapter
% is activated.
%
% NotRegisteredException Properties:
%   kindOfObject - The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
%   id - The ID (or name) of the object that could not be removed.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef NotRegisteredException < Ice.LocalException
    properties
        % kindOfObject - The kind of object that could not be removed: "servant", "facet",
        % "object", "default servant", "servant locator", "value factory", "plugin",
        % "object adapter", "object adapter with router", "replica group".
        kindOfObject char
        % id - The ID (or name) of the object that could not be removed.
        id char
    end
    methods
        function obj = NotRegisteredException(ice_exid, ice_exmsg, kindOfObject, id)
            if nargin <= 2
                kindOfObject = '';
                id = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:NotRegisteredException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.NotRegisteredException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.kindOfObject = kindOfObject;
            obj.id = id;
        end
        function id = ice_id(~)
            id = '::Ice::NotRegisteredException';
        end
    end
end
