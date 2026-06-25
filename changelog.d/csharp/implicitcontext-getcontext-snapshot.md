- The per-thread `ImplicitContext.getContext` in C# now returns a snapshot of the context instead of the live
  internal dictionary (matching the `Shared` implementation). Code that mutated the returned dictionary to
  update the implicit context must now use `put` or `setContext`.
