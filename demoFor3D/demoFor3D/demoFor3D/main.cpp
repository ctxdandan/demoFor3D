//
//  main.cpp
//  demoFor3D
//
//  Created by 陈庭轩 on 2018/12/29.
//  Copyright © 2018 陈庭轩. All rights reserved.
//

#include <iostream>
//仅用于测试，代码直接往上拷贝，只修改了个头文件

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include<string>
#include<vector>
#include<iostream>
#include <fstream>
#include <GLUT/GLUT.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

using namespace std;

int if_face=0, if_line=0, if_point=0;//判断三种几何信息是否展示

static GLfloat my_set_material[] = { 1.0, 1.0, 0.1, 0.8};
int if_control_move = 0 ;
static const char *name2  = "/Users/chentingxuan/Desktop/1.stl";//文件路径

//设置光源的位置
static GLfloat light0_position[] = { 4.0, 4.0, 4.0, 0.0 };
static GLfloat light1_position[] = { -3.0, -3.0, -3.0, 0.0 };

typedef struct Vertex {
    //定义三维图形的
    //用于face结构体中
    float x, y, z;
} Vertex;

typedef struct Face {
    //多边形（三角形）面的结构体
    Face(void) : vert_number(0), verts(0) {};
    int vert_number;        //记录顶点的个数
    Vertex **verts;          //这是一个面的所有 顶点数组（含有坐标）
    float normal[3];         //记录点的法向量，分别是x，y，z三个方向
    //注意点的法向量通过顶点的信息计算得到！
    //对于obj模型如果我们已经得到了法线的信息
    //那么就直接拿来用就好！
} Face;

typedef struct myMesh {
    //自定义mesh的结构体
    myMesh(void) : vert_number(0), verts(0), face_number(0), faces(0) {};
    //自定义构造器
    int vert_number;        //总的顶点个数
    Vertex *verts;             //定点数组
    int face_number;                 //面的数目
    Face *faces;
    vector<Vertex>point;
} myMesh;

static int GLUTwindow = 0;
static int GLUTwindow_height = 800;     //设置窗口的大小
static int GLUTwindow_width = 800;
static int GLUTmouse[2] = { 0, 0 };
static int GLUTbutton[3] = { 0, 0, 0 };
static int GLUTarrows[4] = { 0, 0, 0, 0 };
static int GLUTmodifiers = 0;
static int scaling = 0;
static int translating = 0;
static int rotating = 0;
static float scale = 1.0;
static float center[3] = { 0.0, 0.0, 0.0 };
static float rotation[3] = { 0.0, 0.0, 0.0 };
static float translation[3] = { 0.0, 0.0, -4.0 };
static myMesh *mesh = NULL;

int num;
float* verts;
float* vnorms;

void get_normal( Face& face){
    //计算面法线！
    //计算面的法线
    //通过公式计算：
    face.normal[0] = face.normal[1] = face.normal[2] = 0;
    Vertex *v1 = face.verts[face.vert_number-1];
    for (int i = 0; i < face.vert_number; i++) {
        Vertex *v2 = face.verts[i];
        //新建所有的点
        face.normal[0] += (v1->y - v2->y) * (v1->z + v2->z);
        face.normal[1] += (v1->z - v2->z) * (v1->x + v2->x);
        face.normal[2] += (v1->x - v2->x) * (v1->y + v2->y);
        //首先完成叉乘的工作
        v1 = v2;
    }
    //计算归一化法线
    float squared_normal_length = 0.0;
    squared_normal_length += face.normal[0]*face.normal[0];
    squared_normal_length += face.normal[1]*face.normal[1];
    squared_normal_length += face.normal[2]*face.normal[2];
    float normal_length = sqrt(squared_normal_length);
    //得到归一化长度
    if (normal_length > 1.0E-6) {
        face.normal[0] /= normal_length;
        face.normal[1] /= normal_length;
        face.normal[2] /= normal_length;
    }
    //然后完成归一化任务
}

vector<string> split(const string &str,const string &pattern)
{
    //进行字符串的切割
    //const char* convert to char*
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }
    
    delete[] strc;
    
    return resultVec;
}

