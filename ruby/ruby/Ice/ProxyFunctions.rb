# Copyright (c) ZeroC, Inc.

require_relative 'Proxy.rb'
require_relative 'OperationMode.rb'
require_relative 'BuiltinSequences.rb'

module ::Ice
    module ObjectPrx_mixin
        def ice_ping(context=nil)
            ObjectPrx_mixin::OP_ice_ping.invoke(self, [], context)
        end

        def ice_ids(context=nil)
            return ObjectPrx_mixin::OP_ice_ids.invoke(self, [], context)
        end

        def ice_isA(id, context=nil)
            return ObjectPrx_mixin::OP_ice_isA.invoke(self, [id], context)
        end

        def ice_id(context=nil)
            ObjectPrx_mixin::OP_ice_id.invoke(self, [], context)
        end
    end

    class ObjectPrx
        include ObjectPrx_mixin
    end

    ObjectPrx_mixin::OP_ice_ping = Ice::__defineOperation(
        'ice_ping',
        'ice_ping',
        Ice::OperationMode::Idempotent,
        nil,
        [],
        [],
        nil,
        [])

    ObjectPrx_mixin::OP_ice_ids = Ice::__defineOperation(
        'ice_ids',
        'ice_ids',
        Ice::OperationMode::Idempotent,
        nil,
        [],
        [],
        [::Ice::T_StringSeq, false, 0],
        [])

    ObjectPrx_mixin::OP_ice_id = Ice::__defineOperation(
        'ice_id',
        'ice_id',
        Ice::OperationMode::Idempotent,
        nil,
        [],
        [],
        [Ice::T_string, false, 0],
        [])

    ObjectPrx_mixin::OP_ice_isA = Ice::__defineOperation(
        'ice_isA',
        'ice_isA',
        Ice::OperationMode::Idempotent,
        nil,
        [[Ice::T_string, false, 0]],
        [],
        [Ice::T_bool, false, 0],
        [])
end
