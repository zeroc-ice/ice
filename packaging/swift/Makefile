#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
# Makefile for building Ice XCFrameworks for Swift Package Manager distribution.
#
# This Makefile builds static libraries for macOS, iOS, and iOS Simulator,
# then creates XCFrameworks from them.
#
# Usage:
#   make                    Build all XCFrameworks
#   make OPTIMIZE=yes       Build optimized XCFrameworks
#   make V=1                Build with verbose output
#   make clean              Clean XCFramework output
#   make distclean          Clean everything including C++ builds
#

top_srcdir      := ../..
cpp_srcdir      := $(top_srcdir)/cpp

# XCFramework components to build
xcframeworks    := Ice IceSSL IceDiscovery IceLocatorDiscovery

# Components with public headers in cpp/include/
xcframeworks_with_headers := Ice IceSSL

# Platforms to build for
platforms       := macosx iphoneos iphonesimulator

# Output directories
build_dir       := $(CURDIR)/build
xcframework_dir := $(build_dir)/XCFrameworks

# Verbosity
V               ?= 0
ifeq ($(V),0)
    Q           := @
    E           = @echo $1
else
    Q           :=
    E           =
endif

# Common flags for all platforms
common_make_flags       := CPPFLAGS="-DICE_SWIFT" \
                           SLICEFLAGS="-DICE_SWIFT" \
                           $(if $(OPTIMIZE),OPTIMIZE=$(OPTIMIZE))

# Platform-specific make flags
macosx_configs          := static
macosx_make_flags       := CONFIGS="$(macosx_configs)" \
                           $(common_make_flags)
macosx_libdir           := $(cpp_srcdir)/lib

iphoneos_configs        := static
iphoneos_make_flags     := CONFIGS="$(iphoneos_configs)" \
                           PLATFORMS=iphoneos \
                           $(common_make_flags)
iphoneos_libdir         := $(cpp_srcdir)/lib/iphoneos

iphonesimulator_configs         := static
iphonesimulator_make_flags      := CONFIGS="$(iphonesimulator_configs)" \
                                   PLATFORMS=iphonesimulator \
                                   $(common_make_flags)
iphonesimulator_libdir          := $(cpp_srcdir)/lib/iphonesimulator

#
# Get static library path for a component on a platform
# $(call lib-path,component,platform)
#
lib-path = $($(2)_libdir)/lib$(1).a

#
# Get all library paths for a component across all platforms
# $(call all-libs,component)
#
all-libs = $(foreach p,$(platforms),$(call lib-path,$1,$p))

#
# Check if component has public headers
# $(call has-headers,component)
#
has-headers = $(filter $1,$(xcframeworks_with_headers))

#
# XCFramework flags for xcodebuild
# $(call xcframework-flags,component)
#
xcframework-flags = $(foreach p,$(platforms),-library $(call lib-path,$1,$p) $(if $(call has-headers,$1),-headers $(cpp_srcdir)/include/$1))

.PHONY: all clean distclean generate-srcs $(platforms) xcframeworks \
        $(foreach f,$(xcframeworks),$f.xcframework)

all: xcframeworks

#
# Generate C++ sources from Slice files with ICE_SWIFT defined.
# This must be done before building to ensure headers have correct dispatch_queue_t types.
# Uses a sentinel file to ensure it only runs once even with parallel make.
#
$(build_dir)/.generate-srcs-done:
	$(call E,"Generating C++ sources with ICE_SWIFT")
	$(Q)$(MAKE) -C $(cpp_srcdir) SLICEFLAGS="-DICE_SWIFT" generate-srcs
	$(Q)mkdir -p $(build_dir)
	$(Q)touch $@

generate-srcs: $(build_dir)/.generate-srcs-done

#
# Build static libraries for each platform
#
macosx: $(build_dir)/.generate-srcs-done
	$(call E,"Building C++ static libraries for macOS")
	$(Q)$(MAKE) -C $(cpp_srcdir) $(macosx_make_flags) $(xcframeworks)

iphoneos: $(build_dir)/.generate-srcs-done
	$(call E,"Building C++ static libraries for iOS")
	$(Q)$(MAKE) -C $(cpp_srcdir) $(iphoneos_make_flags) $(xcframeworks)

iphonesimulator: $(build_dir)/.generate-srcs-done
	$(call E,"Building C++ static libraries for iOS Simulator")
	$(Q)$(MAKE) -C $(cpp_srcdir) $(iphonesimulator_make_flags) $(xcframeworks)

#
# Create XCFramework from static libraries
# $(call create-xcframework,component)
#
define create-xcframework

$1.xcframework:
	$(call E,"Creating XCFramework for $1")
	$(Q)mkdir -p $(xcframework_dir)
	$(Q)rm -rf $(xcframework_dir)/$1.xcframework
	$(Q)xcodebuild -create-xcframework \
		$(call xcframework-flags,$1) \
		-output $(xcframework_dir)/$1.xcframework
	$(Q)if [ -d $(cpp_srcdir)/include/$1 ]; then \
		find $(xcframework_dir)/$1.xcframework -name Headers -prune \
			-exec mv {} {}_$1 \; \
			-exec mkdir {} \; \
			-exec mv {}_$1 {}/$1 \; ; \
		if [ -d $(cpp_srcdir)/include/generated/$1 ]; then \
			find $(xcframework_dir)/$1.xcframework -name Headers -prune \
				-exec cp -r $(cpp_srcdir)/include/generated/$1/* {}/$1 \; ; \
		fi ; \
	fi

endef

# Generate rules for each XCFramework
$(foreach f,$(xcframeworks),$(eval $(call create-xcframework,$f)))

# Build all XCFrameworks - platforms must complete before creating xcframeworks
xcframeworks: $(platforms)
	$(Q)$(MAKE) $(foreach f,$(xcframeworks),$f.xcframework)

clean:
	$(call E,"Cleaning XCFrameworks")
	$(Q)rm -rf $(build_dir)

distclean: clean
	$(call E,"Cleaning C++ builds")
	$(Q)$(MAKE) -C $(cpp_srcdir) clean
