//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//        Trabalho Final de Computação Gráfica
//       Alunos: Afonso Ferrer e Victoria Alves

#include "callbacks.h"
#include <time.h>

int main(int argc, char* argv[])
{
    srand (time(NULL));
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com a OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Pedimos apra utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas da OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Nyan Cat Wants Milk", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas pela OpenGL 3.3, utilizando a
    // biblioteca GLAD. Veja
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slide 216 do documento
    // "Aula_03_Rendering_Pipeline_Grafico.pdf".
    //
    LoadShadersFromFiles();

    // Carregamos n imagens para serem utilizadas como textura
    LoadTextureImage("../../data/skyTexture.jpg");      // TextureImage0
    LoadTextureImage("../../data/houseTexture.jpg");    // TextureImage1
    LoadTextureImage("../../data/groundTexture.jpg");   // TextureImage2
    LoadTextureImage("../../data/cowTexture.jpg");      // TextureImage3
    LoadTextureImage("../../data/catTexture.jpg");      // TextureImage4

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel cowmodel("../../data/cow.obj");
    ComputeNormals(&cowmodel);
    BuildTrianglesAndAddToVirtualScene(&cowmodel);

    ObjModel shipmodel("../../data/cat.obj");
    ComputeNormals(&shipmodel);
    BuildTrianglesAndAddToVirtualScene(&shipmodel);

    ObjModel poolmodel("../../data/rainbowpool.obj");
    ComputeNormals(&poolmodel);
    BuildTrianglesAndAddToVirtualScene(&poolmodel);

    ObjModel barnmodel("../../data/house.obj");
    ComputeNormals(&barnmodel);
    BuildTrianglesAndAddToVirtualScene(&barnmodel);

    glm::vec3 cowB_min = g_VirtualScene["cow"].boundingBox_min;
    glm::vec3 cowB_max = g_VirtualScene["cow"].boundingBox_max;

    glm::vec3 catB_min = g_VirtualScene["cat"].boundingBox_min;
    glm::vec3 catB_max = g_VirtualScene["cat"].boundingBox_max;

    glm::vec3 houseB_min = g_VirtualScene["house"].boundingBox_min;
    glm::vec3 houseB_max = g_VirtualScene["house"].boundingBox_max;

    glm::mat4 house_model = Matrix_Translate(g_house_position.x, g_house_position.y, g_house_position.z);

    g_cowM_min = glm::vec4(cowB_min.x, cowB_min.y, cowB_min.z, 1.0f);
    g_cowM_max = glm::vec4(cowB_max.x, cowB_max.y, cowB_max.z, 1.0f);

    g_catM_min = glm::vec4(catB_min.x, catB_min.y, catB_min.z, 1.0f);
    g_catM_max = glm::vec4(catB_max.x, catB_max.y, catB_max.z, 1.0f);

    g_houseW_min = house_model * glm::vec4(houseB_min.x, houseB_min.y, houseB_min.z, 1.0f);
    g_houseW_max = house_model * glm::vec4(houseB_max.x, houseB_max.y, houseB_max.z, 1.0f);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 46 à 66 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 22 à 33 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    // Cria vaquinhas em posições aleatórias
    float randomPosX;
    float randomPosZ;

    int negativeZ;
    int negativeX;

    float X;
    float Z;

    for(int i=0; i<NUM_COWS;i++){
        randomPosX = rand() % 100 + 1;
        negativeX = rand() % 2;
        randomPosZ = rand() % 100 + 1;
        negativeZ = rand() % 2;

        if(negativeX == 0)
            X = 0 - randomPosX;
        else
            X = randomPosX;

        if(negativeZ == 0)
            Z = 0 - randomPosZ;
        else
            Z = randomPosZ;
        cows[i].pos = glm::vec4(X, -0.5f, Z, 1.0f);

    }

    glm::vec4 cow_lookat = glm::vec4(1.0f,0.0f,0.0f,0.0f);

    float c = 0;

    glm::vec4 cow_y = glm::vec4(0.0f, 0.5f,0.0f, 0.0f);

    for(int i=0; i<NUM_COWS;i++){
        cows[i].direction = normalize(g_house_position - (cows[i].pos - cow_y));
        c = dot(cow_lookat, cows[i].direction);
        cows[i].angle = (cows[i].pos.z <=0)? -acos(c): acos(c);
    }

    double t0 = glfwGetTime();
    double t1;
    double t;
    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))

    {
        if(game == GAME){
            // Aqui executamos as operações de renderização

            // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
            // e também resetamos todos os pixels do Z-buffer (depth buffer).
            glClear(GL_DEPTH_BUFFER_BIT);

            // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
            // os shaders de vértice e fragmentos)
            glUseProgram(g_program_id);

            // Computamos a posição da câmera utilizando coordenadas esféricas.  As
            // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
            // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
            // e ScrollCallback().
            if(!g_probe_on){
                g_camera_lookat_l.y = g_CameraDistance * sin(g_CameraPhi);
                g_camera_lookat_l.z = g_CameraDistance * cos(g_CameraPhi)*cos(g_CameraTheta);
                g_camera_lookat_l.x = g_CameraDistance * cos(g_CameraPhi)*sin(g_CameraTheta);

                g_camera_position_c = g_cat_position + g_camera_lookat_l;

                g_camera_lookat_l = g_camera_lookat_l/norm(g_camera_lookat_l);
            }else{
                g_probe_lookat.y = sin(g_CameraPhi);
                g_probe_lookat.z = cos(g_CameraPhi)*cos(g_CameraTheta);
                g_probe_lookat.x = cos(g_CameraPhi)*sin(g_CameraTheta);

                //g_probe_lookat = g_probe_lookat/norm(g_probe_lookat);
            }

            // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
            // Veja slide 159 do documento "Aula_08_Sistemas_de_Coordenadas.pdf".
                glm::vec4 camera_view_vector = (g_probe_on)? -g_probe_lookat:-g_camera_lookat_l; // Vetor "view", sentido para onde a câmera está virada

            // Computamos a matriz "View" utilizando os parâmetros da câmera para
            // definir o sistema de coordenadas da câmera.  Veja slide 162 do
            // documento "Aula_08_Sistemas_de_Coordenadas.pdf".
            glm::mat4 view = Matrix_Camera_View((g_probe_on)? g_probe_position:g_camera_position_c, camera_view_vector, g_camera_up_vector);

            // Agora computamos a matriz de Projeção.
            glm::mat4 projection;

            // Note que, no sistema de coordenadas da câmera, os planos near e far
            // estão no sentido negativo! Veja slides 180-183 do documento
            // "Aula_09_Projecoes.pdf".
            float nearplane = -0.1f;  // Posição do "near plane"
            float farplane  = -100.0f; // Posição do "far plane"

            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

            model = Matrix_Identity(); // Transformação identidade de modelagem

            // Enviamos as matrizes "view" e "projection" para a placa de vídeo
            // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
            // efetivamente aplicadas em todos os pontos.
            glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
            glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

            glDisable(GL_CULL_FACE);

            if (!g_probe_on){
                model = Matrix_Translate(g_camera_position_c.x, g_camera_position_c.y, g_camera_position_c.z);
            }else{
                model = Matrix_Translate(g_probe_position.x, g_probe_position.y, g_probe_position.z);
            }
            model = model * Matrix_Scale(SKY_SIZE, SKY_SIZE, SKY_SIZE);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, SPHERE);
            DrawVirtualObject("sphere");

            // Definindo o raio de arco-iris do Nyan Cat
            glUniform4f(g_cat_light_uniform, g_cat_position.x, g_cat_position.y, g_cat_position.z, 1.0f);
            g_cat_model = Matrix_Translate(g_cat_position.x, g_cat_position.y, g_cat_position.z);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(g_cat_model));
            glUniform1i(g_object_id_uniform, CAT);
            DrawVirtualObject("cat");

            glEnable(GL_CULL_FACE);

            // Desenhamos o plano do chão
            model = Matrix_Scale(GROUND_SIZE, 1.0f, GROUND_SIZE);
            model = model * Matrix_Translate(0.0f,-1.1f,0.0f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, PLANE);
            DrawVirtualObject("plane");

            // Desenha a casinha
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(house_model));
            glUniform1i(g_object_id_uniform, HOUSE);
            DrawVirtualObject("house");

            // Desenha as vaquinhas
            if(g_under_rainbow_ray){
                Maintain_MilkSearch();
            }
            for(int i=0; i<NUM_COWS;i++){
                DrawCow(i);
            }
            t1 = glfwGetTime();
            t = t1 - t0;
            if (t > 0.3){
                MoveCow();
                t0 = t1;
            }

            // Imprimimos na tela os ângulos de Euler que controlam a rotação do
            // terceiro cubo.
            TextRendering_ShowInfo(window);

            // Imprimimos na tela informação sobre o número de quadros renderizados
            // por segundo (frames per second).
            TextRendering_ShowFramesPerSecond(window);

            // O framebuffer onde a OpenGL executa as operações de renderização não
            // é o mesmo que está sendo mostrado para o usuário, caso contrário
            // seria possível ver artefatos conhecidos como "screen tearing". A
            // chamada abaixo faz a troca dos buffers, mostrando para o usuário
            // tudo que foi renderizado pelas funções acima.
            glfwSwapBuffers(window);

        }else{
            g_score = g_milkCows * g_score_MILK + g_gliterredCows * g_score_diamond;
            TextRendering_Showg_score(window);
            glfwSwapBuffers(window);
        }

        // Verifica com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos os recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

