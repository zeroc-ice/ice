# Copyright (c) ZeroC, Inc.

require 'Ice'

def oneways(helper, p)
    p = Test::MyClassPrx.uncheckedCast(p.ice_oneway())

    # Calling a ["oneway"] operation on a oneway proxy succeeds.
    p.opOneway()

    # Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
    begin
        Test::MyClassPrx.uncheckedCast(p.ice_twoway()).opOneway()
        test(false)
    rescue Ice::OnewayOnlyException
    end
end
