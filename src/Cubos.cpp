#include "loadSimpleOBJ.h"
#include "Camera.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

//Prototipos
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void lerTrajetoria(const std::string& filename, std::vector<glm::vec3>& points);
glm::vec3 Bezier(float t, const std::vector<glm::vec3>& p);
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
    "gl_Position = projection * view * model * vec4(position, 1.0);\n"
    "FragPos = vec3(model * vec4(position, 1.0));\n"
    "Normal = mat3(model) * normal;\n"
    "ObjectColor = color;\n"
    "TexCoord = texCoord;\n"
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
"uniform bool useMaterial;\n"
"uniform vec3 materialKa;\n"
"uniform vec3 materialKd;\n"
"uniform vec3 materialKs;\n"
"uniform float materialNs;\n"
"uniform vec3 viewPos;\n"

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

        "// Cálculo da iluminação difusa (Lei de Lambert)\n"
        "float diff = max(dot(norm, lightDir), 0.0);\n"

        "vec3 diffuse = diff * lightColor[i] * lightIntensity[i];\n"
        "float distance = length(lightPos[i] - FragPos);\n"

        "// Atenuação da intensidade luminosa pela distância\n"
        "float attenuation = 1.0 / (1.0 + 0.2 * distance * distance);\n"
        "diffuse *= attenuation;\n"

        "vec3 viewDir = normalize(viewPos - FragPos);\n"

        "vec3 reflectDir = reflect(-lightDir, norm);\n"

        "// Cálculo da reflexão especular (modelo de Phong)\n"
        "float spec = pow(max(dot(viewDir, reflectDir), 0.0),materialNs);\n"

        "vec3 specular = materialKs * spec * lightColor[i] * lightIntensity[i];\n"
        "specular *= attenuation;\n"

        "finalLight += diffuse + specular;\n"
    "}\n"

	"vec3 texColor = texture(texture1, TexCoord).rgb;\n"
	"vec3 surfaceColor = texColor;\n"
    "if(useMaterial)\n"
    "{\n"
        "surfaceColor = materialKd;\n"
    "}\n"
    "else\n"
    "{\n"
        "surfaceColor = texture(texture1, TexCoord).rgb;\n"
    "}\n"

    "// Componente ambiente do material\n"
    "vec3 ambient = materialKa * 0.2;\n"

    "// Resultado final da iluminação aplicada sobre a superfície\n"
    "vec3 result = (finalLight + ambient) * surfaceColor;\n"
    "color = vec4(result, 1.0);\n"
"}\n\0";

GLuint texture;
GLuint cubeTextures[3];
bool usarMaterial = false;

int luzesAtivas[3] = { 1, 1, 1 };
float intensidades[3] = {1.0f, 0.5f, 1.0f};
glm::vec3 luzPrincipal;
glm::vec3 luzPreenchimento;
glm::vec3 luzFundo;

std::vector<glm::vec3> cubos = {
    glm::vec3(0.0f, 0.9f, 0.0f),
    glm::vec3(3.0f, -0.6f, 0.0f),
    glm::vec3(-3.0f, -2.1f, 0.0f)
};
std::vector<glm::vec3> posicoes = {
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f)
};
std::vector<glm::vec3> suzanne = {
    glm::vec3(0.0f,3.0f,0.0f),
    glm::vec3(3.0f,1.5f,0.0f),
    glm::vec3(-3.0f,0.0f,0.0f)
};
int objetoSelecionado = 0;
std::vector<float> escalas = {1.0f, 1.0f, 1.0f};
std::vector<int> rotacao = {0, 0, 0};

bool mouse = true;
float ultimoX = WIDTH / 2.0f;
float ultimoY = HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera;

