CX=clang++-3.7
PGO_CX=g++
# clang++ <= 3.6 doesn't support -g with -std=c++1y
DBG_CX=clang++-3.7
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold when using clang++
BASEFLAGS=-std=c++14 -Wall -Wextra -Werror $(EXTRA)
FLAGS=-Ofast -ffast-math -flto
DEBUGFLAGS=-O0 -g
MULTI=-pthread

# this is going to fail miserably on non-Linux
NPROCS=$(shell grep -c ^processor /proc/cpuinfo)
PGOFLAGS=$(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA)

EXECS=test testTT randomTree randomEval randomVerify coding repair testnav strip
#EXECS

all: $(EXECS)

pgo: testPGO randomEvalPGO randomVerifyPGO codingPGO repairPGO

bin_release_%: $(subst bin_release_,,%).cpp *.h
	$(CX) $(BASEFLAGS) $(FLAGS) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_debug_%: $(subst bin_debug_,,%).cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_nodebug_%: $(subst bin_nodebug_,,%).cpp *.h
	$(CX) $(BASEFLAGS) $(FLAGS) -DNDEBUG -o $(subst .cpp,,$<)$(EXTRA) $<

bin_prelease_%: $(subst bin_prelease_,,%).cpp *.h
	$(CX) $(BASEFLAGS) $(FLAGS) $(MULTI) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_pdebug_%: $(subst bin_pdebug_,,%).cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_pnodebug_%: $(subst bin_pnodebug_,,%).cpp *.h
	$(CX) $(BASEFLAGS) $(FLAGS) $(MULTI) -DNDEBUG -o $(subst .cpp,,$<)$(EXTRA) $<

test: bin_release_test
	@#significant comment
testDebug: bin_debug_test
testNoDebug: bin_nodebug_test

testPGO: test.cpp *.h
	rm -f test.gcda
	$(PGO_CX) $(PGOFLAGS) -fprofile-generate -o test-p$(EXTRA) test.cpp
	./test-p$(EXTRA) data/others/dblp_small.xml
	./test-p$(EXTRA) -r data/others/dblp_small.xml
	$(PGO_CX) $(PGOFLAGS) -fprofile-use -o test-p$(EXTRA) test.cpp

testTT: bin_release_testTT
	@#significant comment
testTTDebug: bin_debug_testTT
testTTNoDebug: bin_nodebug_testTT

randomTree: bin_release_randomTree
	@#significant comment
randomTreeDebug: bin_debug_randomTree
randomTreeNoDebug: bin_nodebug_randomTree

randomEval: bin_prelease_randomEval
	@#significant comment
randomEvalDebug: bin_pdebug_randomEval
randomEvalNoDebug: bin_pnodebug_randomEval

randomEvalPGO: randomEval.cpp *.h
	rm -f randomEval.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomEval-p$(EXTRA) randomEval.cpp
	./randomEval-p$(EXTRA) -n 100 -m 100000
	./randomEval-p$(EXTRA) -n 100 -m 100000 -r
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomEval-p$(EXTRA) randomEval.cpp

randomVerify: bin_prelease_randomVerify
	@#significant comment
randomVerifyDebug: bin_pdebug_randomVerify
randomVerifyNoDebug: bin_pnodebug_randomVerify

randomVerifyPGO: randomVerify.cpp *.h
	rm -f randomVerify.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomVerify-p$(EXTRA) randomVerify.cpp
	./randomVerify-p$(EXTRA) -n 100 -m 100000
	./randomVerify-p$(EXTRA) -r -n 100 -m 100000
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomVerify-p$(EXTRA) randomVerify.cpp


coding: bin_release_coding
	@#significant comment
codingDebug: bin_debug_coding
codingNoDebug: bin_nodebug_coding

codingPGO: coding.cpp *.h
	rm -f coding.gcda
	$(PGO_CX) $(PGOFLAGS) -fprofile-generate -o coding-p$(EXTRA) coding.cpp
	./coding-p$(EXTRA) data/others/dblp_small.xml
	./coding-p$(EXTRA) -r data/others/dblp_small.xml
	$(PGO_CX) $(PGOFLAGS) -fprofile-use -o coding-p$(EXTRA) coding.cpp

repair: bin_release_repair
	@#significant comment
repairDebug: bin_debug_repair
repairNoDebug: bin_nodebug_repair

repairPGO: repair.cpp *.h
	rm -f repair.gcda
	$(PGO_CX) $(PGOFLAGS) -fprofile-generate -o repair-p$(EXTRA) repair.cpp
	./repair-p$(EXTRA) data/others/dblp_small.xml
	$(PGO_CX) $(PGOFLAGS) -fprofile-use -o repair-p$(EXTRA) repair.cpp

testnav: bin_release_testnav
	@#significant comment
testnavDebug: bin_debug_testnav
testnavNoDebug: bin_nodebug_testnav

strip: bin_release_strip
	@#significant comment

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o

scan-build:
	scan-build-3.6 -analyze-headers -enable-checker core.CallAndMessage -enable-checker core.DivideZero -enable-checker core.DynamicTypePropagation -enable-checker core.NonNullParamChecker -enable-checker core.NullDereference -enable-checker core.StackAddressEscape -enable-checker core.UndefinedBinaryOperatorResult -enable-checker core.VLASize -enable-checker core.builtin.BuiltinFunctions -enable-checker core.builtin.NoReturnFunctions -enable-checker core.uninitialized.ArraySubscript -enable-checker core.uninitialized.Assign -enable-checker core.uninitialized.Branch -enable-checker core.uninitialized.CapturedBlockVariable -enable-checker core.uninitialized.UndefReturn -enable-checker cplusplus.NewDelete -enable-checker cplusplus.NewDeleteLeaks -enable-checker deadcode.DeadStores -enable-checker security.insecureAPI.UncheckedReturn -enable-checker security.insecureAPI.getpw -enable-checker security.insecureAPI.gets -enable-checker security.insecureAPI.mkstemp -enable-checker security.insecureAPI.mktemp -enable-checker security.insecureAPI.vfork -enable-checker unix.API -enable-checker unix.Malloc -enable-checker unix.MallocSizeof -enable-checker unix.MismatchedDeallocator -enable-checker unix.cstring.BadSizeArg -enable-checker unix.cstring.NullArg  $(CX) $(DEBUGFLAGS) $(BASEFLAGS) -o coding-sb coding.cpp

cppcheck:
	cppcheck --enable=all --inconclusive .
