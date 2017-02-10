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
CXXFLAGS			= -Wall -O0 -std=c++11 -DDEBUG_ENABLED -DROUTER -DQCC_OS_GROUP_POSIX -DBINDINGS=cpp -DWS=off -DBR=on -DICE=off -DOS=linux -DCPU=x86_64 -I./inc -I./inc/alljoyn -I./src/onboarding -I./src/parsing
JANSSON_LIBS		= -ljansson
DEFAULT_LIBS		= -L./lib -lrt -lpthread -lajrouter -lalljoyn -lalljoyn_about -lalljoyn_onboarding -lalljoyn_services_common -lalljoyn_config

LIBS 				= $(DEFAULT_LIBS) $(JANSSON_LIBS)

#
# Directories information
#	
BUILDDIR			= build
OBJDIR 				= obj
BINDIR				= bin
LIBDIR				= lib
OUTPUTDIR			= output
SRCDIR 				= src

# 
# Module source dir
# 
ONBOARDING_SRCDIR 	= src/onboarding
AJCLIENT_SRCDIR		= src/ajclient
PARSINGMODULE_SRC	= src/parsing

ONBOARDING_OBJECTS	= $(patsubst $(ONBOARDING_SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(ONBOARDING_SRCDIR)/*.cc))
AJCLIENT_OBJECTS	= $(patsubst $(AJCLIENT_SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(AJCLIENT_SRCDIR)/*.cc))
PARSINGMODULE_OBJS	= $(patsubst $(PARSINGMODULE_SRC)/%.cc,$(OBJDIR)/%.o,$(wildcard $(PARSINGMODULE_SRC)/*.cc))

AJ_SERVICES_TARBALL	= alljoyn-base-16.04.00.tar.gz
AJ_CORE_TARBALL 	= alljoyn-16.04.00a.tar.gz
AJ_CORE_SRC			= alljoyn-16.04a-src
AJ_SERVICES_SRC		= alljoyn-services-16.04.00-src
SQLITE3_SRC			= sqlite-autoconf-3160200
JANSSON_SRC			= jansson-2.7
ROOT_DIR			= $(shell pwd)
#
# Target: alljoynclient and onboarding
#	
all: directories common_libs AlljoynClientApp AlljoynTester

AlljoynTester: $(AJCLIENT_OBJECTS) $(ONBOARDING_OBJECTS) $(PARSINGMODULE_OBJS)
	@echo "**********Build Alljoyn Tester Application**********"
	$(CXX) -c $(SRCDIR)/AlljoynTester.cc $(CXXFLAGS) $(LIBS) -o $(OBJDIR)/$@.o
	$(CXX) -o $(BINDIR)/$@ $^ $(OBJDIR)/$@.o $(CXXFLAGS) $(LIBS)
	cp $(SRCDIR)/.dev_env $(BINDIR)/

OnboardingTestApp: $(ONBOARDING_OBJECTS) $(AJCLIENT_OBJECTS) $(PARSINGMODULE_OBJS)
	@echo "Build Onboarding app"
	$(CXX) -c $(SRCDIR)/OnboardingClientMain.cc $(CXXFLAGS) $(LIBS) -o $(OBJDIR)/$@.o
	$(CXX) -o $(BINDIR)/$@ $^ $(OBJDIR)/$@.o $(CXXFLAGS) $(LIBS)

AlljoynClientApp: $(AJCLIENT_OBJECTS)
	@echo "**********Build AlljoynClientApp**********"
	$(CXX) -c $(SRCDIR)/AlljoynClientApp.cc $(CXXFLAGS) $(LIBS) -o $(OBJDIR)/$@.o
	$(CXX) -o $(BINDIR)/$@ $^ $(OBJDIR)/$@.o $(CXXFLAGS) $(LIBS)

common_libs: build_alljoyn_src build_alljoyn_services build_jansson

build_alljoyn_src: prepare_aj_sources
	@echo "Build Alljoyn source"
	@if test -e $(LIBDIR)/liballjoyn.so;then echo "AJ libs are available";\
		else cd $(BUILDDIR)/$(AJ_CORE_SRC);\
		scons BINDINGS=cpp WS=off BR=on ICE=off OS=linux CPU=x86_64 CRYPTO=builtin \
		VARIANT=release SQLITE_DIR=$(ROOT_DIR)/$(BUILDDIR)/sqlite-autoconf-3160200;\
		cp -a build/linux/x86_64/release/dist/cpp/lib/* $(ROOT_DIR)/lib/;\
		cp -a build/linux/x86_64/release/dist/cpp/inc/* $(ROOT_DIR)/inc;\
		cp -a common/inc/qcc/* $(ROOT_DIR)/inc/qcc/;fi;
	@cd  $(ROOT_DIR)

prepare_aj_sources:
	@if test -d $(BUILDDIR)/$(AJ_CORE_SRC); then echo "AJ core source is available";else tar xvf common_libs/$(AJ_CORE_TARBALL) -C $(BUILDDIR);fi
	@if test -d $(BUILDDIR)/$(SQLITE3_SRC);then echo "sqlite3 source is available";else tar xvf common_libs/$(SQLITE3_SRC).tar.gz -C $(BUILDDIR);fi

build_alljoyn_services:
	@echo "Build Alljoyn services"
	@if test -d $(BUILDDIR)/$(AJ_SERVICES_SRC);then echo "Aj service source is available";else tar xvf common_libs/$(AJ_SERVICES_TARBALL) -C $(BUILDDIR);fi
	@export ALLJOYN_DISTDIR=`pwd`/$(BUILDDIR)/$(AJ_CORE_SRC)/build/linux/x86_64/release/dist/;cd $(BUILDDIR)/$(AJ_SERVICES_SRC)/onboarding; \
    scons V=1 BINDINGS=cpp WS=off BR=on ICE=off OS=linux CPU=x86_64 VARIANT=release; \
    cp -a build/linux/x86_64/release/dist/onboarding/inc $(ROOT_DIR); \
    cp -a build/linux/x86_64/release/dist/onboarding/lib $(ROOT_DIR); \
    cp -a build/linux/x86_64/release/dist/services_common/inc $(ROOT_DIR); \
    cp -a build/linux/x86_64/release/dist/services_common/lib $(ROOT_DIR); \
    cd $(ROOT_DIR)
    #cp -a build/linux/x86_64/release/dist/config/inc $(ROOT_DIR); \
    #cp -a build/linux/x86_64/release/dist/config/lib $(ROOT_DIR); \

build_jansson:
	@echo "Build Jansson lib"
	@tar xzf common_libs/$(JANSSON_SRC).tar.gz -C $(BUILDDIR)
	@if [ -f $(LIBDIR)/libjansson.so.4.7.0 ] && [ -f inc/jansson.h ] && [ -f inc/jansson_config.h ] ; then \
		echo "do nothing"; \
	else \
		cd $(BUILDDIR)/$(JANSSON_SRC)/ ; ./configure --prefix=$(ROOT_DIR)/ --includedir=$(ROOT_DIR)/inc; make; make install; \
	fi	
	@cd $(ROOT_DIR)

		
# 
# Creat bin/ and obj dir to store temporary object and binary
# 
directories:
	@clear
	@echo "Create obj bin build dirs"
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p $(OUTPUTDIR)

ParsingApp: src/jsonparser.cc 
	$(CXX) -o $(BINDIR)/jsonparser src/jsonparser.cc $(CXXFLAGS) $(LIBS)
# 
# Onboarding Apps
# 

# 
# Onboarding Objects
# 
$(OBJDIR)/%.o: $(ONBOARDING_SRCDIR)/%.cc 
	@echo "Build onboarding object"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(AJCLIENT_SRCDIR)/%.cc
	@echo "**********Build ajclient object***********"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 
# Objects for Parsing Module
# 
$(OBJDIR)/%.o: $(PARSINGMODULE_SRC)/%.cc
	@echo "Build object"
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

# Clean: remove object directory and binary directory
clean:
	rm -rf $(OBJDIR) $(BINDIR) $(BUILDDIR) $(LIBDIR)
