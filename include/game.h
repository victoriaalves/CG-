#include "others.h"
#include <stdlib.h>
void DrawCow(int i); // Desenha uma vaquinha

void MilkCow();

bool boundingBox_collision(glm::vec4 A_min, glm::vec4 A_max, glm::vec4 B_min, glm::vec4 B_max); //Função que testa colisão entre duas bounding boxes A e B recebe duas boundingBox EM COORDENADAS GLOBAIS

int  NyanNyanSuperCuteMilkCollision3k(); // Função que testa se há colisão do gatinho contra alguma vaquinha e retorne ou o índice desta vaca ou -1 caso não houver colisão

void Maintain_MilkSearch();

bool CatInTheWall(); // Função que intersecta gatinho com as paredes

bool CowInsideHome(int i); // Função que retorna se alguma vaquinha chegou na casa do fazendeiro

void MoveCow(); // Movimenta as vaquinhas

int  Probe_collision(); // Função que retorna se a vaquinha colidiu em algum objeto

bool plane_cross_dressY(glm::vec4 A_min, glm::vec4 A_max, glm::vec4 plane_section);

/****************************************************************************************************************************************************************/

void DrawCow(int i){
    cows[i].model = Matrix_Translate(cows[i].pos.x, cows[i].pos.y, cows[i].pos.z) * Matrix_Rotate_Y(cows[i].angle);
    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(cows[i].model));

    if(cows[i].alive && !cows[i].safe){
        glUniform1i(g_object_id_uniform, COW);
        DrawVirtualObject("cow");
    }
    else if (cows[i].pos.y == -1.0f){
        glUniform1i(g_object_id_uniform, RAINBOWPOOL);
        DrawVirtualObject("rainbowpool");
    }
}

// detecta se o objeto cruzou o plano Y
bool plane_cross_dressY(glm::vec4 A_min, glm::vec4 A_max, float plane_section){
    return (A_min.y >= plane_section || A_max.y >= plane_section);
}


bool boundingBox_collision(glm::vec4 A_min, glm::vec4 A_max, glm::vec4 B_min, glm::vec4 B_max){
    return (A_min.x <= B_max.x && A_max.x >= B_min.x) &&
           (A_min.y <= B_max.y && A_max.y >= B_min.y) &&
           (A_min.z <= B_max.z && A_max.z >= B_min.z);
}

int NyanNyanSuperCuteMilkCollision3k(){
    int i;

    glm::vec4 NyanNyan_min = g_cat_model * g_catM_min;
    glm::vec4 NyanNyan_max = g_cat_model * g_catM_max;

    // Para que quando bata no raio ele já considere colidido com a vaquinha
    NyanNyan_min.y = -10;
    NyanNyan_max.y = 100;

    glm::vec4 cowW_min;
    glm::vec4 cowW_max;
    for(i=0; i < NUM_COWS; i++){
        if(!cows[i].alive) continue;
        cowW_min = cows[i].model * g_cowM_min;
        cowW_max = cows[i].model * g_cowM_max;
        if(boundingBox_collision(cowW_min, cowW_max, NyanNyan_min,NyanNyan_max)){
            return i;
        }
    }
    return -1;
}

void MoveCow(){
    int i;
    for (i = 0; i < NUM_COWS; i++){
        int movingParameter = rand() % 3 + 1;
        if (cows[i].abducted == 0 && !cows[i].safe && cows[i].alive){
            cows[i].pos.x += cows[i].direction.x/movingParameter;
            cows[i].pos.z += cows[i].direction.z/movingParameter;
            if (CowInsideHome(i)){
                cows[i].safe = true;
                g_safeCows++;
            }
        }
    }
}

