#include <stdio.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <ctime>
#include <cmath>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>


GLuint player_texture = 0, game_background_texture = 0, menu_background_texture = 0; //texturas dos objetos
float player_posX = 0.0f, player_posY = 0.0f; // posicao do player
int window_width = 500, window_height = 500; // tamanho da tela
int window_posX = 100, window_posY = 100; // posicao da tela

int menuOption = 0;
int cursorPosition = 0;
int indiceAtual = 0;
int pontuacao = 0;
int destino = 0;
int fixo = -1;

bool inMenu = true;
bool isMoving = false;
bool isRoute;

float vel = 0.015f;
float angulo = 0.0f;
float pointSize = 10;

std::vector<int> melhorRota; // vetor para guardar a rota com menor distancia
const int numPontos = 7;
float grafo[numPontos][numPontos] = {0}; // vetor de adjacencia que armazena a distancia entre pontos
float posicoes[numPontos][2] = {  // coordenadas dos pontos do grafo
    {-0.518f, 0.78f},
    {-0.09f, -0.42f},
    {0.33f, 0.34f},
    {0.66f, -0.6f},
    {-0.17f, -0.07f},
    {0.36f, 0.9f},
    {1.05f, 0.76f}
};

std::vector<int> nearestNeighbor(int start) { 
    std::vector<int> caminho;
    std::vector<bool> visitado(numPontos, false);
    int atual = start;

    caminho.push_back(atual);
    visitado[atual] = true;

    for (int i = 1; i < numPontos; i++) {
        int proximo = -1;
        float menorDist = 2.0f;

        for (int j = 0; j < numPontos; j++) {
            if (!visitado[j] && grafo[atual][j] < menorDist) {
                menorDist = grafo[atual][j];
                proximo = j;
            }
        }

        if (proximo != -1) {
            caminho.push_back(proximo);
            visitado[proximo] = true;
            atual = proximo;
        }
    }

    caminho.push_back(start);
    return caminho;
}

void fillGraph(){ // armazena ao grafo as distancia dos pontos
    for(int i = 0; i < numPontos; i++){
        for(int j = 0; j < numPontos; j++){
            float dx = posicoes[j][0] - posicoes[i][0];
            float dy = posicoes[j][1] - posicoes[i][1];
            i == j ? grafo[i][j] = 0 : grafo[i][j] = std::sqrt(dx*dx + dy*dy);
        }
    }
}

int encontrarPontoMaisProximo(float worldX, float worldY) { // 
    int pontoMaisProximo = -1;
    float menorDistancia = 0.5f;

    for (int i = 0; i < numPontos; i++) {
        float dx = posicoes[i][0] - worldX;
        float dy = posicoes[i][1] - worldY;
        float distancia = std::sqrt(dx*dx + dy*dy);
        if (distancia < menorDistancia) {
            menorDistancia = distancia;
            pontoMaisProximo = i;
        }
    }

    return pontoMaisProximo;
}

void load_texture(GLuint* texture, const char* image_location) { 
    if (!*texture) {
        *texture = SOIL_load_OGL_texture(image_location, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        if (!*texture) {
            printf("Erro: '%s'", SOIL_last_result());
        }
    }

    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void city() {

    glPointSize(pointSize);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_POINTS);
    for (int i = 0; i < numPontos; i++) {
        glVertex2f(posicoes[i][0], posicoes[i][1]);
    }
    glEnd();

    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < numPontos; i++) {
        for (int j = i + 1; j < numPontos; j++) {
            glVertex2f(posicoes[i][0], posicoes[i][1]);
            glVertex2f(posicoes[j][0], posicoes[j][1]);
        }
    }
    glEnd();
}

