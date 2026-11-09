- Fixed a bug in PHP-FPM workers and Apache prefork children when the Ice extension is loaded at startup
  (`extension=ice.so` in `php.ini`): the workers hung or aborted when they exited, even if they never used Ice, and
  communicators registered with an expiration time never expired.