myMesh *  ReadASCII(const char *cfilename);
myMesh *  ReadSTLFile(const char *cfilename)
{
    //只处理三角形足够了！
    if (cfilename == NULL)
        return 0;
    std::ifstream in(cfilename, std::ifstream::in);
    if (!in)
        return 0;
    std::string headStr;
    getline(in, headStr, ' ');
    in.close();
    if (headStr.empty())
        return 0;
    if (headStr[0] == 's')
        return ReadASCII(cfilename);
    else
        return  0;
}

myMesh * getstlmodel(const char *cfilename)
{
    std::vector<float> coorX;
    std::vector<float> coorY;
    std::vector<float> coorZ;
    
    int max=0;
    bool isbegin=false;
    long size=0;
    int nlines=0;
    int count1=0;
    int count2=0;
    FILE* file=fopen(cfilename,"r");
    fseek(file,0L,SEEK_END);
    size=ftell(file);
    fclose(file);
    file=fopen(cfilename,"r");
    for (int i=0;i<size;i++)
    {
        if(getc(file)=='\n')
        {
            nlines++;
        }
    }
    num=nlines/7;
    rewind(file);
    while (getc(file) != '\n');
    verts=new float[9*num];
    vnorms=new float[9*num];
    for (int i=0;i<num;i++)
    {
        char x[200]="";
        char y[200]="";
        char z[200]="";
        if(3!=fscanf(file,"%*s %*s %80s %80s %80s\n",x,y,z))
        {
            break;
        }
        vnorms[count1]=vnorms[count1+3]=vnorms[count1+6]=atof(x);
        count1++;
        vnorms[count1]=vnorms[count1+3]=vnorms[count1+6]=atof(y);
        count1++;
        vnorms[count1]=vnorms[count1+3]=vnorms[count1+6]=atof(z);
        count1+=7;
        fscanf(file,"%*s %*s");
        if (3!=fscanf(file,"%*s %80s %80s %80s\n",x,y,z))
        {
            break;
        }
        if (isbegin==false)
        {
            isbegin=true;
            max=atof(z);
        }
        coorX.push_back(atof(x));
       // verts[count2]=atof(x);
        count2++;
        
        coorY.push_back(atof(y));
       // verts[count2]=atof(y);
        count2++;
        
        coorZ.push_back(atof(z));
       // verts[count2]=atof(z);
        count2++;
        if (3!=fscanf(file,"%*s %80s %80s %80s\n",x,y,z))
        {
            break;
        }
        
        coorX.push_back(atof(x));
        
       // verts[count2]=atof(x);
        count2++;
        coorY.push_back(atof(y));
        //verts[count2]=atof(y);
        count2++;
        coorZ.push_back(atof(z));
        //verts[count2]=atof(z);
        count2++;
        if (3!=fscanf(file,"%*s %80s %80s %80s\n",x,y,z))
        {
            break;
        }
        
        coorX.push_back(atof(x));
        coorY.push_back(atof(y));
        coorZ.push_back(atof(z));
        //verts[count2]=atof(x);
        count2++;
        //verts[count2]=atof(y);
        count2++;
       //verts[count2]=atof(z);
        count2++;
        fscanf(file,"%*s");
        fscanf(file,"%*s");
    }

    myMesh *meshs=new myMesh();//建立自己的mesh类
    int vert_number=coorX.size();
    int face_number=num;
    meshs->verts=new Vertex[vert_number+1];
    assert(meshs->verts);
    //处理点的信息
    for(int i = 0 ; i  < vert_number;i++)
    {
        Vertex& vert = meshs->verts[meshs->vert_number++];
        vert.x =coorX[i];
        vert.y =coorY[i];
        vert.z =coorZ[i];
    }
    //处理面的信息
    meshs->faces = new Face [face_number];
    assert(meshs->faces);
    int index=0;
    for(int i  = 0; i < face_number;i++)
    {
        Face fa ;
        fa.vert_number = 3;                  //这里直接设置为定3即可！ STL三角片决定的！
        fa.verts =new Vertex* [fa.vert_number];
        for (int j  = 0 ;  j  < 3; j++) {
            fa.verts[j] = &meshs->verts[index++];
        }
        get_normal(fa);
        meshs->faces[meshs->face_number++] = fa;
        //首先分配第一维数组
    }
    return meshs;
   
}

