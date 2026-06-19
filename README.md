# Projeto de Computação Gráfica - Colocações das Suzannes

## Dependências:
Compilador C++
OpenGL 4.5+
GLFW 3
GLAD
GLM (OpenGL Mathematics)
stb_image.h

## ⚠️ **IMPORTANTE: Baixar a GLAD Manualmente**
Para que o projeto funcione corretamente, é necessário **baixar a GLAD manualmente** utilizando o **GLAD Generator**.

### 🔗 **Acesse o web service do GLAD**:
👉 [GLAD Generator](https://glad.dav1d.de/)

### ⚙️ **Configuração necessária:**
- **API:** OpenGL  
- **Version:** 3.3+ (ou superior compatível com sua máquina)  
- **Profile:** Core  
- **Language:** C/C++  

### 📥 **Baixe e extraia os arquivos:**
Após a geração, extraia os arquivos baixados e coloque-os nos diretórios correspondentes:
- Copie **`glad.h`** para `include/glad/`
- Copie **`khrplatform.h`** para `include/glad/KHR/`
- Copie **`glad.c`** para `common/`

### Compilação e Execução
As compilações desse projeto foram feitas utilizando o CMAKE.
O projeto não depende de arquivos externos e deve compilar tranquilamente usando os arquivos atuais do repositorio contanto que estejam na mesma organiazação de pastas.

Basta usar o CMAKE: Configure e depois dentro da pasta build usar o comando "cmake --build ." para criar o executavel que deve ter o nome de Cubos.exe, executando ele com o comando ".\Cubos.exe".

### Assets
#### Modelos
Suzanne.obj: Modelo padrão de teste incluído nativamente no software Blender.
Cube.obj: Modelo de cubo padrão gerado no Blender.

OBS: Todos os obj foram pegos do repositório do professor da disciplina.

#### Texturas
Suzanne.png: Gerada no software Blender via UV Unwrapping e Texture Painting básica.
Gold.png: Pego deste site https://gallery.yopriceville.com/Backgrounds/Gold_Metal_Background
Silver.png: Pego deste site https://gallery.yopriceville.com/Backgrounds/Silver_Metal_Background
Brick.png: Pego deste site https://gallery.yopriceville.com/Backgrounds/Brick_Wall_Background

OBS: Textura da Suzanne também foi pega do repositório do professor da disciplina.

### Referências
Materiais do Moodle fornecidos pelo professor, como o arquivo tutorial da camera do M5:
M5 Apresentação em PDF | Câmera sintética Arquivo

Tutorial para calculo de Bezier: https://javascript.info/bezier-curve

Funções GL: OpenGL API Documentation khronos.org/registry/OpenGL-Refpages

Transformações de matriz: GLM Documentation (G-Truc) glm.g-truc.net

Callbacks de mouse e teclado: GLFW Documentation glfw.org/docs