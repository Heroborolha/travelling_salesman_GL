#include <stdio.h>
#include <iostream>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>

GLuint player_texture, background_texture = 0;
float player_posX , player_posY = 0.0f;
const int numPontos = 5;
float posicoes[numPontos][2] = {
    {-0.6f, 0.6f},
    {-0.3f, -0.4f},
    {0.5, 0.5},
    {0.8, -0.6},
    {0.0, 0.0}
};

void load_texture(GLuint* texture,const char* image_location){

    //glEnable(GL_TEXTURE_2D);
    if(!*texture){

        *texture = SOIL_load_OGL_texture(image_location, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        if(!*texture){
            printf("Erro: '%s'", SOIL_last_result());
        }
    }
    //glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void city(){

    glPointSize(10);
    glColor3f(0.0f, 0.0f, 0.0f);
    
    glBegin(GL_POINTS);
    for (int i = 0; i < numPontos; i++){
        glVertex2f(posicoes[i][0], posicoes[i][1]);
    }
    glEnd();
}

void background(){

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

void player(){

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, player_texture);
    
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(player_posX, player_posY, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.5f);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

}

void display(void){


    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    background();
    city();
    player();
    
    glutSwapBuffers();

}

void reshape(int width, int height){

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();
    gluOrtho2D(-1.5, 1.5, -1.5, 1.5);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y){

    switch(key){
        
        case 27:
            exit(0);
            break;

        case 'w':
            player_posY += 0.05f; break;
            
        case 's':
            player_posY -= 0.05f; break;

        case 'a':
            player_posX -= 0.05f; break;

        case 'd':
            player_posX += 0.05f; break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jogo");

    load_texture(&background_texture, "brasil.png");
    load_texture(&player_texture, "char.png");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}