myMesh *  ReadASCII(const char *cfilename)
{
    std::vector<float> coorX;
    std::vector<float> coorY;
    std::vector<float> coorZ;
    int i = 0, j = 0, cnt = 0, pCnt = 4;
    
    char a[100];
    char str[100];
    double x = 0, y = 0, z = 0;
    
    std::ifstream in(cfilename, std::ifstream::in);
    
    if (!in)
        return 0;
    do
    {
        i = 0;
        cnt = 0;
        in.getline(a, 100, '\n');
        while (a[i] != '\0')
        {
            if (!islower((int)a[i]) && !isupper((int)a[i]) && a[i] != ' ')
                break;
            cnt++;
            i++;
        }
        while (a[cnt] != '\0')
        {
            str[j] = a[cnt];
            cnt++;
            j++;
        }
        str[j] = '\0';
        j = 0;
        if (sscanf(str, "%lf%lf%lf", &x, &y, &z) == 3)
        {
            coorX.push_back(x);
            coorY.push_back(y);
            coorZ.push_back(z);
        }
        pCnt++;
    } while (!in.eof());
    
    in.close();
    //向结构体进行转换
    int vert_number=coorX.size();
    int face_number=pCnt;
    
    myMesh *meshs = new myMesh();
    meshs->faces=new Face[face_number];
    assert(meshs->faces);
    int index=0;
    for(int i  = 0; i < face_number;i++)
    {
        Face fa ;
        fa.vert_number = 3;                  //这里直接设置为定3即可！ STL三角片决定的！
        fa.verts =new Vertex* [fa.vert_number];
        for (int j  = 0 ;  j  < 3; j++) {
            fa.verts[j] = &meshs->verts[index++];
        }
        get_normal(fa);
        meshs->faces[meshs->face_number++] = fa;
        //首先分配第一维数组
    }
   
    return meshs;
}

void draw_faces(){
    for (int i = 0; i < mesh->face_number; i++) {
        //注意我们的操作都是
        Face& face = mesh->faces[i];
        glBegin(GL_POLYGON);              //绘制多边形即可！
        //在绘制面的过程中载入我们已经计算好的法线量信息
        glNormal3fv(face.normal);           //在绘制面的时候同时载入法向量信息
        for (int j = 0; j < face.vert_number; j++) {
            Vertex *vert = face.verts[j];
            glVertex3f(vert->x, vert->y, vert->z);
        }
        glEnd();
    }
}

void draw_points(){
    //下面绘制点的信息
    //依次将面的信息点进行遍历
    glColor3f(1.0, 1.0, 0.0);
    glPointSize(2);
    glBegin(GL_POINTS);
    for (int j  = 0 ;  j < mesh->vert_number; j++) {
        glVertex3f(mesh->verts[j].x, mesh->verts[j].y,mesh->verts[j].z);
    }
    glEnd();
}
void draw_lines(){
    double temp_x,temp_y,temp_z;
    for (int i = 0; i < mesh->face_number; i++) {
        Face& face = mesh->faces[i];
        glColor3f(0, 0, 1);
        glBegin(GL_LINES);
        for (int j = 0; j < face.vert_number; j++) {
            Vertex *vert = face.verts[j];
            if(j==0){
                temp_x  = vert->x;
                temp_y  = vert->y;
                temp_z  = vert->z;
                continue;
            }
            glVertex3f(temp_x, temp_y, temp_z);
            glVertex3f(vert->x, vert->y, vert->z);
            temp_x  = vert->x;
            temp_y  = vert->y;
            temp_z  = vert->z;
        }
        glEnd();
    }
}

