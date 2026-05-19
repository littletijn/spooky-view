CXX     := g++
WINDRES := windres
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/SpookyView.exe
SRC := $(wildcard *.cpp)
OBJ := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
RES := $(OBJ_DIR)/SpookyViewRes.o $(OBJ_DIR)/Manifest.o

CXXFLAGS := -std=c++14 -DWIN32 -D_WINDOWS -DDEBUG -DUNICODE -D_UNICODE -g \
            -Wall -Wextra -Wformat=2 -Wformat-security \
            -fstack-protector-strong -fno-strict-aliasing \
            -I. -I..\packages\nlohmann.json.3.11.3\build\native\include
# Static-link the C/C++ runtime so the resulting exe has no libgcc/libstdc++
# DLL dependencies — fixes "missing DLL" errors on clean target machines.
# PE hardening: ASLR (--dynamicbase), DEP (--nxcompat), High-Entropy VA
# (effective on 64-bit, harmless warning on 32-bit).
LDFLAGS  := -static -static-libgcc -static-libstdc++ \
            -Wl,--dynamicbase,--nxcompat,--high-entropy-va \
            -lcomctl32 -lcomdlg32 -lshlwapi -lversion -lpsapi -lwinhttp \
            -lshell32 -ldbghelp -mwindows -municode

.PHONY: build clean

build: $(EXE)

$(EXE): $(OBJ) $(RES) | $(BIN_DIR)
	$(CXX) $(OBJ) $(RES) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/SpookyViewRes.o: SpookyView.rc | $(OBJ_DIR)
	$(WINDRES) $< $@

$(OBJ_DIR)/Manifest.o: Manifest.rc | $(OBJ_DIR)
	$(WINDRES) $< $@

$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

clean:
	@if exist "$(OBJ_DIR)\*.o" del /q "$(OBJ_DIR)\*.o"
	@if exist "$(BIN_DIR)\SpookyView.exe" del /q "$(BIN_DIR)\SpookyView.exe"