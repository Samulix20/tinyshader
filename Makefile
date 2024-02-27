
CC := g++ -O3

ifeq ($(OS),Windows_NT)
	LIBS := lib/opengl32.dll lib/glew32.dll lib/glfw3.dll
else
	LIBS := -lopengl32 -lglew32 -lglfw3 
endif

all:
	@echo "OS: $(OS)"
	$(CC) $(LIBS) src/main.cc -o tinyshader.exe
	./tinyshader.exe
