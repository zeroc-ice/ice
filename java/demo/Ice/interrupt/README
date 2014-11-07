This demo illustrates how to interrupt blocking servant dispatches on
the server and interrupt blocking proxy invocations on the client by
using Thread.interrupt().

To run the demo, first start the server:

$ java -jar build/libs/server.jar

In a separate window, start the client:

$ java -jar build/libs/client.jar

Calling TaskManager::run on the server simulates a long running task
by sleeping 10 seconds. Ordinarily a server will not shutdown until
all executing dispatched requests are complete. By interrupting
dispatch threads using Thread.interrupt() a server shutdown will
proceed, as long as the servant implementation correctly handles the
interrupt.

The simplest way to interrupt dispatch threads is by using an
Ice.Dispatcher and ExecutorService. Calling shutdownNow on the
ExecutorService interrupts any executing tasks.

Pressing ^C in the server calls shutdownNow on the executor service,
as does pressing 's' in the client which calls TaskManager::shutdown,
the implementation of which itself calls shutdownNow.

It is also possible to interrupt blocking invocations on an Ice proxy
by calling Thread.interrupt().

In this demo, to interrupt a blocking proxy invocation on the client
press 'b' to run the invocation and 'i' to interrupt the invocation.
Only a single blocking invocation can be active at once.

Pressing 't' in the client runs the task on the server using a
non-blocking AMI invocation.