int localAtual = 0;
float velocidade = 1.0f;
bool trajetoriaAtiva = false;
float bezierT = 0.0f;
bool indo = true;

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH,HEIGHT,"Colocações das Suzannes",nullptr,nullptr);

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    glViewport(0, 0, width, height);

    std::vector<glm::vec3> trajetoria;
    lerTrajetoria("../assets/trajetoria.txt",trajetoria);

    GLuint shaderID = setupShader();

	int nVertices;
	GLuint VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVertices);

    int cubeVertices;
    GLuint cubeVAO = loadSimpleOBJ("../assets/Modelos3D/Cube.obj", cubeVertices);

    glUseProgram(shaderID);

   	glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

	GLint lightPosLoc =	glGetUniformLocation(shaderID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderID, "lightColor");
	GLint lightIntensityLoc = glGetUniformLocation(shaderID, "lightIntensity");
	GLint lightEnabledLoc =	glGetUniformLocation(shaderID, "lightEnabled");

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    GLint useMaterialLoc = glGetUniformLocation(shaderID, "useMaterial");
    GLint kaLoc = glGetUniformLocation(shaderID, "materialKa");
    GLint kdLoc = glGetUniformLocation(shaderID, "materialKd");
    GLint ksLoc = glGetUniformLocation(shaderID, "materialKs");
    GLint nsLoc = glGetUniformLocation(shaderID, "materialNs");
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

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

    glGenTextures(1, &cubeTextures[0]);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* data1 = stbi_load("../assets/Modelos3D/Gold.png", &texWidth, &texHeight, &nrChannels, 4);
    if (data1)
		{GLenum format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D,	0, format, texWidth, texHeight,	0, format, GL_UNSIGNED_BYTE, data1);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Erro ao carregar textura" << endl;
	}
	stbi_image_free(data1);

    glGenTextures(1, &cubeTextures[1]);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* data2 = stbi_load("../assets/Modelos3D/Silver.png", &texWidth, &texHeight, &nrChannels, 4);
    if (data2)
		{GLenum format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D,	0, format, texWidth, texHeight,	0, format, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Erro ao carregar textura" << endl;
	}
	stbi_image_free(data2);

    glGenTextures(1, &cubeTextures[2]);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* data3 = stbi_load("../assets/Modelos3D/Brick.png", &texWidth, &texHeight, &nrChannels, 4);
    if (data3)
		{GLenum format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D,	0, format, texWidth, texHeight,	0, format, GL_UNSIGNED_BYTE, data3);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Erro ao carregar textura" << endl;
	}
	stbi_image_free(data3);

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

		glm::vec3 mainObjectPos = suzanne[objetoSelecionado] + posicoes[objetoSelecionado];
		luzPrincipal  = glm::vec3( 2.0f, 2.0f, 2.0f);
        luzPreenchimento = glm::vec3(-2.0f, 1.0f, 2.0f);
        luzFundo = glm::vec3( 0.0f, 1.0f,-3.0f);

		glm::vec3 lightposicoes[3] = {luzPrincipal, luzPreenchimento, luzFundo};
		glm::vec3 lightColors[3] = {
			glm::vec3(1.0f, 1.0f, 1.0f), // key
			glm::vec3(1.0f, 1.0f, 1.0f), // fill
			glm::vec3(1.0f, 1.0f, 1.0f)  // back
			};

		glUniform3fv(lightPosLoc, 3, glm::value_ptr(lightposicoes[0]));
		glUniform3fv(lightColorLoc, 3, glm::value_ptr(lightColors[0]));
		glUniform1fv(lightIntensityLoc, 3, intensidades);
		glUniform1iv(lightEnabledLoc, 3, luzesAtivas);

        glUniform1i(useMaterialLoc, usarMaterial);
        glUniform3fv(kaLoc, 1, glm::value_ptr(material.Ka));
        glUniform3fv(kdLoc, 1, glm::value_ptr(material.Kd));
        glUniform3fv(ksLoc, 1, glm::value_ptr(material.Ks));
        glUniform1f(nsLoc, material.Ns);

        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.Position));

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        for (int i = 0; i < suzanne.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, suzanne[i] + posicoes[i]);

            model = glm::scale(model, glm::vec3(escalas[i]));

            if (rotacao[i] == 1)
                model = glm::rotate(model, angle, glm::vec3(1,0,0));
            else if (rotacao[i] == 2)
                model = glm::rotate(model, angle, glm::vec3(0,1,0));
            else if (rotacao[i] == 3)
                model = glm::rotate(model, angle, glm::vec3(0,0,1));
                    
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(VAO);

			glDrawArrays(GL_TRIANGLES, 0, nVertices);
		}

        for (int i = 0; i < cubos.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, cubos[i]);
            model = glm::scale(model, glm::vec3(1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i]);

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertices);
        }

        //Trajetoria de Bezier, indo e voltando
        if(trajetoriaAtiva && trajetoria.size() >= 4)
        {
            if(indo)
                bezierT += velocidade * deltaTime * 0.2f;
            else
                bezierT -= velocidade * deltaTime * 0.2f;

            if(bezierT >= 1.0f)
            {
                bezierT = 1.0f;
                indo = false;
            }

            if(bezierT <= 0.0f)
            {
                bezierT = 0.0f;
                indo = true;
            }

            posicoes[objetoSelecionado] = Bezier(bezierT, trajetoria);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();

    return 0;
}