void background(){
    glEnable(GL_TEXTURE_2D);
    inMenu ? glBindTexture(GL_TEXTURE_2D, menu_background_texture) : glBindTexture(GL_TEXTURE_2D, game_background_texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(-1.5, -1.5);
    glTexCoord2f(1.0, 0.0); glVertex2f(1.5, -1.5);
    glTexCoord2f(1.0, 1.0); glVertex2f(1.5, 1.5);
    glTexCoord2f(0.0, 1.0); glVertex2f(-1.5, 1.5);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void route() { // mostra a rota baseado na escolha do player

    if(isMoving){
        if(isRoute) glColor3f(0.0f, 1.0f, 0.0f);
        else glColor3f(1.0f, 0.0f, 0.0f);
        if(fixo == -1) fixo = encontrarPontoMaisProximo(player_posX, player_posY);

        glBegin(GL_LINES);
        glVertex2f(posicoes[fixo][0], posicoes[fixo][1]);
        glVertex2f(posicoes[destino][0], posicoes[destino][1]);
        glEnd();
    }else{
        fixo = -1;
    }
}

void player() { // desenha o player e a textura
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, player_texture);

    glPushMatrix();
    glTranslatef(player_posX, player_posY, 0.0f);
    glRotatef(angulo, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.08f, -0.08f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(0.08f, -0.08f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(0.08f, 0.08f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.08f, 0.08f);
    glEnd();

    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void move_player() { 

    if(isMoving && !melhorRota.empty()) { 

        float destX = posicoes[destino][0];
        float destY = posicoes[destino][1];

        float dx = destX - player_posX;
        float dy = destY - player_posY;
        float distancia = std::sqrt(dx*dx + dy*dy);
        player_texture = 0;
        bool isInvert = false;

        if (dx > 0) { 
            load_texture(&player_texture, "char.png");
            isInvert = false;
        } else if(dx < 0) { 
            load_texture(&player_texture, "char_es.png");
            isInvert = true;
        }

        if(distancia > 0.01f) { 
            player_posX += (dx / distancia) * vel;
            player_posY += (dy / distancia) * vel;

            !isInvert ? angulo = atan2(dy, dx) * (180.0 / M_PI) : angulo = atan2(dy, dx) * (180.0 / M_PI) + 180; 
            if(melhorRota[1] == destino || melhorRota.size() < 2){       
                melhorRota.erase(melhorRota.begin());         
                isRoute = true;
                //fixo = -1; 
                if(posicoes[destino][0] == player_posX && posicoes[destino][1] == player_posY){
                      
                    isMoving = false;
                    fixo = -1;
                }            
            }   
            else if(melhorRota[1] != destino){
                fixo = -1;
                //isRoute = false;
            }
        }                   
    }
}

void init_player() { // inicializacao do player de forma aleatoria
    std::srand(std::time(0));
    int pos_ale = std::rand() % numPontos;

    player_posX = posicoes[pos_ale][0];
    player_posY = posicoes[pos_ale][1];
    indiceAtual = pos_ale; 

    melhorRota = nearestNeighbor(indiceAtual); // inicia a melhor rota baseado na posicao inicial do player

    load_texture(&player_texture, "char.png");

    glutPostRedisplay();
}

void menu() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    background();

    glColor3d(0.0f, 0.0f, 0.0f);
    float startX = -1.45f, startY = 0.2f;
    const char* tutorial[] = {
        "Como Jogar:", 
        "O objetivo do jogo eh analisar os pontos", 
        "exibidos na tela e as distancias entre eles", 
        "para encontrar a melhor rota possível."
    };

    for (int i = 0; i < 4; i++) {
        glRasterPos2f(startX, startY);

        for (const char* c = tutorial[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }
        
        startY -= 0.1f;
    }

    glColor3d(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.6f, 0.5f);
    const char* title = "O Carteiro Viajando";
    for (int i = 0; title[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, title[i]);
    }

    for (int i = 0; i < 2; i++) {
        cursorPosition == i ? glColor3f(0.0f, 1.0f, 0.0f) : glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        glVertex2f(-0.3f, 0.3f - i * 0.4f);
        glVertex2f(0.3f, 0.3f - i * 0.4f);
        glVertex2f(0.3f, 0.1f - i * 0.4f);
        glVertex2f(-0.3f, 0.1f - i * 0.4f);
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(-0.13f + (i - 1) * 0.05, 0.15f - i * 0.4f);
        const char* menuOption[] = {"Iniciar", "Sair"};
        for (int j = 0; menuOption[i][j] != '\0'; j++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, menuOption[i][j]);
        }
    }

    glutSwapBuffers();
}

void display(void) { 
    if (inMenu) {
        menu();
    } else {
        //inGame = true;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        background();
        city();
        route();
        move_player();
        player();

        glColor3f(0.0f, 0.0f, 0.0f); 

        float startX = -1.48f;
        float startY = -0.2f; 

        int point = encontrarPontoMaisProximo(player_posX, player_posY);
        std::string dist = "Distancia entre pontos:";
        glRasterPos2f(startX, startY + 0.1);
        for(char c : dist){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }

        const char* nomesPontos[] = {"AM", "MS", "TO", "RJ", "MT", "PA", "CE"};
        for (int i = 0; i < numPontos; i++) {
            
            for (int j = 0; j < numPontos; j++) {
                if(j != i && i == point){
                    std::string text = std::string(nomesPontos[i]) + " -> " + nomesPontos[j] + ": " + std::to_string(grafo[i][j]);

                    glRasterPos2f(startX, startY);
    
                    for (char c : text) {
                        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, c);
                    }
    
                    startY -= 0.065f; 
                }             
            }
        }


        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(-1.45f, 1.41f);
        std::string text = "Pontuacao: " + std::to_string(pontuacao);
        for(char c: text){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
        std::cout << "Melhor Rota: ";
        for (int ponto : melhorRota) {
            std::cout << ponto << " ";
        }
        std::cout << std::endl;

        glutSwapBuffers();
    }
}

void reshape(int width, int height) { 
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.5, 1.5, -1.5, 1.5);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        float worldX = (x / (float)windowWidth) * 3.0f - 1.5f; 
        float worldY = 1.5f - (y / (float)windowHeight) * 3.0f; 

        destino = encontrarPontoMaisProximo(worldX, worldY);
        int player_pos = encontrarPontoMaisProximo(player_posX, player_posY);
    
        if(melhorRota.size() > 1 && melhorRota[1] == destino) pontuacao += 10;  
        else if(destino != player_pos) pontuacao -= 5;
        isMoving = true;
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
            break;

        case 'w':
            cursorPosition = (cursorPosition - 1 + 2) % 2; break;

        case 's':
            cursorPosition = (cursorPosition + 1 ) % 2; break;

        case 13:
            if (cursorPosition == 0) {
                inMenu = false;
                
                init_player();
                 
            } else if (cursorPosition == 1) {
                exit(0);
            }
            break;
    }
    glutPostRedisplay();
}

void specialKeyboard(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            cursorPosition = (cursorPosition - 1 + 2) % 2;
            break;

        case GLUT_KEY_DOWN:
            cursorPosition = (cursorPosition + 1) % 2;
            break;

        case GLUT_KEY_F11:
            glutFullScreen(); break;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(window_posX, window_posY);
    glutCreateWindow("O Carteiro Viajando");

    fillGraph();

    load_texture(&menu_background_texture, "menu.jpg");
    load_texture(&game_background_texture, "brazil.jpg");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);

    glutMainLoop();

    return 0;
}