void GLUTRedraw(void)
{
    //进行空间的重绘
    glLoadIdentity();
    glScalef(scale, scale, scale);
    glTranslatef(translation[0], translation[1], 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat) GLUTwindow_width /(GLfloat) GLUTwindow_height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translation[0], translation[1], translation[2]);
    glScalef(scale, scale, scale);
    //刷新放缩的大小
    glRotatef(rotation[0], 1.0, 0.0, 0.0);
    glRotatef(rotation[1], 0.0, 1.0, 0.0);          //控制不同角度
    glRotatef(rotation[2], 0.0, 0.0, 1.0);
    glTranslatef(-center[0], -center[1], -center[2]);
    //改变旋转中心
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //设置光照
    //载入不同光源的位置
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    //定义材料信息
    //这里可以调整环境颜色和散射颜色数组
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, my_set_material);
    // 下面开始绘制表面
    if(if_face==1)
        draw_faces();
    if(if_line==1)
        draw_lines();
    if(if_point==1)
        draw_points();
    glutSwapBuffers();
}
void GLUTResize(int w, int h)
{
    glViewport(0, 0, w, h);
    //当用户拖拽之后用获取的高度和宽度信息去更新全局变量
    GLUTwindow_width = w;
    GLUTwindow_height = h;
    glutPostRedisplay();
}
void GLUTMotion(int x, int y)
{
    //通过鼠标事件控制所调用的活动类型！
    //真正的鼠标控制事件的执行者
    y = GLUTwindow_height - y;
    if (rotating) {
        //控制旋转的信号
        rotation[0] += -0.5 * (y - GLUTmouse[1]);
        rotation[2] += 0.5 * (x - GLUTmouse[0]);
        //rotation[1] += 0.5 * (x - GLUTmouse[0]);
        //这样可以面向你进行旋转
        //在重绘的时候就可以了
    }
    else if (scaling) {
        // 控制缩放信号
        //GLUTmouse存储了之前记录的点信息
        scale *= exp(2.0 * (float)( (x- GLUTmouse[0])) / (float) GLUTwindow_width);
        //如果想调成按照y方向控制可以:
        // scale *= exp(2.0 * (float)( (y- GLUTmouse[1])) / (float) GLUTwindow_width);
    }
    else if (translating) {
        // 控制平移信号
        translation[0] += 2.0 * (float) (x - GLUTmouse[0]) / (float) GLUTwindow_width;
        translation[1] += 2.0 * (float) (y - GLUTmouse[1]) / (float) GLUTwindow_height;
    }
    //我们在拖拽旋转的过程中需要设置定点中心
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;
    //刷新定点
}

void GLUTMouse(int button, int state, int x, int y)
{   cout << "控制键的情况: " << if_control_move << endl;
    y = GLUTwindow_height - y;
    int  kind = 0;
    if(button == GLUT_LEFT_BUTTON&&if_control_move){
        translating = 1;
        cout << "double" << endl;
        kind=2;
        rotating=0;
        scaling = 0;
    }else{if(button == GLUT_LEFT_BUTTON&&button != GLUT_RIGHT_BUTTON){
        kind = 0;
        rotating  =1;
        scaling = 0;
        translating = 0;
    }else if(button != GLUT_LEFT_BUTTON&&button == GLUT_RIGHT_BUTTON){
        kind = 1;
        scaling = 1;
        rotating =0;
        translating = 0;
    }}
    if (rotating || scaling || translating) glutIdleFunc(GLUTRedraw);
    else glutIdleFunc(0);
    cout << "此时的B 选择的操作："<< kind<< endl;
    GLUTbutton[kind] = (state == GLUT_DOWN) ? 1 : 0;
    GLUTmodifiers = glutGetModifiers();
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;
}

void GLUTSpecial(int key, int x, int y)
{
    y = GLUTwindow_height - y;
    //记录当下鼠标点击的位置
    GLUTmouse[0] = x;
    GLUTmouse[1] = y;
    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();
    glutPostRedisplay();
}

