# 	Makefile for CERES test tool
# 	Version: 1.0
# 
# 	Prerequisite: 
# 		1. jansson dev library was installed in the build PC (eg:  jansson-devel in CentOS)
# 		2. scons
# 		3. 
# 		 
# 	Compiler
# 
CC=gcc
CXX=g++

#
# Link and include
#	
CXXFLAGS			= -DROUTER -DQCC_OS_GROUP_POSIX -DBINDINGS=cpp -DWS=off -DBR=on -DICE=off -DOS=linux -DCPU=x86_64 -I./inc -I./src/onboarding -I./src/parsing
JANSSON_LIBS		= -ljansson
DEFAULT_LIBS		= -L./lib -lrt -lpthread -lajrouter -lalljoyn -lalljoyn_about -lalljoyn_onboarding -lalljoyn_services_common -lalljoyn_config

LIBS 				= $(DEFAULT_LIBS) $(JANSSON_LIBS)

#
# Directories information
#	
BUILDDIR			= build
OBJDIR 				= obj
BINDIR				= bin
# 
# Module source dir
# 
ONBOARDING_SRCDIR 	= src/onboarding
AJCLIENT_SRCDIR		= src/ajclient
PARSINGMODULE_SRC	= src/parsing

ONBOARDING_OBJECTS	= $(patsubst $(ONBOARDING_SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(ONBOARDING_SRCDIR)/*.cc))
AJCLIENT_OBJECTS	= $(patsubst $(AJCLIENT_SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(AJCLIENT_SRCDIR)/*.cc))
PARSINGMODULE_OBJS	= $(patsubst $(PARSINGMODULE_SRC)/%.cc,$(OBJDIR)/%.o,$(wildcard $(PARSINGMODULE_SRC)/*.cc))

AJ_CORE_SRC			= alljoyn-15.04.00b-src
AJ_SERVICES_SRC		= alljoyn-services-15.04.00-src
ROOT_DIR			= $(shell pwd)
#
# Target: alljoynclient and onboarding
#	
all: directories common_libs onboarding alljoynclient

onboarding: OnboardingTestApp

alljoynclient: $(AJCLIENT_OBJECTS)
	@echo "**********Build AlljoynClientApp**********"
	$(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)

common_libs: build_alljoyn_src build_alljoyn_services

build_alljoyn_src:
	@tar xzf common_libs/$(AJ_CORE_SRC).tar.gz -C $(BUILDDIR)
	@tar xzf common_libs/$(AJ_SERVICES_SRC).tar.gz -C $(BUILDDIR)
	@cd $(BUILDDIR)/$(AJ_CORE_SRC); scons BINDINGS=cpp WS=off BR=on ICE=off OS=linux CPU=x86_64 VARIANT=release; \
	cp -a build/linux/x86_64/release/dist/cpp/lib/ ../../; \
	cp -a build/linux/x86_64/release/dist/cpp/inc/ ../../; \
	cd -
	
build_alljoyn_services:
	@export ALLJOYN_DISTDIR=`pwd`/$(BUILDDIR)/$(AJ_CORE_SRC)/build/linux/x86_64/release/dist/;cd $(BUILDDIR)/$(AJ_SERVICES_SRC)/services/base/onboarding; \
        scons V=1 BINDINGS=cpp WS=off BR=on ICE=off OS=linux CPU=x86_64 VARIANT=release; \
        cp -a build/linux/x86_64/release/dist/config/inc $(ROOT_DIR); \
        cp -a build/linux/x86_64/release/dist/config/lib $(ROOT_DIR); \
        cp -a build/linux/x86_64/release/dist/onboarding/inc $(ROOT_DIR); \
        cp -a build/linux/x86_64/release/dist/onboarding/lib $(ROOT_DIR); \
        cp -a build/linux/x86_64/release/dist/services_common/inc $(ROOT_DIR); \
        cp -a build/linux/x86_64/release/dist/services_common/lib $(ROOT_DIR); \
        cd $(ROOT_DIR)

# 
# Creat bin/ and obj dir to store temporary object and binary
# 
directories:
	@clear
	@echo "Create obj bin build dirs"
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# 
# Onboarding Apps
# 
# OnboardingTestApp: $(ONBOARDING_OBJECTS) $(PARSINGMODULE_OBJS)
OnboardingTestApp: $(ONBOARDING_OBJECTS)
	@echo "Build Onboarding app"
	$(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)

# 
# Onboarding Objects
# 
$(OBJDIR)/%.o: $(ONBOARDING_SRCDIR)/%.cc 
	@echo "build onboarding object"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(AJCLIENT_SRCDIR)/%.cc
	@echo "**********Build ajclient object***********"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 
# Objects for Parsing Module
# 
$(OBJDIR)/%.o: $(PARSINGMODULE_SRC)/%.cc
	@echo "build object"
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

# Clean: remove object directory and binary directory
clean:
	rm -rf $(OBJDIR) $(BINDIR) $(BUILDDIR)
