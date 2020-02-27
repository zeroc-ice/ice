# define slices here for all targets
set(GLACIER2_SLICES
	${SLICE_DIR}/Glacier2/Metrics.ice
	${SLICE_DIR}/Glacier2/PermissionsVerifier.ice
	${SLICE_DIR}/Glacier2/PermissionsVerifierF.ice
	${SLICE_DIR}/Glacier2/Router.ice
	${SLICE_DIR}/Glacier2/RouterF.ice
	${SLICE_DIR}/Glacier2/Session.ice
	${SLICE_DIR}/Glacier2/SSLInfo.ice
)

set(ICE_SLICES
	${SLICE_DIR}/Ice/BuiltinSequences.ice
	${SLICE_DIR}/Ice/Communicator.ice
	${SLICE_DIR}/Ice/CommunicatorF.ice
	${SLICE_DIR}/Ice/Connection.ice
	${SLICE_DIR}/Ice/ConnectionF.ice
	${SLICE_DIR}/Ice/Current.ice
	${SLICE_DIR}/Ice/Endpoint.ice
	${SLICE_DIR}/Ice/EndpointF.ice
	${SLICE_DIR}/Ice/EndpointTypes.ice
	${SLICE_DIR}/Ice/FacetMap.ice
	${SLICE_DIR}/Ice/Identity.ice
	${SLICE_DIR}/Ice/ImplicitContext.ice
	${SLICE_DIR}/Ice/ImplicitContextF.ice
	${SLICE_DIR}/Ice/Instrumentation.ice
	${SLICE_DIR}/Ice/InstrumentationF.ice
	${SLICE_DIR}/Ice/LocalException.ice
	${SLICE_DIR}/Ice/Locator.ice
	${SLICE_DIR}/Ice/LocatorF.ice
	${SLICE_DIR}/Ice/Logger.ice
	${SLICE_DIR}/Ice/LoggerF.ice
	${SLICE_DIR}/Ice/Metrics.ice
	${SLICE_DIR}/Ice/ObjectAdapter.ice
	${SLICE_DIR}/Ice/ObjectAdapterF.ice
	${SLICE_DIR}/Ice/ObjectFactory.ice
	${SLICE_DIR}/Ice/Plugin.ice
	${SLICE_DIR}/Ice/PluginF.ice
	${SLICE_DIR}/Ice/Process.ice
	${SLICE_DIR}/Ice/ProcessF.ice
	${SLICE_DIR}/Ice/Properties.ice
	${SLICE_DIR}/Ice/PropertiesAdmin.ice
	${SLICE_DIR}/Ice/PropertiesF.ice
	${SLICE_DIR}/Ice/RemoteLogger.ice
	${SLICE_DIR}/Ice/Router.ice
	${SLICE_DIR}/Ice/RouterF.ice
	${SLICE_DIR}/Ice/ServantLocator.ice
	${SLICE_DIR}/Ice/ServantLocatorF.ice
	${SLICE_DIR}/Ice/SliceChecksumDict.ice
	${SLICE_DIR}/Ice/ValueFactory.ice
	${SLICE_DIR}/Ice/Version.ice
)

set(ICEBOXLIB_SLICES
	${SLICE_DIR}/IceBox/IceBox.ice
)

set(ICEDISCOVERY_SLICES
	${SLICE_DIR}/IceDiscovery/IceDiscovery.ice
)

set(ICEGRID_SLICES
	${SLICE_DIR}/IceGrid/Admin.ice
	${SLICE_DIR}/IceGrid/Descriptor.ice
	${SLICE_DIR}/IceGrid/Exception.ice
	${SLICE_DIR}/IceGrid/FileParser.ice
	${SLICE_DIR}/IceGrid/PluginFacade.ice
	${SLICE_DIR}/IceGrid/Registry.ice
	${SLICE_DIR}/IceGrid/Session.ice
	${SLICE_DIR}/IceGrid/UserAccountMapper.ice
)

set(ICELOCATORDISCOVERY_SLICES
	${SLICE_DIR}/IceLocatorDiscovery/IceLocatorDiscovery.ice
)

set(ICEPATCH2_SLICES
	${SLICE_DIR}/IcePatch2/FileInfo.ice
	${SLICE_DIR}/IcePatch2/FileServer.ice
)

set(ICESSL_SLICES
	${SLICE_DIR}/IceSSL/ConnectionInfo.ice
	${SLICE_DIR}/IceSSL/ConnectionInfoF.ice
	${SLICE_DIR}/IceSSL/EndpointInfo.ice
)

set(ICESTORM_SLICES
	${SLICE_DIR}/IceStorm/IceStorm.ice
	${SLICE_DIR}/IceStorm/Metrics.ice
)

set(ICEBT_SLICES
	${SLICE_DIR}/IceBT/ConnectionInfo.ice
	${SLICE_DIR}/IceBT/EndpointInfo.ice
	${SLICE_DIR}/IceBT/Types.ice
)