void GLUTKeyboard(unsigned char key, int x, int y)
{
    // Process keyboard button event
    //处理鼠标事件
    switch (key) {
        case '1':
            cout << "打开/关闭点的信息" << endl;
            if_point = 1 - if_point;
            break;
        case '2':
            cout << "打开/关闭线的信息" << endl;
            if_line  = 1 - if_line;
            break;
        case '3':
            cout << "打开/关闭面的信息" << endl;
            if_face  =  1 - if_face;
            break;
            //调节表面的材质的信息
        case 'z':
            if_control_move = 1 - if_control_move;
            break;
        case 'q':
            my_set_material[0] += 0.1;
            break;
        case 'w':
            my_set_material[1] += 0.1;
            break;
        case 'e':
            my_set_material[2] += 0.1;
            break;
        case 'r':
            my_set_material[3] += 0.1;
            break;
        case 'a':
            my_set_material[0] -= 0.1;
            break;
        case 's':
            my_set_material[1] -= 0.1;
            break;
        case 'd':
            my_set_material[2] -= 0.1;
            break;
        case 'f':
            my_set_material[3] -= 0.1;
            break;
        case '4':
            for(int i = 0 ; i  < 4 ;i ++){
                light0_position [i]+=0.01;
            }
            break;
        case '5':
            for(int i = 0 ; i  < 4 ;i ++){
                light0_position [i]-=0.01;
                if(light0_position[i]<=0)
                    light0_position[i] = 0;
            }
            break;
        case '6':
            for(int i = 0 ; i  < 4 ;i ++){
                cout <<light1_position [i] << " ~~ "<< endl;
                light1_position [i]+=0.01;
                
            }
            break;
        case '7':
            for(int i = 0 ; i  < 4 ;i ++){
                light1_position [i]-=0.01;
                cout <<light1_position [i] << " ~~ "<< endl;
                
                if(light1_position[i]<=0)
                    light1_position[i] = 0;
            }
            break;
            
    }
    // Remember mouse position
    GLUTmouse[0] = x;
    GLUTmouse[1] = GLUTwindow_height - y;
    // Remember modifiers
    GLUTmodifiers = glutGetModifiers();
}

void GLUTInit(int *argc, char **argv)
{
    // Open window
    glutInit(argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);// | GLUT_STENCIL
    glutCreateWindow("应力云图");
    
    glutReshapeFunc(GLUTResize);//设置大小.
    
    glClearColor(0.3f, 0.3f, 0.3f, 0.3f);
    
    //设置重绘信息
    glutDisplayFunc(GLUTRedraw);
    //注册键盘事件
    glutKeyboardFunc(GLUTKeyboard);
    glutSpecialFunc(GLUTSpecial);//
    glutMouseFunc(GLUTMouse);
    glutMotionFunc(GLUTMotion);
    glutIdleFunc(0);
    
    // 设置光照信息
    static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    static GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    //设置满散射
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glEnable(GL_LIGHT0);
    static GLfloat light1_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

void GLUTMainLoop(void)
{
    float bbox[2][3] = { { 1.0E30F, 1.0E30F, 1.0E30F }, { -1.0E30F, -1.0E30F, -1.0E30F } };
    for (int i = 0; i < mesh->vert_number; i++) {
        Vertex& vert = mesh->verts[i];
        if (vert.x < bbox[0][0]) bbox[0][0] = vert.x;
        else if (vert.x > bbox[1][0]) bbox[1][0] = vert.x;
        if (vert.y < bbox[0][1]) bbox[0][1] = vert.y;
        else if (vert.y > bbox[1][1]) bbox[1][1] = vert.y;
        if (vert.z < bbox[0][2]) bbox[0][2] = vert.z;
        else if (vert.z > bbox[1][2]) bbox[1][2] = vert.z;
    }
    // Setup initial viewing scale
    float dx = bbox[1][0] - bbox[0][0];
    float dy = bbox[1][1] - bbox[0][1];
    float dz = bbox[1][2] - bbox[0][2];
    scale = 2.0 / sqrt(dx*dx + dy*dy + dz*dz);
    // Setup initial viewing center
    center[0] = 0.5 * (bbox[1][0] + bbox[0][0]);
    center[1] = 0.5 * (bbox[1][1] + bbox[0][1]);
    center[2] = 0.5 * (bbox[1][2] + bbox[0][2]);
    glutMainLoop();
    //计算并更新视角边框以及中心
}



void myDisplay(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glRectf(-0.5f, -0.5f, 0.5f, 0.5f);
    glFlush();
}
int main(int argc, char **argv)
{
    GLUTInit(&argc, argv);
    mesh=getstlmodel(name2);
    //mesh = ReadSTLFile(name2);
    //    mesh = ReadOffFile(filename);
    GLUTMainLoop();
    return 0;
}

