CC := g++ -O3
LIBS := -lglfw3 -lglew32 -lopengl32

# Tested in windows mingw64 env for static linking. Change for LIBS
#LIBS := -static -mwindows $(LIBS) -static-libgcc -static-libstdc++

all:
	@echo "OS: $(OS)"
	$(CC) src/main.cc $(LIBS) -o tinyshader.exe
	./tinyshader.exe
