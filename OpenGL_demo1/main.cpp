//
//  main.cpp
//  OpenGL_demo1
//
//  Created by baihuajun on 2020/3/27.
//  Copyright © 2020 baihuajun. All rights reserved.
//

/*
 OpenGL 学习第一天
 1、搭建环境；
 2、点击屏幕，将固定位置上的顶点数据以6种不同形态展示
 */
#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/*
 GLMatrixStack 变化管线使用矩阵堆栈
 GLMatrixStack 构造函数允许指定堆栈的最大深度、默认的堆栈深度为64
 该矩阵堆在初始化时已经在堆栈中包含了单位矩阵
 GLMatrixStack:GLMatrixStack(int iStackDepth = 64);
 // 通过调用顶部载入这个单位矩阵
 void GLMatrixStack:LoadIndentiy(void);
 // 在堆栈顶部载入任何矩阵
 void GLMatrixStack:LoadMatrix(const M3DMatrix44f m);
 */

GLShaderManager shaderManager;
//  英  [ˈmeɪtrɪks]   美  [ˈmeɪtrɪks]  矩阵
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLFrame cameraFrame;
GLFrame objectFrame;
GLFrustum viewFrustum; // 投影矩阵

/*
 三角形批次类
 可以传输顶点、光照、纹理、颜色数据等到存储着色器中
 */
// 容器类
GLBatch pointBatch;// 点
GLBatch lineBatch;// 线
GLBatch lineStripBatch;// 条
GLBatch lineLoopBatch;// 环
GLBatch triangleBatch;// 三角形
GLBatch triangleStripBatch;//三角形带
GLBatch triangeFanBatch;//扇形

// 几何变换的管道
GLGeometryTransform transformPipeline;

GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };

// 跟踪效果步骤
int nStep = 0;


void SetupRC()
{
    // 灰色的背景
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);
    //设置变换管线以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    cameraFrame.MoveForward(-20.0f);
    
    /*
     常见函数：
     void GLBatch::Begin(GLenum primitive,GLuint nVerts,GLuint nTextureUnits = 0);
     参数1：表示使用的图元
     参数2：顶点数
     参数3：纹理坐标（可选）
     //复制顶点坐标
     void GLBatch::CopyVertexData3f(GLFloat *vNorms);
     //结束，表示已经完成数据复制工作
     void GLBatch::End(void);
     */
    //定义一些点，三角形形状。
    
    GLfloat vCoast[9] = {
        3,3,0,0,3,0,3,0,0
    };
    
    //用点的形式
    pointBatch.Begin(GL_POINTS, 3);
    pointBatch.CopyVertexData3f(vCoast);
    pointBatch.End();
    
    // 通过线的形式
    lineBatch.Begin(GL_LINES, 3);
    lineBatch.CopyVertexData3f(vCoast);
    lineBatch.End();
    // 通过线段的形式
    lineStripBatch.Begin(GL_LINE_STRIP, 3);
    lineStripBatch.CopyVertexData3f(vCoast);
    lineStripBatch.End();
    // 通过线环的形式
    lineLoopBatch.Begin(GL_LINE_LOOP, 3);
    lineLoopBatch.CopyVertexData3f(vCoast);
    lineLoopBatch.End();
    // 通过三角形创建金字塔
    GLfloat vPyramid[12][3] = {
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f
    };
    // 每三个顶点定义一个三角形
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    
    // 三角形扇形 六边形
    GLfloat vPoints[100][3];
    int nVerts = 0;
    GLfloat r = 3.0f;
    // 原点(x,y,z) = (0,0,0)
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;
    for (GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        // 数组下标自增 每次都是一个顶点
        nVerts ++;
        /*
         弧长 = 半径 * 角度  这里的角度是弧度制
         既然知道了cos值，那么角度 = arccos,求一个反三角函数就行了。
         */
        // x点坐标 cos(angle) * r
        vPoints[nVerts][0] = float(cos(angle) * r);
        // y点坐标 sin(angle) * r
        vPoints[nVerts][1] = float(sin(angle) * r);
        // z点坐标
        vPoints[nVerts][2] = -0.5f;
    }
    // 结束扇形 前面共绘制了7个顶点（包括圆心）
    // 添加终点
    nVerts ++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;
    // GL_TRIANGLE_FAN  以一个圆心为中心呈扇形排列，公用相邻顶点的一组三角形
    triangeFanBatch.Begin(GL_TRIANGLE_FAN, 8);
    triangeFanBatch.CopyVertexData3f(vPoints);
    triangeFanBatch.End();
    
    // 三角形带 一个小环或者圆柱
    int iCounter = 0;
    // 半径
    GLfloat radius = 3.0f;
    GLfloat step = 0.3f;
    // 从0~360,每0.3弧度为步长
    for (GLfloat angle = 0.0f; angle <= (2.0f * M3D_PI); angle += step) {
        // 设 圆的顶点为X，Y
        GLfloat x = sin(angle) * radius;
        GLfloat y = cos(angle) * radius;
        // 绘制2个三角形，他们的X、Y顶点一样，只是Z轴不一样
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5f;
        iCounter ++;
        
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5f;
        iCounter ++;
    }
    // 关闭循环
    printf("三角形带的顶点数：%d\n",iCounter);
    // 结束循环，在循环位置生成2个三角形
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5f;
    iCounter ++;
    
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = 0.5f;
    iCounter ++;
    /*
     GL_TRIANGLE_STRIP:共用一个条带上的顶点的一组三角形
     */
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();
}


