# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

class ObjectFactoryI
    def create(id)
        return nil
    end

    def destroy()
    end
end

def allTests(communicator)
    platformInfo = RUBY_PLATFORM.split("-")
    isWin32 = false
    if platformInfo.length >= 2 && platformInfo[1] == "mswin32"
        isWin32 = true
    end

    print "testing object factory registration exception... "
    STDOUT.flush
    of = ObjectFactoryI.new
    communicator.addObjectFactory(of, "x")
    begin
        communicator.addObjectFactory(of, "x")
        test(false)
    rescue Ice::AlreadyRegisteredException
    end
    puts "ok"

    print "testing stringToProxy... "
    STDOUT.flush
    ref = "thrower:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    thrower = Test::ThrowerPrx::checkedCast(base)
    test(thrower)
    test(thrower == base)
    puts "ok"

    print "catching exact types... "
    STDOUT.flush

    begin
        thrower.throwAasA(1)
        test(false)
    rescue Test::A => ex
        test(ex.aMem == 1)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwAorDasAorD(1)
        test(false)
    rescue Test::A => ex
        test(ex.aMem == 1)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwAorDasAorD(-1)
        test(false)
    rescue Test::D => ex
        test(ex.dMem == -1)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwBasB(1, 2)
        test(false)
    rescue Test::B => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwCasC(1, 2, 3)
        test(false)
    rescue Test::C => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    #
    # We cannot invoke throwModA if the server was built with VC6.
    #
    if !isWin32
        begin
            thrower.throwModA(1, 2)
            test(false)
        rescue Test::Mod::A => ex
            test(ex.aMem == 1)
            test(ex.a2Mem == 2)
        rescue Ice::OperationNotExistException
            #
            # This operation is not supported in Java.
            #
        rescue
            print $!
            print $!.backtrace.join("\n")
            test(false)
        end
    end

    puts "ok"

    print "catching base types... "
    STDOUT.flush

    begin
        thrower.throwBasB(1, 2)
        test(false)
    rescue Test::A => ex
        test(ex.aMem == 1)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwCasC(1, 2, 3)
        test(false)
    rescue Test::B => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    #
    # We cannot invoke throwModA if the server was built with VC6.
    #
    if !isWin32
        begin
            thrower.throwModA(1, 2)
            test(false)
        rescue Test::A => ex
            test(ex.aMem == 1)
        rescue Ice::OperationNotExistException
            #
            # This operation is not supported in Java.
            #
        rescue
            print $!.backtrace.join("\n")
            test(false)
        end
    end

    puts "ok"

    print "catching derived types... "
    STDOUT.flush

    begin
        thrower.throwBasA(1, 2)
        test(false)
    rescue Test::B => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwCasA(1, 2, 3)
        test(false)
    rescue Test::C => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwCasB(1, 2, 3)
        test(false)
    rescue Test::C => ex
        test(ex.aMem == 1)
        test(ex.bMem == 2)
        test(ex.cMem == 3)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    if thrower.supportsUndeclaredExceptions()
        print "catching unknown user exception... "
        STDOUT.flush

        begin
            thrower.throwUndeclaredA(1)
            test(false)
        rescue Ice::UnknownUserException
            #
            # We get an unknown user exception without collocation
            # optimization.
            #
        rescue
            print $!.backtrace.join("\n")
            test(false)
        end

        begin
            thrower.throwUndeclaredB(1, 2)
            test(false)
        rescue Ice::UnknownUserException
            #
            # We get an unknown user exception without collocation
            # optimization.
            #
        rescue
            print $!.backtrace.join("\n")
            test(false)
        end

        begin
            thrower.throwUndeclaredC(1, 2, 3)
            test(false)
        rescue Ice::UnknownUserException
            #
            # We get an unknown user exception without collocation
            # optimization.
            #
        rescue
            print $!.backtrace.join("\n")
            test(false)
        end

        puts "ok"
    end

    print "testing memory limit marshal exception..."
    STDOUT.flush

    begin
        thrower.throwMemoryLimitException(Array.new(1, 0x00));
        test(false)
    rescue Ice::MemoryLimitException
        # Expected
    rescue
        test(false)
    end

    begin
        thrower.throwMemoryLimitException(Array.new(20 * 1024, 0x00)) # 20KB
        test(false)
    rescue Ice::ConnectionLostException
        # Expected
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    print "catching object not exist exception... "
    STDOUT.flush

    id = communicator.stringToIdentity("does not exist")
    begin
        thrower2 = Test::ThrowerPrx::uncheckedCast(thrower.ice_identity(id))
        thrower2.throwAasA(1)
#       thrower2.ice_ping()
        test(false)
    rescue Ice::ObjectNotExistException => ex
        test(ex.id == id)
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    print "catching facet not exist exception... "
    STDOUT.flush

    begin
        thrower2 = Test::ThrowerPrx::uncheckedCast(thrower, "no such facet")
        begin
            thrower2.ice_ping()
            test(false)
        rescue Ice::FacetNotExistException => ex
            test(ex.facet == "no such facet")
        end
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    print "catching operation not exist exception... "
    STDOUT.flush

    begin
        thrower2 = Test::WrongOperationPrx::uncheckedCast(thrower)
        thrower2.noSuchOperation()
        test(false)
    rescue Ice::OperationNotExistException => ex
        test(ex.operation == "noSuchOperation")
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    print "catching unknown local exception... "
    STDOUT.flush

    begin
        thrower.throwLocalException()
        test(false)
    rescue Ice::UnknownLocalException
        #
        # We get an unknown local exception without collocation
        # optimization.
        #
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    begin
        thrower.throwLocalExceptionIdempotent()
        test(false)
    rescue Ice::UnknownLocalException
        #
        # We get an unknown local exception without collocation
        # optimization.
        #
    rescue Ice::OperationNotExistException
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    print "catching unknown non-Ice exception... "
    STDOUT.flush

    begin
        thrower.throwNonIceException()
        test(false)
    rescue Ice::UnknownException
        #
        # We get an unknown exception without collocation
        # optimization.
        #
    rescue
        print $!.backtrace.join("\n")
        test(false)
    end

    puts "ok"

    return thrower
end