int Probe_collision(){
    int i;

    glm::vec4 shipW_min = g_cat_model * g_catM_min;
    glm::vec4 shipW_max = g_cat_model * g_catM_max;

    glm::vec4 probeW_min = g_probe_position + g_probe_min;
    glm::vec4 probeW_max = g_probe_position + g_probe_max;

    if(boundingBox_collision(probeW_min, probeW_max, shipW_min, shipW_max)){
            return NUM_COWS;
    }

    if(boundingBox_collision(probeW_min, probeW_max, g_houseW_min, g_houseW_max)){
            return NUM_COWS+2;
    }

    if(g_probe_position.y <= -1.0) return NUM_COWS +1; // Colisão com o chão retorna vacas +1

    glm::vec4 cowW_min;
    glm::vec4 cowW_max;
    for(i=0; i < NUM_COWS; i++){
        if(!cows[i].alive) continue;
        cowW_min = cows[i].model * g_cowM_min;
        cowW_max = cows[i].model * g_cowM_max;
        if(boundingBox_collision(cowW_min, cowW_max, probeW_min, probeW_max)){
            return i;  //colisão com uma vaca retorna seu índice
        }
    }
    return -1;
}

bool CowAbductionCollision(int i){

    int planeY_intersection = 99.0f; // um valor bem alto fora do game space

    glm::vec4 cowW_min = cows[i].model * g_cowM_min;
    glm::vec4 cowW_max = cows[i].model * g_cowM_max;

    return plane_cross_dressY(cowW_min, cowW_max, planeY_intersection);
}

//Função que retorna o índice da primeira vaca dentro do circulo de iluminação da nave. -1 caso nenhuma
int CowUnderRay(){
    int i;

    glm::vec4 cow_to_ship;
    for(i=0; i < NUM_COWS; i++){
        cow_to_ship = cows[i].pos - g_cat_position;
        if(dotproduct(cow_to_ship/norm(cow_to_ship), -g_camera_up_vector) >= cos(ALPHA)){
            return i;
        }
    }

    return -1;
}

bool CatInTheWall(){
    glm::vec4 catW_min = g_cat_model * g_catM_min;
    glm::vec4 catW_max = g_cat_model * g_catM_max;
    return((g_cat_position.x <= -GROUND_SIZE + WALL_DIST) || (g_cat_position.x >= GROUND_SIZE - WALL_DIST)
           || (g_cat_position.z <= -GROUND_SIZE + WALL_DIST) || (g_cat_position.z >= GROUND_SIZE - WALL_DIST)
            || boundingBox_collision(catW_min, catW_max, g_houseW_min, g_houseW_max));
}

// funcao para quando o jogador aperta espao para ordenhar a vaquinha de forma nao invasiva
void MilkCow(){
    int index;

    index = CowUnderRay();

    if (index != -1){
        cows[index].abducted = 1;
        g_under_rainbow_ray = true;
        g_milk_cow = index;
    }
}

// se as vaquinhas sao capturadas pelo raio
void Maintain_MilkSearch(){

    bool colide = CowAbductionCollision(g_milk_cow);

    // se ela ainda está viva, ela vai subir até colidir
    if(cows[g_milk_cow].alive){
        cows[g_milk_cow].pos.y = 100.0f; // numero alto pra sair da box
        cows[g_milk_cow].alive = false;
        cows[g_milk_cow].abducted = 2;
        g_under_rainbow_ray = false;
        g_milk_cow = -1;
        g_milkCows++;

    }

}

bool CowInsideHome(int i){
    glm::vec4 cowW_min = cows[i].model * g_cowM_min;
    glm::vec4 cowW_max = cows[i].model * g_cowM_max;
    glm::vec4 house_min  = g_houseW_min;
    glm::vec4 house_max  = g_houseW_max;
    house_min.x = house_min.x / 0.6;
    house_min.y = house_min.y / 0.6;
    house_min.z = house_min.z / 0.6;
    house_max.x = house_max.x*1.5;
    house_max.y = house_max.y*1.5;
    house_max.z = house_max.z*1.5 ;
    if(boundingBox_collision(cowW_min, cowW_max, house_min, house_max)){
        return true;
    }
    return false;
}
