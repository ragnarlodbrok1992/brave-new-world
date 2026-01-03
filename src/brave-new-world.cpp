#pragma warning(push, 0)
#include <glad/glad.h>

#define GLFW_STATIC
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#pragma warning(pop)

#include <stdio.h>

// --------------------- ENGINE CONSTANTS
constexpr int RATIO_WIDTH = 16;
constexpr int RATIO_HEIGHT = 9;

constexpr int RESOLUTION_RATIO = 120;

constexpr int SCREEN_WIDTH = RATIO_WIDTH * RESOLUTION_RATIO;
constexpr int SCREEN_HEIGHT = RATIO_HEIGHT * RESOLUTION_RATIO;

const char* GAME_TITLE = "Brave New World";

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f

#define CAMERA_YAW_DEFAULT -90.0f
#define CAMERA_PITCH_DEFAULT 0.0f
#define CAMERA_SPEED_DEFAULT 2.5f
#define CAMERA_SENSITIVITY_DEFAULT 0.1f
#define CAMERA_FOV_DEFAULT 45.0f
#define CAMERA_POSITION_DEFAULT glm::vec3(0.0f, 0.0f, 3.0f)
#define CAMERA_FRONT_DEFAULT glm::vec3(0.0f, 0.0f, -1.0f)
#define CAMERA_WORLD_UP_DEFAULT glm::vec3(0.0f, 1.0f, 0.0f)

// #define MOUSE_SENSITIVITY_DEFAULT 0.1f

// Engine constants end

// Engine structures definitions
typedef struct {
	unsigned int shader_program;
} ShaderData;

typedef struct {
	unsigned int VBO, VAO, EBO;
	unsigned int num_vertices;
	unsigned int num_indices;
} RenderData;

typedef struct {
	glm::vec3 pos, front, up, right, world_up;
	float yaw, pitch, speed, sensitivity, zoom;
	float fov;
} CameraData;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

enum openGl_ErrorType {
	SHADER_VERTEX,
	SHADER_FRAGMENT,
	PROGRAM
};

// Engine - global data
ShaderData shaderData = {};
RenderData renderData = {};
CameraData cameraData = {};

// Engine - global data - time values
float delta_time = 0.0f;
float last_frame = 0.0f;

// Engine procedures
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	(void) window; // Remove if used for real - it's for warning purposes
	
	glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Camera movements
	float camera_speed = static_cast<float>(cameraData.speed * delta_time);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraData.pos += camera_speed * cameraData.front;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraData.pos -= camera_speed * cameraData.front;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraData.pos -= glm::normalize(glm::cross(cameraData.front, cameraData.right)) * camera_speed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraData.pos += glm::normalize(glm::cross(cameraData.front, cameraData.right)) * camera_speed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraData.pos -= glm::normalize(glm::cross(cameraData.front, cameraData.up)) * camera_speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraData.pos += glm::normalize(glm::cross(cameraData.front, cameraData.up)) * camera_speed;
}

// Mouse values

static bool first_mouse = true;
static bool left_mouse_button_down = false;
static double last_mouse_pos_x = SCREEN_WIDTH / 2.0;
static double last_mouse_pos_y = SCREEN_HEIGHT / 2.0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	(void) mods;
	(void) window;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		printf("Mouse button pressed!\n");
		left_mouse_button_down = true;
	} else {
		left_mouse_button_down = false;
		first_mouse = true;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	(void) window; 

	if (!left_mouse_button_down) return;

	printf("Last mouse pos --> x: %f, y: %f\n", last_mouse_pos_x, last_mouse_pos_y);

	if (first_mouse) {
		printf("First mouse!\n");
		last_mouse_pos_x = xpos;
		last_mouse_pos_y = ypos;
		first_mouse = false;
	}

	float x_offset = static_cast<float>(xpos - last_mouse_pos_x);
	float y_offset = static_cast<float>(last_mouse_pos_y - ypos); // Reversed since y-coordinates go from bottom to top (taken from learnopengl.com)
	last_mouse_pos_x = xpos;
	last_mouse_pos_y = ypos;

	float sensitivity = cameraData.sensitivity;
	x_offset *= sensitivity;
	y_offset *= sensitivity;

	// Camera is integrated in this function, move it somewhere
	cameraData.yaw += x_offset;
	cameraData.pitch += y_offset;

	// Boundries on pitch!
	if (cameraData.pitch > 89.0f) cameraData.pitch = 89.0f;
	if (cameraData.pitch < -89.0f) cameraData.pitch = -89.0f;

	// Proper calculations
	glm::vec3 front = cameraData.front;
	front.x = cos(glm::radians(cameraData.yaw)) * cos(glm::radians(cameraData.pitch));
	front.y = sin(glm::radians(cameraData.pitch));
	front.z = sin(glm::radians(cameraData.yaw)) * cos(glm::radians(cameraData.pitch));
	cameraData.front = glm::normalize(front);
}


