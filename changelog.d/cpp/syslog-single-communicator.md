- Only one Ice syslog logger can be active at a time in a process. Creating a second one (for example, a second
  communicator with `Ice.UseSyslog=1`) while the first is still active now throws `InitializationException` instead of
  corrupting the process-global syslog state.
