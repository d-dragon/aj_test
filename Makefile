# 	Makefile for CERES test tool
# 	Version: 1.0
# 
# 	Prerequisite: 
# 		1. jansson dev library was installed in the build PC (eg:  jansson-devel in CentOS)
# 		2.
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
OBJDIR 				= obj
BINDIR				= bin
# 
# Module source dir
# 
ONBOARDING_SRCDIR 	= src/onboarding
PARSINGMODULE_SRC	= src/parsing

ONBOARDING_OBJECTS	= $(patsubst $(ONBOARDING_SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(ONBOARDING_SRCDIR)/*.cc) $(wildcard src/OnboardingClientMain.cc))
PARSINGMODULE_OBJS	= $(patsubst $(PARSINGMODULE_SRC)/%.cc,$(OBJDIR)/%.o,$(wildcard $(PARSINGMODULE_SRC)/*.cc))

#
# Target: alljoynclient and onboarding
#	
all: onboarding alljoynclient

onboarding: directories OnboardingTestApp

alljoynclient:

# 
# Creat bin/ and obj dir to store temporary object and binary
# 
directories:
	@clear
	@echo "Create obj dir"
	@echo $(ONBOARDING_OBJECTS)
	@echo $(SOURCES)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# 
# Onboarding Apps
# 
OnboardingTestApp: $(ONBOARDING_OBJECTS) $(PARSINGMODULE_OBJS)
	@echo "build app"
	@echo $(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)
	$(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)

# 
# Onboarding Objects
# 
$(OBJDIR)/%.o: $(ONBOARDING_SRCDIR)/%.cc
	@echo "build object"
	@echo $(CXX) $(CXXFLAGS) -c $< -o $@ 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 
# Objects for Parsing Module
# 
$(OBJDIR)/%.o: $(PARSINGMODULE_SRC)/%.cc
	@echo "build object"
	@echo $(CXX) $(CXXFLAGS) -c $< -o $@ 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean: remove object directory and binary directory
clean:
	rm -rf $(OBJDIR) $(BINDIR)