//Função de cada tecla
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    //Fecha o programa
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    //Rotações
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotacao[objetoSelecionado] = 1;
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotacao[objetoSelecionado] = 2;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        rotacao[objetoSelecionado] = 3;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        rotacao[objetoSelecionado] = 0;
    }

    //Ativar e desativar trajetoria
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        trajetoriaAtiva = !trajetoriaAtiva;
    }

    //Mover Suzanne
    float step = 0.1f;
    if (key == GLFW_KEY_UP) 
    {
        posicoes[objetoSelecionado].y += step;
    }

    if (key == GLFW_KEY_DOWN)
    {
        posicoes[objetoSelecionado].y -= step;
    } 

    if (key == GLFW_KEY_LEFT) 
    {
        posicoes[objetoSelecionado].x -= step;
    }

    if (key == GLFW_KEY_RIGHT) 
    {
        posicoes[objetoSelecionado].x += step;
    }

    if (key == GLFW_KEY_F) 
    {
        posicoes[objetoSelecionado].z += step;
    }

    if (key == GLFW_KEY_G) 
    {
        posicoes[objetoSelecionado].z -= step;
    }

    //Controlar tamanho
    if (key == GLFW_KEY_LEFT_BRACKET) 
    {
        escalas[objetoSelecionado] -= 0.1f;
    }
    if (key == GLFW_KEY_RIGHT_BRACKET) 
    {
        escalas[objetoSelecionado] += 0.1f;
    }

    //Selecionar a Suzanne
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        objetoSelecionado = 2;
    }

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
        objetoSelecionado = 0;
    }	

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
        objetoSelecionado = 1;
    }

    //Controle de Luzes
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        luzesAtivas[0] = !luzesAtivas[0];
    }

	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
        luzesAtivas[1] = !luzesAtivas[1];
    }	

	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
        luzesAtivas[2] = !luzesAtivas[2];
    }

    if (key == GLFW_KEY_MINUS){
        for(int i = 0; i < 3; i++)
            intensidades[i] -= 0.1f;
    }

    if (key == GLFW_KEY_EQUAL){
        for(int i = 0; i < 3; i++)
            intensidades[i] += 0.1f;
    }

    //Alterna entre material e textura
    if(key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        usarMaterial = !usarMaterial;
    }
    
}

//Funções de mouse para a camera conforme tutorial do moodle
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(mouse)
    {
        ultimoX = xpos;
        ultimoY = ypos;
        mouse = false;
    }

    float xoffset = xpos - ultimoX;
    float yoffset = ultimoY - ypos;

    ultimoX = xpos;
    ultimoY = ypos;

    camera.ProcessMouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessScroll((float)yoffset);
}

//Função do movimento do teclado tambem conforme tutorial
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

//Função pra ler a trajetoria do arquivo
void lerTrajetoria(const std::string& filename, std::vector<glm::vec3>& points)
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

//Calculo de Bezier
glm::vec3 Bezier(float t, const std::vector<glm::vec3>& p)
{
    glm::vec3 a = glm::mix(p[0], p[1], t);
    glm::vec3 b = glm::mix(p[1], p[2], t);
    glm::vec3 c = glm::mix(p[2], p[3], t);

    glm::vec3 d = glm::mix(a, b, t);
    glm::vec3 e = glm::mix(b, c, t);

    return glm::mix(d, e, t);
}

//Função para carregar os shaders
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