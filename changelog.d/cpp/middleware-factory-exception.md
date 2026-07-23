- Improved the handling of exceptions thrown by middleware factories. A middleware factory should never throw; if it
  does throw during the creation of an object adapter's dispatch pipeline (at first dispatch), the program previously
  terminated. Now, the object adapter logs an error and all dispatches on this object adapter fail with an
  `UnknownException`.
