#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Hello.ice')

def menu
    print <<MENU
usage:
t: send greeting as twoway
o: send greeting as oneway
O: send greeting as batch oneway
d: send greeting as datagram
D: send greeting as batch datagram
f: flush all batch requests
T: set a timeout
P: set a server delay
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
MENU
end

class Client < Ice::Application
    def interruptCallback(sig)
        begin
            Ice::Application::communicator.destroy
        rescue => ex
            puts ex
        end
        exit(0)
    end

    def run(args)
        #
        # Since this is an interactive demo we want the custom interrupt
        # callback to be called when the process is interrupted.
        #
        Ice::Application::callbackOnInterrupt

        twoway = Demo::HelloPrx::checkedCast(
            Ice::Application::communicator().propertyToProxy('Hello.Proxy').
                ice_twoway().ice_timeout(-1).ice_secure(false))
        if not twoway
            puts $0 + ": invalid proxy"
            return 1
        end

        oneway = twoway.ice_oneway()
        batchOneway = twoway.ice_batchOneway()
        datagram = twoway.ice_datagram()
        batchDatagram = twoway.ice_batchDatagram()

        secure = false
        timeout = -1
        delay = 0

        menu()

        c = nil
        while c != 'x'
            begin
                print "==> "
                STDOUT.flush
                line = STDIN.readline
                c = line[0..0]
                if c == 't'
                    twoway.sayHello(delay)
                elsif c == 'o'
                    oneway.sayHello(delay)
                elsif c == 'O'
                    batchOneway.sayHello(delay)
                elsif c == 'd'
                    if secure
                        puts "secure datagrams are not supported"
                    else
                        datagram.sayHello(delay)
                    end
                elsif c == 'D'
                    if secure
                        puts "secure datagrams are not supported"
                    else
                        batchDatagram.sayHello(delay)
                    end
                elsif c == 'f'
                    Ice::Application::communicator().flushBatchRequests()
                elsif c == 'T'
                    if timeout == -1
                        timeout = 2000
                    else
                        timeout = -1
                    end

                    twoway = twoway.ice_timeout(timeout)
                    oneway = oneway.ice_timeout(timeout)
                    batchOneway = batchOneway.ice_timeout(timeout)

                    if timeout == -1
                        puts "timeout is now switched off"
                    else
                        puts "timeout is now set to 2000ms"
                    end
                elsif c == 'P'
                    if delay == 0
                        delay = 2500
                    else
                        delay = 0
                    end

                    if delay == 0
                        puts "server delay is now deactivated"
                    else
                        puts "server delay is now set to 2500ms"
                    end
                elsif c == 'S'
                    secure = !secure

                    twoway = twoway.ice_secure(secure)
                    oneway = oneway.ice_secure(secure)
                    batchOneway = batchOneway.ice_secure(secure)
                    datagram = datagram.ice_secure(secure)
                    batchDatagram = batchDatagram.ice_secure(secure)

                    if secure
                        puts "secure mode is now on"
                    else
                        puts "secure mode is now off"
                    end
                elsif c == 's'
                    twoway.shutdown()
                elsif c == 'x'
                    # Nothing to do
                elsif c == '?'
                    menu()
                else
                    puts "unknown command `" + c + "'"
                    menu()
                end
            rescue Ice::Exception => ex
                puts ex
            rescue EOFError
                break
            end
        end

        return 0
    end
end

app = Client.new()
exit(app.main(ARGV, "config.client"))
