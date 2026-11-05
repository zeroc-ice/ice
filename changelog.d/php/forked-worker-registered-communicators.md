- Fixed registered communicators never expiring in PHP-FPM workers and Apache prefork children when the Ice extension
  is loaded from `php.ini`. These workers also aborted or hung when they exited, even if they never used Ice.
