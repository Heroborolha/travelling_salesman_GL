#include <stdio.h>
#include <iostream>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>

GLuint texture = 0;

void display(void){


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0, 0.0); glVertex2f(0.5f, 0.0f);
    glTexCoord2f(1.0, 1.0); glVertex2f(0.5f, 0.5f);
    glTexCoord2f(0.0, 1.0); glVertex2f(0.0f, 0.5f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    
    glutSwapBuffers();

}

void reshape(int width, int height){

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();
    gluOrtho2D(-1.5, 1.5, -1.5, 1.5);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jogo");

    glEnable(GL_TEXTURE_2D);
    if(!texture){

        texture = SOIL_load_OGL_texture("char.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        if(!texture){
            printf("Erro: '%s'", SOIL_last_result());
        }
    }
    glDisable(GL_TEXTURE_2D);

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}