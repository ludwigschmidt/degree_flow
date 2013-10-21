# Ludwig Schmidt (ludwigschmidt2@gmail.com) 2013
#
# This makefile is based on http://make.paulandlesley.org/autodep.html .

CXX = g++
MEX = mex
CXXFLAGS = -Wall -Wextra -O2 -std=c++98 -ansi -fPIC
MEXCXXFLAGS = -Wall -Wextra -O2 -std=c++98 -ansi
GTESTDIR = /usr/src/gtest

SRCDIR = src
DEPDIR = .deps
OBJDIR = obj

SRCS = main.cc degree_flow.cc

.PHONY: clean archive

clean:
	rm -rf $(OBJDIR)
	rm -rf $(DEPDIR)
	rm -f degree_flow
	rm -f degree_flow_test
	rm -f degree_flow.mexa64
	rm -f degree_flow.mexmaci64
	rm -f degree_flow.tar.gz

archive:
	mkdir archive-tmp
	tar --transform='s,^\.,degree_flow,' --exclude='.git' --exclude='archive-tmp' -czf archive-tmp/degree_flow.tar.gz .
	mv archive-tmp/degree_flow.tar.gz .
	rm -rf archive-tmp

DEGREE_FLOW_OBJS = degree_flow.o

# degree_flow executable
DEGREE_FLOW_BIN_OBJS = $(DEGREE_FLOW_OBJS) main.o
degree_flow: $(DEGREE_FLOW_BIN_OBJS:%=$(OBJDIR)/%)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_program_options

# gtest
$(OBJDIR)/gtest-all.o: $(GTESTDIR)/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -I $(GTESTDIR) -c -o $@ $<

# degree_flow tests
DEGREE_FLOW_TEST_OBJS = $(DEGREE_FLOW_OBJS) degree_flow_test.o gtest-all.o
degree_flow_test: $(DEGREE_FLOW_TEST_OBJS:%=$(OBJDIR)/%)
	$(CXX) $(CXXFLAGS) -o $@ $^ -pthread

run_degree_flow_test: degree_flow_test
	./degree_flow_test

# degree_flow MEX file
MEXFILE_OBJECTS = degree_flow.o
MEXFILE_SRC = mex_wrapper.cc
MEXFILE_SRC_DEPS = $(MEXFILE_SRC) mex_helper.h degree_flow.h

mexfile: $(MEXFILE_OBJECTS:%=$(OBJDIR)/%) $(MEXFILE_SRC_DEPS:%=$(SRCDIR)/%)
	$(MEX) -v CXXFLAGS="\$$CXXFLAGS $(MEXCXXFLAGS)" -output degree_flow $(SRCDIR)/$(MEXFILE_SRC) $(MEXFILE_OBJECTS:%=$(OBJDIR)/%)


$(OBJDIR)/%.o: $(SRCDIR)/%.cc
  # Create the directory the current target lives in.
	@mkdir -p $(@D)
  # Compile and generate a dependency file.
  # See http://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html .
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<
  # Move dependency file to dependency file directory.
  # Create the dependency file directory if necessary.
	@mkdir -p $(DEPDIR)
	@mv $(OBJDIR)/$*.d $(DEPDIR)/$*.d

# Include the generated dependency files.
# The command replaces each file name in SRCS with its dependency file.
# See http://www.gnu.org/software/make/manual/html_node/Substitution-Refs.html#Substitution-Refs for the GNU make details.
-include $(SRCS:%.cc=$(DEPDIR)/%.d)
