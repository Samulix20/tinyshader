// Create a basic window using GLFW and GLEW

// Define because we are using glfw3.dll
#define GLFW_DLL

// Extension Wrangler
#include <GL/glew.h>
// Simple API for OpenGL
#include <GLFW/glfw3.h>

// C++ libs
#include <iostream>
#include <tuple>
#include <chrono>
#include <iomanip>

// C libs
#include <cstdlib>
#include <cstdio>
#include <cmath>

// Terminal dimensions for Windows and Unix
#if defined(_WIN64)

#include <windows.h>

std::tuple<uint32_t, uint32_t> terminal_dimensions() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	uint32_t columns, rows;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return std::make_tuple(columns, rows);
}

#elif defined(__unix__)

#include <sys/ioctl.h>

std::tuple<int, int> terminal_dimensions() {
	struct winsize w;
	ioctl(fileno(stdout), TIOCGWINSZ, &w);
	return std::make_tuple(w.ws_col, w.ws_row);
}

#endif

/*
	Reads <filename> into a C style string
*/
char* read_file(std::string filename) {
	// Open file
	FILE* shader_file = fopen(filename.c_str(), "r");
	// Error check
	if(shader_file == NULL) return nullptr;
	// Go to EOF
	fseek(shader_file, 0, SEEK_END);
	// Get num chars
	unsigned int numchars = ftell(shader_file);
	// Go back to file start
	fseek(shader_file, 0, SEEK_SET);
	// Malloc +1 Byte for \0
	char* c_buff = new char[numchars + 1];
	// Copy file to heap
	fread(c_buff, sizeof(char), numchars, shader_file);
	// Set \0
	c_buff[numchars] = '\0';
	// Close file
	fclose(shader_file);
	return c_buff;
}

/*
	Compiles <filename> shader
	<shader_type> can be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	
	Returns shader OpenGL id and the source in a C style string
*/
std::tuple<GLuint, char*> compile_shader(std::string filename, GLenum shader_type) {
	// Create shader
	GLuint shader_id = glCreateShader(shader_type);
	// Get source
	char* source = read_file(filename);
	// Error check
	if (source == nullptr) return std::make_tuple(0, nullptr);
	glShaderSource(shader_id, 1, &source, NULL);
	// Compile shader
	glCompileShader(shader_id);
	return std::make_tuple(shader_id, source);
}

/*
	Links a complete shader program using <vertex> and <fragment> shaders
	<vertex> and <fragment> must be the filenames of the source files

	Returns program OpenGL id and the linking process status
*/
std::tuple<GLuint, GLint> link_shader_program(std::string vertex, std::string fragment) {
	// Vertex shader
	GLuint vertex_shader;
	char* vertex_shader_src;
	std::tie(vertex_shader, vertex_shader_src) = compile_shader(vertex, GL_VERTEX_SHADER);
	// Fragment shader
	GLuint fragment_shader;
	char* fragment_shader_src;
	std::tie(fragment_shader, fragment_shader_src) = compile_shader(fragment, GL_FRAGMENT_SHADER);
	// Error check
	if (fragment_shader_src == nullptr || vertex_shader_src == nullptr) return std::make_tuple(0,0);
	// Link Shader program
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	// Link shader
	glLinkProgram(shader_program);
	GLint success;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	// Free memory
	glDeleteShader(vertex_shader);
	delete vertex_shader_src;
	glDeleteShader(fragment_shader);
	delete fragment_shader_src;

	return std::make_tuple(shader_program, success);
}

/*
	Callback for updating the OpenGL viewport dimensions acording to the window
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// Set new window dimensions to OpenGL viewport
	glViewport(0, 0, width, height);
}

int main() {

	// Init glfw
	int error = glfwInit();
	if (error == GLFW_FALSE) {
		std::cout << "glfwInit error" << std::endl;
		exit(1);
	}

	// Create Window
	GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	if (window == nullptr) {
		std::cout << "glfwCreateWindow error" << std::endl;
		glfwTerminate();
		exit(1);
	}

	// Create OpenGL context
	glfwMakeContextCurrent(window);

	// Init glew
	glewInit();

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// get renderer string
	printf("Renderer %s\n", glGetString(GL_RENDERER));
	// version as a string
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
	std::cout << "-------------" << std::endl;

	// Set up vertices for a Square
	float vertices[] = {
		 1.0,  1.0, 0.0,
		-1.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
		-1.0,  1.0, 0.0
	};
	GLuint indices[] = {
		0, 2, 3,
		1, 2, 3
	};
	// Set up vertex array info
	// VAO
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	// VBO
	GLuint vertex_buffer_object;
	glGenBuffers(1, &vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// EBO
	GLuint element_buffer_object;
	glGenBuffers(1, &element_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// Position for vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);
	
	GLint success = 0;
	GLuint active_shader, new_shader;
	std::string prev_info_log = "";

	// Compile first version of shader
	std::tie(active_shader, success) = link_shader_program("vertex.vert", "fragment.frag");

	// Main loop
	while (!glfwWindowShouldClose(window)){

		// Compile and link shader
		std::tie(new_shader, success) = link_shader_program("vertex.vert", "fragment.frag");
		
		// Check compilation error
		if (success) {
			// Swap shader
			glDeleteProgram(active_shader);
			active_shader = new_shader;
		} else {
			// Get error
			char tmpbuff[512];
			glGetProgramInfoLog(new_shader, 512, NULL, tmpbuff);
			std::string link_log_str(tmpbuff);
			// Print error if different from last
			if (prev_info_log != link_log_str) {
				std::cout << "shader program link error" << std::endl;
				std::cout << link_log_str;
				std::cout << "-------------" << std::endl;
				prev_info_log = link_log_str;
			}
		}

		// Activate shader
		glUseProgram(active_shader);

		// glfw stuff for window
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Pass float time parameter
		float time_s = glfwGetTime();
		GLuint var_location = glGetUniformLocation(active_shader, "time");
		glUniform1f(var_location, time_s);

		// Get resolution
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		// Pass resulution as vec2
		var_location = glGetUniformLocation(active_shader, "resolution");
		glUniform2f(var_location, width, height);

		// Render call
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	}

	// Cleanup glfw window
	glfwDestroyWindow(window);
	glfwTerminate();
}
