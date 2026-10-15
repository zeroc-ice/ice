- Fixed `Ice.Future.result`, `Ice.Future.exception` and `Ice.InvocationFuture.sent` to return immediately when called
  with a timeout of `0`, raising `Ice.TimeoutException` if the result is not available yet or the invocation was not
  sent yet; they previously waited indefinitely. Fixed `InvocationFuture.sent` to raise as soon as an invocation that
  was never sent is canceled or fails; it previously waited until the timeout expired, or indefinitely without a
  timeout.
