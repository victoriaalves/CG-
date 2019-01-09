#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>


#include <cstdio>
#include <cstdlib>


#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <tiny_obj_loader.h>


#include <stb_image.h>


#include "utils.h"
#include "matrices.h"


struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};

struct SceneObject
{
    std::string  name;                   // Nome do objeto
    void*        first_index;            // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int          num_indices;            // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode;         // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    boundingBox_min;               // Axis-Aligned Bounding Box do objeto
    glm::vec3    boundingBox_max;
};

struct NyanAndCowsGame
{
    glm::vec4 pos = glm::vec4(0.0f,0.0f,0.0f, 1.0f);                              // Posição da vaquinha
    glm::mat4 model = Matrix_Identity() * Matrix_Translate(pos.x, pos.y, pos.z);
    glm::vec4 direction;
    float angle = 0;                                                              // Ângulo da direção da vaquinha
    bool alive = true; 
    int abducted = 0;
    bool safe = false;
};

std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

#define INC         0.2f
#define ALPHA       0.6587347
#define GROUND_SIZE 100.0f
#define WALL_DIST   10.0f      // O quão longe do precipício Nyan Cat voa
#define SKY_SIZE    50.0

#define SPHERE        0
#define PLANE         2

#define COW           3
#define CAT           4
#define RAINBOWPOOL   5
#define HOUSE         6

#define NUM_COWS      42

#define ENDGAME_TAYLOR_FT_ED       2
#define GAME                       1

#define g_score_MILK     230
#define g_score_diamond  -50

int game = GAME;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;  // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f;     // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;       // Ângulo em relação ao eixo Y
float g_CameraDistance = 10.0f; // Distância da câmera para a origem

//vetores tem que ser definiidos globalmente para serem acessados na função de callback
glm::vec4 g_camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
glm::vec4 g_cat_position       = glm::vec4(3.0f,3.5f,3.0f,1.0f);
glm::vec4 g_camera_lookat_l    = glm::vec4(0.0f,0.0f,g_CameraDistance,0.0f);
glm::vec4 g_camera_position_c  = g_camera_lookat_l + g_cat_position;
glm::vec4 g_probe_position     = glm::vec4(3.0f, 0.0f, 0.0f, 0.0f) + g_cat_position;
glm::vec4 g_probe_lookat       = glm::vec4(0.0f,0.0f, 1.0f, 0.0f);
glm::vec4 g_house_position     = glm::vec4(0.0f,-1.0f,0.0f,1.0f);

// Variáveis globais 
glm::vec4 g_cowM_min;
glm::vec4 g_cowM_max;

glm::vec4 g_catM_min;
glm::vec4 g_catM_max;

glm::vec4 g_houseW_min;
glm::vec4 g_houseW_max;

glm::mat4 g_cat_model;

glm::vec4 g_probe_min = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
glm::vec4 g_probe_max = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);

bool g_under_rainbow_ray = false;
int  g_milk_cow = -1;
bool g_probe_on = false;
int  g_freeCows = NUM_COWS;
int  g_milkCows = 0;
int  g_gliterredCows = 0;
int  g_safeCows = 0;
int  g_score = 0;

NyanAndCowsGame cows[NUM_COWS];

//model global por perfomance
glm::mat4 model = Matrix_Identity();

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_vertex_shader_id;
GLuint g_fragment_shader_id;
GLuint g_program_id = 0;
GLint  g_model_uniform;
GLint  g_view_uniform;
GLint  g_projection_uniform;
GLint  g_object_id_uniform;
GLint  g_boundingBox_min_uniform;
GLint  g_boundingBox_max_uniform;
GLint  g_cat_light_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

double g_LastCursorPosX, g_LastCursorPosY;
