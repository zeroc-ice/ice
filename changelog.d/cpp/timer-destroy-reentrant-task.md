- Fixed a deadlock in `IceUtil::Timer::destroy`. Destroying the timer destroyed its scheduled tasks while holding the
  timer's lock, so a task whose destructor called back into the timer, for example to cancel another task,
  deadlocked.
