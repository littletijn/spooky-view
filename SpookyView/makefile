CXX = g++
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/SpookyView.exe
SRC := $(wildcard *.cpp)
OBJ := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

CXXFLAGS =-std=c++14 -DWIN32 -D_WINDOWS -DDEBUG -g -nostdlib -I. -I..\packages\nlohmann.json.3.10.5\build\native\include
LDFLAGS :=-lcomctl32 -lcomdlg32 -lshlwapi -lversion -lpsapi -lwinhttp -mwindows

build: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) $^ obj/SpookyViewRes.o obj/Manifest.o -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@