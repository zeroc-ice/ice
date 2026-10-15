- Fixed `icestormdb` to open the subscribers database with the same key comparator as the IceStorm service.
  Previously a database produced by `icestormdb --import` was ordered differently than the service expected, so
  after restoring a backup the service could fail to find subscriber and topic records whose identities have
  differing lengths. Existing export files remain valid and can be imported with the fixed `icestormdb`.
