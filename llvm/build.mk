
top_srcdir ?= $(realpath $(CURDIR)/..)

LLVM_BUILD ?= $(realpath $(top_srcdir)/llvm/build)
LLVM_PREFIX ?= $(realpath $(top_srcdir)/llvm/usr)

LLVM_BRANCH  := $(shell git -C "$(realpath $(top_srcdir)/external/llvm)" rev-parse --abbrev-ref HEAD)
LLVM_VERSION := $(shell git -C "$(realpath $(top_srcdir)/external/llvm)" rev-parse HEAD)

CMAKE := $(or $(CMAKE),$(shell which cmake))
NINJA := $(shell which ninja)

$(LLVM_BUILD) $(LLVM_PREFIX):
	mkdir -p $@

$(LLVM_BUILD)/$(if $(NINJA),build.ninja,Makefile): $(top_srcdir)/external/llvm/CMakeLists.txt build.mk | $(LLVM_BUILD)
	cd $(LLVM_BUILD) && $(CMAKE) \
		$(if $(NINJA),-G Ninja) \
		-DCMAKE_INSTALL_PREFIX="$(LLVM_PREFIX)" \
		-DCMAKE_BUILD_TYPE=Release \
		-DLLVM_BUILD_TESTS=Off \
		-DLLVM_INCLUDE_TESTS=Off \
		-DLLVM_BUILD_EXAMPLES=Off \
		-DLLVM_INCLUDE_EXAMPLES=Off \
		-DLLVM_TOOLS_TO_BUILD="opt;llc;llvm-config;llvm-dis" \
		-DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" \
		-DLLVM_ENABLE_ASSERTIONS=$(if $(INTERNAL_LLVM_ASSERTS),On,Off) \
		$(LLVM_CMAKE_ARGS) \
		$(dir $<)

configure-llvm: $(LLVM_BUILD)/$(if $(NINJA),build.ninja,Makefile)

build-llvm: configure-llvm
	$(if $(NINJA),$(NINJA),$(MAKE)) -C $(LLVM_BUILD)

install-llvm: build-llvm | $(LLVM_PREFIX)
	$(if $(NINJA),$(NINJA),$(MAKE)) -C $(LLVM_BUILD) install

# FIXME: URL should be http://xamjenkinsartifact.blob.core.windows.net/build-package-osx-llvm-$(LLVM_BRANCH)/llvm-osx64-$(LLVM_VERSION).tar.gz
download-llvm:
	wget --no-verbose -O - http://xamjenkinsartifact.blob.core.windows.net/build-package-osx-llvm-release60/llvm-osx64-$(LLVM_VERSION).tar.gz | tar xzf -

clean-llvm:
	$(RM) -r $(LLVM_BUILD) $(LLVM_PREFIX)
