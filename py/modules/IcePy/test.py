import Ice
Ice.loadSlice("../../demo/Ice/hello/Hello.ice")
c = Ice.initialize()
p = c.stringToProxy("hello:tcp -p 10000")
import Demo
h = Demo.HelloPrx.checkedCast(p)
r1 = h.begin_sayHello(0)

p2 = c.stringToProxy("hello:tcp -p 10000")
h2 = Demo.HelloPrx.checkedCast(p2)
r2 = h2.begin_sayHello(0)

h2.end_sayHello(r1)
