
examples_sources = $(wildcard $(lang_srcdir)/doxygen/examples/*.cpp) \
	$(wildcard $(lang_srcdir)/doxygen/examples/**/*.cpp) \
	$(wildcard $(lang_srcdir)/doxygen/examples/**/**/*.cpp)
examples_objects = $(patsubst %.cpp,%.o, $(examples_sources))

%.o: %.cpp
	$(E) "Compiling $<"
	$(Q)$(platform_cxx) $(cxxflags) $(cppflags) -I$(lang_srcdir)/include -I$(lang_srcdir)/include/generated -c $< -o $@

doxygen_examples: $(examples_objects)
	$(E) "Linking $(examples_objects)"
	$(Q)$(platform_cxx) $(examples_objects) $(ldflags) -o $@

ldflags += -L$(lang_srcdir)/lib -lIce

ifeq ($(os),Darwin)
    ldflags += -framework Security -framework CoreFoundation
endif

ifeq ($(os),Linux)
    ldflags += -lssl -lcrypto
endif
