// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import com.zeroc.Ice.*;
import com.zeroc.Ice.Object;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CompletionStage;
import test.Ice.middleware.Test.*;

public class AllTests {

  private static class Middleware implements Object {
    private final Object _next;
    private final String _name;
    private final List<String> _inLog;
    private final List<String> _outLog;

    Middleware(Object next, String name, List<String> inLog, List<String> outLog) {
      _next = next;
      _name = name;
      _inLog = inLog;
      _outLog = outLog;
    }

    @Override
    public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
        throws UserException {
      _inLog.add(_name);
      return _next
          .dispatch(request)
          .thenApply(
              response -> {
                _outLog.add(_name);
                return response;
              });
    }
  }

  public static void allTests(test.TestHelper helper) {
    Communicator communicator = helper.communicator();
    PrintWriter output = helper.getWriter();
    testMiddlewareExecutionOrder(communicator, output);
  }

  private static void testMiddlewareExecutionOrder(Communicator communicator, PrintWriter output) {
    output.write("testing middleware execution order... ");
    output.flush();

    // Arrange
    List<String> inLog = new ArrayList<>();
    List<String> outLog = new ArrayList<>();

    ObjectAdapter oa = communicator.createObjectAdapter("");

    ObjectPrx obj = oa.add(new MyObjectI(), new Identity("test", ""));

    oa.use(next -> new Middleware(next, "A", inLog, outLog))
        .use(next -> new Middleware(next, "B", inLog, outLog))
        .use(next -> new Middleware(next, "C", inLog, outLog));

    var p = MyObjectPrx.uncheckedCast(obj);

    // Act
    p.ice_ping();

    // Assert
    test(inLog.equals(Arrays.asList("A", "B", "C")));
    test(outLog.equals(Arrays.asList("C", "B", "A")));

    output.println("ok");
    oa.deactivate();
  }

  private static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }
}
