#include "loadSimpleOBJ.h"
#include "Camera.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void loadTrajectory(const std::string& filename, std::vector<glm::vec3>& points);

int setupShader();

const GLuint WIDTH = 1000, HEIGHT = 1000;

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"layout (location = 2) in vec2 texCoord;\n"
"layout (location = 3) in vec3 normal;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"

"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"out vec3 ObjectColor;\n"

"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(position, 1.0);\n"
"   FragPos = vec3(model * vec4(position, 1.0));\n"
"   Normal = mat3(model) * normal;\n"
"   ObjectColor = color;\n"
"	TexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"in vec3 ObjectColor;\n"
"in vec2 TexCoord;\n"

"uniform vec3 lightPos[3];\n"
"uniform vec3 lightColor[3];\n"
"uniform float lightIntensity[3];\n"
"uniform bool lightEnabled[3];\n"
"uniform sampler2D texture1;\n"

"out vec4 color;\n"

"void main()\n"
"{\n"
    "vec3 norm = normalize(Normal);\n"
    "vec3 finalLight = vec3(0.0);\n"

    "for(int i = 0; i < 3; i++)\n"
    "{\n"
        "if(!lightEnabled[i])\n"
            "continue;\n"

        "vec3 lightDir = normalize(lightPos[i] - FragPos);\n"
        "float diff = max(dot(norm, lightDir), 0.0);\n"
        "vec3 diffuse = diff * lightColor[i] * lightIntensity[i];\n"
        "float distance = length(lightPos[i] - FragPos);\n"
        "float attenuation = 1.0 / (1.0 + 0.2 * distance * distance);\n"
        "diffuse *= attenuation;\n"
        "finalLight += diffuse;\n"
    "}\n"

	"vec3 texColor = texture(texture1, TexCoord).rgb;\n"
	"vec3 result = finalLight * texColor;\n"
    "color = vec4(result, 1.0);\n"
"}\n\0";

GLuint texture;

bool rotateX = false, rotateY = false, rotateZ = false;
int lightsEnabled[3] = { 1, 1, 1 };

glm::vec3 position(0.0f, 0.0f, 0.0f);
float scale = 1.0f;

std::vector<glm::vec3> suzanne = {
    glm::vec3(0,0,0),
    glm::vec3(3,0,0),
    glm::vec3(-3,0,0)
};

glm::vec3 keyLight;
glm::vec3 fillLight;
glm::vec3 backLight;

bool firstMouse = true;

float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int currentPoint = 0;
float movementSpeed = 1.0f;
bool trajetoriaAtiva = false;

Camera camera;

void processInput(GLFWwindow* window)
{
    camera.ProcessKeyboard(
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS,
        deltaTime
    );
}

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH,HEIGHT,"Ola 3D -- Lucas!",nullptr,nullptr);

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);

    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    std::vector<glm::vec3> trajetoria;

    loadTrajectory("../assets/trajetoria.txt",trajetoria);

    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();

	int nVertices;
	GLuint VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVertices);

    glUseProgram(shaderID);

   	glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

	GLint lightPosLoc =	glGetUniformLocation(shaderID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderID, "lightColor");
	GLint lightIntensityLoc = glGetUniformLocation(shaderID, "lightIntensity");
	GLint lightEnabledLoc =	glGetUniformLocation(shaderID, "lightEnabled");

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST); 

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int texWidth, texHeight, nrChannels;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load("../assets/Modelos3D/Suzanne.png", &texWidth, &texHeight, &nrChannels, 0);

	if (data)
		{GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D,	0, format, texWidth, texHeight,	0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Erro ao carregar textura" << endl;
	}

	stbi_image_free(data);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;

        lastFrame = currentFrame;

        glfwPollEvents();

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = (GLfloat)glfwGetTime();

		glm::vec3 mainObjectPos = suzanne[0] + position;
		keyLight  = mainObjectPos + glm::vec3( 2.0f * scale, 2.0f * scale, 2.0f * scale);
		fillLight = mainObjectPos + glm::vec3(-2.0f * scale, 1.0f * scale, 2.0f * scale);
		backLight = mainObjectPos + glm::vec3( 0.0f, 1.0f * scale, -3.0f * scale);

		glm::vec3 lightPositions[3] = {keyLight, fillLight, backLight};
		glm::vec3 lightColors[3] = {
			glm::vec3(1.0f, 1.0f, 1.0f), // key
			glm::vec3(1.0f, 1.0f, 1.0f), // fill
			glm::vec3(1.0f, 1.0f, 1.0f)  // back
			};

		float intensities[3] = {1.0f, 0.5f, 1.0f};
			//principal, preenchimento e fundo

		glUniform3fv(lightPosLoc, 3, glm::value_ptr(lightPositions[0]));
		glUniform3fv(lightColorLoc, 3, glm::value_ptr(lightColors[0]));
		glUniform1fv(lightIntensityLoc, 3, intensities);
		glUniform1iv(lightEnabledLoc, 3, lightsEnabled);

        glm::mat4 view = camera.GetViewMatrix();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        for (auto& suzannePos : suzanne)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, suzannePos + position);

            model = glm::scale(model, glm::vec3(scale));

            if (rotateX)
                model = glm::rotate(model, angle, glm::vec3(1,0,0));
            else if (rotateY)
                model = glm::rotate(model, angle, glm::vec3(0,1,0));
            else if (rotateZ)
                model = glm::rotate(model, angle, glm::vec3(0,0,1));
                    
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

            glBindVertexArray(VAO);

			glDrawArrays(GL_TRIANGLES, 0, nVertices);
		}

        if(trajetoriaAtiva &&!trajetoria.empty())
        {
            glm::vec3 target = trajetoria[currentPoint];

            glm::vec3 direction = target - position;

            float distance = glm::length(direction);

            if(distance < 0.05f)
            {   currentPoint++;
                if(currentPoint >= trajetoria.size())
                    currentPoint = 0;
            }
            else
            {   position += glm::normalize(direction) * movementSpeed * deltaTime;}
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        trajetoriaAtiva = !trajetoriaAtiva;
    }

    float step = 0.1f;

    if (key == GLFW_KEY_UP) position.z -= step;
    if (key == GLFW_KEY_DOWN) position.z += step;
    if (key == GLFW_KEY_LEFT) position.x -= step;
    if (key == GLFW_KEY_RIGHT) position.x += step;
    if (key == GLFW_KEY_I) position.y += step;
    if (key == GLFW_KEY_J) position.y -= step;

    if (key == GLFW_KEY_LEFT_BRACKET) scale -= 0.1f;
    if (key == GLFW_KEY_RIGHT_BRACKET) scale += 0.1f;

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        suzanne.push_back(position);
    }

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    	lightsEnabled[0] = !lightsEnabled[0];

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		lightsEnabled[1] = !lightsEnabled[1];

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		lightsEnabled[2] = !lightsEnabled[2];
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessScroll((float)yoffset);
}

void loadTrajectory(const std::string& filename, std::vector<glm::vec3>& points)
{
    std::ifstream file(filename);

    if(!file.is_open())
    {
        std::cout << "Erro ao abrir trajetória\n";
        return;
    }

    float x, y, z;

    while(file >> x >> y >> z)
    {
        points.push_back(glm::vec3(x,y,z));
    }

    file.close();
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);

        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);

        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);

        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}