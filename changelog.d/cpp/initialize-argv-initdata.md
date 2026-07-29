- Added an `Ice::initialize` overload that takes command-line arguments together with an `InitializationData`, for
  a program that wants to supply its own property defaults and still let the command line and a configuration file
  override them.
