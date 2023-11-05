CXX = g++
CXXFLAGS = -std=c++11

# Define the targets
TARGETS = Low-luminosity-enhancement SharpnessEnhancement Denoise

all: $(TARGETS)

# Compilation rules for each target
Low-luminosity-enhancement: Low-luminosity-enhancement.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

SharpnessEnhancement: SharpnessEnhancement.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

Denoise: Denoise.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rules for running the programs with arguments
run:
	./Low-luminosity-enhancement 1 1
	./Low-luminosity-enhancement 1 2
	./SharpnessEnhancement 2 1
	./SharpnessEnhancement 2 2
	./Denoise 3 1
	./Denoise 3 2
	make clean

.PHONY: clean

clean:
	rm -f $(TARGETS)
