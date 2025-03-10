#include <stdio.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <climits>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>

GLuint player_texture = 0, background_texture = 0;
float player_posX = 0.0f, player_posY = 0.0f;
int window_width = 500, window_height = 500;
int window_posX = 100, window_posY = 100;

int menuOption = 0;
int cursorPosition = 0;
int indiceAtual = 0;
int pontuacao = 0;

bool inMenu = true;
bool isMoving = false;
float vel = 0.02f;
float angulo = 0.0f;

std::vector<int> melhorRota;
const int numPontos = 6;
float grafo[numPontos][numPontos] = {0};
float posicoes[numPontos][2] = {
    {-0.45f, 0.83f},
    {0.0f, -0.37f},
    {0.48, 0.3},
    {0.84, -0.68},
    {-0.22, -0.03},
    {0.48, 0.95}
};

std::vector<int> nearestNeighbor(int start) {
    std::vector<int> caminho;
    std::vector<bool> visitado(numPontos, false);
    int atual = start;

    caminho.push_back(atual);
    visitado[atual] = true;

    for (int i = 1; i < numPontos; i++) {
        int proximo = -1;
        int menorDist = INT_MAX;

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

    // Voltar ao ponto inicial
    caminho.push_back(start);
    return caminho;
}

std::vector<int> calcularCaminho(int inicio, int destino) {
    std::vector<int> caminho;
    std::vector<float> distancias(numPontos, FLT_MAX);
    std::vector<int> predecessores(numPontos, -1);
    std::vector<bool> visitado(numPontos, false);

    distancias[inicio] = 0;

    for (int i = 0; i < numPontos - 1; i++) {
        int u = -1;
        float menorDistancia = FLT_MAX;

        // Encontra o ponto não visitado com a menor distância
        for (int j = 0; j < numPontos; j++) {
            if (!visitado[j] && distancias[j] < menorDistancia) {
                menorDistancia = distancias[j];
                u = j;
            }
        }

        if (u == -1) break; // Todos os pontos foram visitados

        visitado[u] = true;

        // Atualiza as distâncias dos pontos vizinhos
        for (int v = 0; v < numPontos; v++) {
            if (!visitado[v] && grafo[u][v] > 0 && distancias[u] + grafo[u][v] < distancias[v]) {
                distancias[v] = distancias[u] + grafo[u][v];
                predecessores[v] = u;
            }
        }
    }

    // Reconstrói o caminho
    int atual = destino;
    while (atual != -1) {
        caminho.insert(caminho.begin(), atual);
        atual = predecessores[atual];
    }

    return caminho;
}

void fillGraph(){
    for(int i = 0; i < numPontos; i++){
        for(int j = 0; j < numPontos; j++){
            float dx = posicoes[j][0] - posicoes[i][0];
            float dy = posicoes[j][1] - posicoes[i][1];
            i == j ? grafo[i][j] = 0 : grafo[i][j] = std::sqrt(dx*dx + dy*dy);
        }
    }
}

int encontrarPontoMaisProximo(float worldX, float worldY) {
    int pontoMaisProximo = -1;
    float menorDistancia = FLT_MAX;

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
    glPointSize(10);
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

void background() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, background_texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(-1.5, -1.5);
    glTexCoord2f(1.0, 0.0); glVertex2f(1.5, -1.5);
    glTexCoord2f(1.0, 1.0); glVertex2f(1.5, 1.5);
    glTexCoord2f(0.0, 1.0); glVertex2f(-1.5, 1.5);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void rote() {
    glColor3f(0.0, 1.0, 0.0); // Verde para a melhor rota
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < melhorRota.size(); i++) {
        glVertex2f(posicoes[melhorRota[i]][0], posicoes[melhorRota[i]][1]);
    }
    glEnd();
}

void player() {
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, player_texture);

    glPushMatrix();
    glTranslatef(player_posX, player_posY, 0.0f);
    glRotatef(angulo, 0.0f, 0.0f, 1.0f); // Aplica a rotação
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
    //int custoTotal = 0;
    if (isMoving && !melhorRota.empty()) {
        int proximo = melhorRota[0]; 

        float destX = posicoes[proximo][0];
        float destY = posicoes[proximo][1];

        float dx = destX - player_posX;
        float dy = destY - player_posY;
        float distancia = std::sqrt(dx * dx + dy * dy);

        if (distancia > 0.01f) {
            player_posX += (dx / distancia) * vel;
            player_posY += (dy / distancia) * vel;

        
            angulo = atan2(dy, dx) * (180.0 / M_PI);
        } else {
        
            melhorRota.erase(melhorRota.begin()); 
            indiceAtual = proximo;
            if(melhorRota.empty()){
                isMoving = false;        
            }
        }    
    }
}

void init_player() {
    std::srand(std::time(0));
    int pos_ale = std::rand() % numPontos;

    player_posX = posicoes[pos_ale][0];
    player_posY = posicoes[pos_ale][1];
    indiceAtual = pos_ale; 

    melhorRota = nearestNeighbor(indiceAtual);
    

    glutPostRedisplay();
}

void menu() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glColor3d(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.2f, 0.5f);
    const char* title = "O Jogo";
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        background();
        city();
        rote();
        player();

        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(-1.43f, 1.3f);
        std::string text = "Pontuacao: " + std::to_string(pontuacao);
        for(char c: text){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        move_player();
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

        int destino = encontrarPontoMaisProximo(worldX, worldY);

        if (destino != -1 && destino != indiceAtual) {
            // Se o destino é o próximo na melhor rota, aumenta pontuação
            if(!melhorRota.empty() && (melhorRota[1] == destino || melhorRota[melhorRota.size() - 2] == destino)){
                pontuacao += 10;
                melhorRota.erase(melhorRota.begin());
            }

            // Atualiza a posição e inicia movimento
            melhorRota.clear();
            melhorRota.push_back(destino);
            isMoving = true;
        }
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
                std::cout << "Iniciar" << std::endl;
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
    glutCreateWindow("Jogo");

    fillGraph();

    
    load_texture(&background_texture, "brasil.png");
    load_texture(&player_texture, "char.png");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);

    glutMainLoop();

    return 0;
}