void openGl_GetError(openGl_ErrorType et, unsigned int id) {
	char infoLog[512];

	switch (et) {
		case SHADER_VERTEX:
			glGetShaderInfoLog(id, 512, NULL, infoLog);
			printf("Error: --> VERTEX_SHADER: Compilation failed!\n%s\n", infoLog);
			break;
		case SHADER_FRAGMENT:
			glGetShaderInfoLog(id, 512, NULL, infoLog);
			printf("Error: --> FRAGMENT_SHADER: Compilation failed!\n%s\n", infoLog);
			break;
		case PROGRAM:
			glGetProgramInfoLog(id, 512, NULL, infoLog);
			printf("Error: --> SHADER_PROGRAM: Linking failed!\n%s\n", infoLog);
			break;
	}
}

void compile_shader(ShaderData* sd, const char* vertex_shader_src, const char* fragment_shader_src) {
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	unsigned int shaderProgram = glCreateProgram();

	int success = 0;

	glShaderSource(vertexShader, 1, &vertex_shader_src, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) openGl_GetError(openGl_ErrorType::SHADER_VERTEX, vertexShader);

	glShaderSource(fragmentShader, 1, &fragment_shader_src, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) openGl_GetError(openGl_ErrorType::SHADER_FRAGMENT, fragmentShader);

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) openGl_GetError(openGl_ErrorType::PROGRAM, shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	sd->shader_program = shaderProgram;
}

void shader_set_uniform_mat4(ShaderData* sd, const char* name, glm::mat4& mat) {
	// sd->shader_program == ID
	glUniformMatrix4fv(glGetUniformLocation(sd->shader_program, name), 1, GL_FALSE, &mat[0][0]);
	// TODO: Add error checking?
}

void bind_render_data(RenderData* rd, float* vertices, unsigned int* indices, size_t vertices_size, size_t indices_size) {
	glGenVertexArrays(1, &rd->VAO);
	glGenBuffers(1, &rd->VBO);
	glGenBuffers(1, &rd->EBO);

	glBindVertexArray(rd->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rd->VBO);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) vertices_size, vertices, GL_STATIC_DRAW);

	// Position attribute
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei) (vertices_size * sizeof(float)), (void*) 0);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertices_size, (void*) 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// Color attribute
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei) (vertices_size * sizeof(float)), (void*) (3 * sizeof(float)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rd->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) indices_size, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	// Set rd data - num of vertices
	rd->num_vertices = (unsigned int) vertices_size / (6 * sizeof(float));
	rd->num_indices = (unsigned int) indices_size;
}

void render(RenderData* rd) {
	// glUseProgram(shaderData.shader_program); // This is global stuff for now

	glBindVertexArray(rd->VAO);
	
	// glDrawElements(GL_TRIANGLES, (GLsizei) rd->num_indices, GL_UNSIGNED_INT, 0);
	// glDrawElements(GL_TRIANGLES, rd->num_vertices, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, rd->num_indices, GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
}

void delete_render_data(RenderData* rd) {
	glDeleteVertexArrays(1, &rd->VAO);
	glDeleteBuffers(1, &rd->VBO);
	glDeleteBuffers(1, &rd->EBO);
}

// Multiline strings - shaders

// This needs to be kept updated in shaders
// TODO: Find a better solution for that
#define MULTILINE_STR(...) #__VA_ARGS__
// Some tips - # is \x23

const char* model_uniform_name = "uModel";
const char* view_uniform_name = "uView";
const char* projection_uniform_name = "uProjection";

const char* _mvp_vertexShaderSource = MULTILINE_STR(
\x23version 330 core\n

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vColor;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	vColor = aColor;
}
);

const char* _mvp_fragmentShaderSource = MULTILINE_STR(
\x23version 330 core\n

in vec3 vColor;
out vec4 FragColor;

void main() {
	FragColor = vec4(vColor, 1.0);
}
);

const char* vertexShaderSource = MULTILINE_STR(
\x23version 330 core\n

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
	gl_Position = vec4(aPos, 1.0);
	vColor = aColor;
}
);

const char* fragmentShaderSource = MULTILINE_STR(
\x23version 330 core\n

in vec3 vColor;
out vec4 FragColor;

void main() {
	FragColor = vec4(vColor, 1.0);
}
);

void update_camera_vectors(CameraData*);

void initialize_camera(CameraData* camera) {
	camera->yaw         = CAMERA_YAW_DEFAULT;
	camera->pitch       = CAMERA_PITCH_DEFAULT;
	camera->speed       = CAMERA_SPEED_DEFAULT;
	camera->sensitivity = CAMERA_SENSITIVITY_DEFAULT;
	camera->fov         = CAMERA_FOV_DEFAULT;

	camera->pos      = CAMERA_POSITION_DEFAULT;
	camera->front    = CAMERA_FRONT_DEFAULT;
	camera->world_up = CAMERA_WORLD_UP_DEFAULT;

	update_camera_vectors(camera);

}

