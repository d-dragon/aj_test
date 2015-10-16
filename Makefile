# 	Makefile for CERES test tool
# 	Version: 1.0
# 
# 
# 	Compiler
# 
CC=gcc
CXX=g++

#
#	Link and include
#	
CXXFLAGS			= -DROUTER -DQCC_OS_GROUP_POSIX -DBINDINGS=cpp -DWS=off -DBR=on -DICE=off -DOS=linux -DCPU=x86_64 -I./inc -I./src/onboarding
LIBS				= -L./lib -lrt -lpthread -lajrouter -lalljoyn -lalljoyn_about -lalljoyn_onboarding -lalljoyn_services_common -lalljoyn_config
#
#	Directories information
#	
OBJDIR 				= obj
SRCDIR 				= src/onboarding
BINDIR				= bin
ONBOARDING_OBJECTS	= $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/*.cc) $(wildcard src/OnboardingClientMain.cc))

#
#	2 Target: alljoynclient and onboarding
#	
all: onboarding alljoynclient

onboarding: directories OnboardingTestApp

alljoynclient:


directories:
	@clear
	@echo "Create obj dir"
	@echo $(ONBOARDING_OBJECTS)
	@echo $(SOURCES)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

OnboardingTestApp: $(ONBOARDING_OBJECTS)
	@echo "build app"
	@echo $(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)
	$(CXX) -o $(BINDIR)/$@ $^ $(CXXFLAGS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@echo "build object"
	@echo $(CXX) $(CXXFLAGS) -c $< -o $@ 
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

clean:
	rm -rf $(OBJDIR) $(BINDIR)