void DrawWireFramedBatch(GLBatch *batch){
    // 使用平面着色器画物体
    /*
     GLShaderManager 中的Uniform 值——平面着色器
     GLT_SHADER_FLAT ：平面着色器
     第二个参数：为几何图形变换指定一个4*4的变换矩阵
     第三个参数：颜色
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeline.GetModelViewProjectionMatrix(),vGreen);
    
    batch -> Draw();
    /*
     边框部分
     glEnable(GLenum mode); 用于启用各种功能。功能由参数决定
     注意：glEnable()不能写在glBegin 和 glEnd中间
     GL_POLYGON_OFFSET_LINE ：根据函数glPolygonOffset设置的参数，启用线的深度偏移
     GL_LINE_SMOOTH：启用后过滤线的锯齿
     GL_BLEND：启用颜色混合
     GL_DEPTH_TEST：启用深度测试（根据坐标的远近自动隐藏被遮住的图形）
     
     glDisable(GLenum mode):关闭指定的功能
     */
    // 画边框
    glPolygonOffset(-1.0f, -1.0f);// 偏移深度，在同一位置绘制边线和填充，会产生Z轴冲突，需要偏移
    glEnable(GL_POLYGON_OFFSET_LINE);
    // 画反向锯齿  让边线看着顺滑
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*
     绘制线框几何黑色版
     三种模式：实心、边框、点，可以作用在正面、背面，或者两面
     通过调用glPolygonMode()将多边形正面或者背面设为线框模式，实现线框模式
     */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeline.GetModelViewProjectionMatrix(),vBlack);
    batch -> Draw();
    // 复原原本的模式
    // 将多边形正面或背面设置为全部填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// 召唤场景
void RenderScene()
{
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //压栈
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mCamera);
    
    M3DMatrix44f mObjectFrame;
    //只要使用 GetMatrix 函数就可以获取矩阵堆栈顶部的值，这个函数可以进行2次重载。用来使用GLShaderManager 的使用。或者是获取顶部矩阵的顶点副本数据
    objectFrame.GetMatrix(mObjectFrame);
    
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
     --transformPipeline.GetModelViewProjectionMatrix() 获取的
     GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
    
    switch(nStep) {
        case 0:
            //设置点的大小
            glPointSize(8.0f);
            pointBatch.Draw();
            glPointSize(1.0f);
            break;
        case 1:
            glLineWidth(2.0f);
            lineBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 2:
            glLineWidth(2.0f);
            lineStripBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 3:
            glLineWidth(2.0f);
            lineLoopBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 4:
            DrawWireFramedBatch(&triangleBatch);
            break;
        case 5:
            DrawWireFramedBatch(&triangleStripBatch);
            break;
        case 6:
            DrawWireFramedBatch(&triangeFanBatch);
            break;
    }
    //还原到以前的模型视图矩阵（单位矩阵）
    modelViewMatrix.PopMatrix();
    // 进行缓冲区交换
    glutSwapBuffers();
}


//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y)
{
    
    if(key == GLUT_KEY_UP)
        //围绕一个指定的X,Y,Z轴旋转。
    /*
     RotateWorld 角度转弧度
     */
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    if(key == GLUT_KEY_DOWN)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    if(key == GLUT_KEY_LEFT)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    if(key == GLUT_KEY_RIGHT)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    glutPostRedisplay();
}




//根据空格次数。切换不同的“窗口名称”
void KeyPressFunc(unsigned char key, int x, int y)
{
    if(key == 32)
    {
        nStep++;
        
        if(nStep > 6)
            nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("GL_POINTS");
            break;
        case 1:
            glutSetWindowTitle("GL_LINES");
            break;
        case 2:
            glutSetWindowTitle("GL_LINE_STRIP");
            break;
        case 3:
            glutSetWindowTitle("GL_LINE_LOOP");
            break;
        case 4:
            glutSetWindowTitle("GL_TRIANGLES");
            break;
        case 5:
            glutSetWindowTitle("GL_TRIANGLE_STRIP");
            break;
        case 6:
            glutSetWindowTitle("GL_TRIANGLE_FAN");
            break;
    }
    
    glutPostRedisplay();
}


// 窗口已更改大小，或刚刚创建。无论哪种情况，我们都需要
// 使用窗口维度设置视口和投影矩阵.
void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    //创建投影矩阵，并将它载入投影矩阵堆栈中
    /*
     第一个参数：角度
     2、纵横比
     */
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //调用顶部载入单元矩阵
    modelViewMatrix.LoadIdentity();
}


int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    //申请一个颜色缓存区、深度缓存区、双缓存区、模板缓存区
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    //设置window 的尺寸
    glutInitWindowSize(800, 600);
    //创建window的名称
    glutCreateWindow("GL_POINTS");
    //注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    //点击空格时，调用的函数
    glutKeyboardFunc(KeyPressFunc);
    //特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    //显示函数
    glutDisplayFunc(RenderScene);
    
    //判断一下是否能初始化glew库，确保项目能正常使用OpenGL 框架
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    //绘制
    SetupRC();
    
    //runloop运行循环
    glutMainLoop();
    return 0;
}
