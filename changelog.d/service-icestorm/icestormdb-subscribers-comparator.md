- Fixed `icestormdb --import`: after restoring a backup with this command, the IceStorm service could fail to find some
  subscriber and topic records. Backups produced by `icestormdb --export` are not affected: you can restore an existing
  backup with the fixed `icestormdb --import` to produce a correct database.
