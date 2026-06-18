- Fixed a deadlock in a replicated IceStorm deployment. A master replica that lost the majority of its replicas (for
  example during a network partition) while a topic update — such as a subscribe or unsubscribe — was in progress
  could hang, halting all further operations on that node until it was restarted.