void update_camera_vectors(CameraData* camera) {
	glm::vec3 front;

	front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
	front.y = sin(glm::radians(camera->pitch));
	front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));

	camera->front = glm::normalize(front);

	camera->right = glm::normalize(glm::cross(camera->front, camera->world_up));
	camera->up    = glm::normalize(glm::cross(camera->right, camera->front));
}

// Game entitites
float test_rectangle[] = {
    0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	 -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
};
unsigned int test_rectangle_indices[] = {
	0, 1, 3,
	1, 2, 3,
};

float cube_vertices[] = {
    // Front face (red)
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

    // Back face (green)
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

    // Top face (blue)
    -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

    // Bottom face (yellow)
    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

    // Right face (cyan)
     0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,

    // Left face (magenta)
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,

    // Back-right face (white)
     0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
};

unsigned int cube_indices[] = {
    // Front face
    0, 1, 2,
    2, 3, 0,

    // Back face
    4, 5, 6,
    6, 7, 4,

    // Top face
    8, 9, 10,
    10, 11, 8,

    // Bottom face
    12, 13, 14,
    14, 15, 12,

    // Right face
    16, 17, 18,
    18, 19, 16,

    // Left face
    20, 21, 22,
    22, 23, 20
};

int main() {
	printf("Hello, Brave New World!\n");

	// Test debug prints
	printf("vertexShaderSource: %s\n", _mvp_vertexShaderSource);
	printf("fragmentShaderSource: %s\n", _mvp_fragmentShaderSource);

	// glfw initialization goes here
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE, NULL, NULL);
	if (!window) {
		printf("Failed to create GLFW window!\n");
		return 1;
	}

	glfwMakeContextCurrent(window); // Only after this call we can initialize GLAD!
	
#pragma warning(suppress: 5039)
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
#pragma warning(suppress: 5039)
	glfwSetCursorPosCallback(window, mouse_callback);
#pragma warning(suppress: 5039)
	glfwSetMouseButtonCallback(window, mouse_button_callback);

#pragma warning(suppress: 4191)
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		printf("Failed to initialize GLAD!\n");
		return 1;
	}

	const GLubyte* version = glGetString(GL_VERSION);
	printf("OpenGL Version: %s\n", version);
	
	// OpenGL setup
	glEnable(GL_DEPTH_TEST);

	// Compiling shaders
	// compile_shader(&shaderData, vertexShaderSource, fragmentShaderSource);
	compile_shader(&shaderData, _mvp_vertexShaderSource, _mvp_fragmentShaderSource);

	// Initialization of camera
	initialize_camera(&cameraData);

	// Uniform for model and projection goes here
	glm::mat4 projection = glm::perspective(glm::radians(cameraData.fov),
			(float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	glm::mat4 model = glm::mat4(1.0f);

	// Binding render data - test rectangle
	// size_t vertices_size = sizeof(test_rectangle);
	// size_t indices_size = sizeof(test_rectangle_indices);
	// bind_render_data(&renderData, test_rectangle, test_rectangle_indices, vertices_size, indices_size);

	// Binding render data - test cube
	size_t vertices_size = sizeof(cube_vertices);
	size_t indices_size = sizeof(cube_indices);
	bind_render_data(&renderData, cube_vertices, cube_indices, vertices_size, indices_size);

	while (!glfwWindowShouldClose(window)) {
		// Time - beginning of the frame
		float current_time = static_cast<float>(glfwGetTime());
		delta_time = current_time - last_frame;
		last_frame = current_time;
		// printf("Delta time: %f\n", delta_time);

		// Input
		// Pool IO events - I think this should be done at the start of the frame
		glfwPollEvents();
		process_input(window);

		// Render - clear screen
		glClearColor(0.75f, 0.65f, 0.24f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Update camera projection
		update_camera_vectors(&cameraData);
		glm::mat4 view = glm::lookAt(cameraData.pos, cameraData.pos + cameraData.front, cameraData.up);

		// Render - the rest of stuff
		glUseProgram(shaderData.shader_program);
		shader_set_uniform_mat4(&shaderData, model_uniform_name, model);
		shader_set_uniform_mat4(&shaderData, view_uniform_name, view);
		shader_set_uniform_mat4(&shaderData, projection_uniform_name, projection);
		render(&renderData);

		// Final stuff - swap buffers
		glfwSwapBuffers(window);
	}

	// Delete graphics resources
	delete_render_data(&renderData);
	glDeleteProgram(shaderData.shader_program);

	printf("All went well, goodbye...\n");
	return 0;
}
