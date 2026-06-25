- Fixed a certificate context leak in the Schannel (Windows) SSL transport. It now takes ownership of the
  certificate contexts supplied by a credentials-selection callback and releases them on close, honoring the
  `ClientAuthenticationOptions`/`ServerAuthenticationOptions` contract; a custom callback that transferred
  ownership per that contract previously leaked one reference per certificate per connection.
