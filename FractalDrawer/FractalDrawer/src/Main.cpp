#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include"Vector.h"
#include"ShaderLoader.h"

#include <iostream>
#include<vector>

#include"Switch.h"

//#define FULLSCREEN

#ifdef FULLSCREEN
const uint32_t windowWidth = 1920, windowHeight = 1080;
#else
const uint32_t windowWidth = 1440, windowHeight = 900;
#endif // FULLSCREEN

const vec2 windowSize{ windowWidth, windowHeight };

static GLuint canvasTextures[2];
static Switch<GLuint> canvasTexture{ canvasTextures[0], canvasTextures[1] };


static vec2 mousePos(0, 0), pmousePos(0, 0);

static const size_t fractalPivotCount = 3;
static const size_t allPivotCount = fractalPivotCount + 1;
static bool isPivot = false;
static uint32_t pivotIndex;
static vec2 pivotPoints[allPivotCount * 2]; // viewPortPivot_ll, viewPortPivot_ur, pivot1_ll, pivot1_ur, ..

static struct PaintSample {
    enum class PoistionInChain : unsigned int {
        START = 0,
        INTERMEDIATE = 1,
        END = 2
    };

    PoistionInChain positionInChain;
    alignas(4) uint32_t isDelete;
    alignas(8) vec2 coord;

    constexpr PaintSample(
        const PoistionInChain positionInChain_,
        const bool isDelete_,
        const vec2 coord_
    ) :
        positionInChain{ positionInChain_ },
        isDelete{ isDelete_ },
        coord{ coord_ }
    {}
};

bool isDrawing, isDelete;
static std::vector<PaintSample> paintSamples{}; /*
    temporal storage of key samples defining what to draw, sent to GPU and then cleared before every draw call.
*/

static size_t bufferCapacity = 30;

static void pivotUpdate(bool isPress) {
    if (isPress) {
        size_t index = 0;
        float shortestDist = pivotPoints[0].sqDistance(mousePos);

        for (size_t i = 1; i < allPivotCount * 2; i++) {
            float dist = pivotPoints[i].sqDistance(mousePos);
            if (dist < shortestDist) {
                shortestDist = dist;
                index = i;
            }
        }

        float threshold = 0.001 * windowSize.sqLength();
        if (shortestDist < threshold) {
            isPivot = true;
            pivotIndex = index;
            pivotPoints[pivotIndex] = mousePos;
        }
    }
    else {
        isPivot = false;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_GRAVE_ACCENT) pivotUpdate(action == GLFW_PRESS || action == GLFW_REPEAT);
}

static void cursor_position_callback(GLFWwindow* window, double mousex, double mousey) {
	pmousePos = mousePos;
    mousePos = vec2(mousex, mousey);

    if (isDrawing) {
        paintSamples.push_back(
            PaintSample{
                PaintSample::PoistionInChain::INTERMEDIATE,
                isDelete,
                mousePos
            }
        );

    }
    else if (isPivot) {
        pivotPoints[pivotIndex] = mousePos;
    }
}

static void mouse_button_callback(GLFWwindow* window, const int button, const int action, const int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
        isDelete = button == GLFW_MOUSE_BUTTON_RIGHT;
        PaintSample::PoistionInChain p;
        if (action == GLFW_PRESS) {
            p = PaintSample::PoistionInChain::START;
            isDrawing = true;
        }
        else if (action == GLFW_RELEASE) {
            p = PaintSample::PoistionInChain::END;
            isDrawing = false;
        }
        else exit(-1);

        paintSamples.push_back(
            PaintSample{
                p,
                isDelete,
                mousePos
            }
        );
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        pivotUpdate(action == GLFW_PRESS);
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

}

int main(void) {
    for (size_t i = 0; i < allPivotCount; i++) {
        pivotPoints[i * 2 + 0] = vec2(0, 0);
        pivotPoints[i * 2 + 1] = windowSize;
    }

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    GLFWmonitor* monitor;
#ifdef FULLSCREEN
    monitor = glfwGetPrimaryMonitor();
#else
    monitor = NULL;
#endif // !FULLSCREEN

    window = glfwCreateWindow(windowWidth, windowHeight, "Template", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwWindowHint(GLFW_SAMPLES, 8);
    glEnable(GL_MULTISAMPLE);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        glfwTerminate();
        return -1;
    }
    fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));

    //callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    //load shaders
    GLuint programId = glCreateProgram();
    ShaderLoader sl{};
    sl.addScreenSizeTriangleStripVertexShader();
    sl.addShaderFromProjectFileName("shaders/fs.shader", GL_FRAGMENT_SHADER, "Main shader");

    sl.attachShaders(programId);

    glLinkProgram(programId);
    glValidateProgram(programId);

    sl.deleteShaders();

    GLuint drawEffect = glCreateProgram();
    {
        ShaderLoader desl{};
        desl.addScreenSizeTriangleStripVertexShader();
        //desl.addScreenCoordFragmentShader();
        desl.addShaderFromProjectFileName("shaders/drawEffect.shader", GL_FRAGMENT_SHADER, "Draw effect shader");

        desl.attachShaders(drawEffect);

        glLinkProgram(drawEffect);
        glValidateProgram(drawEffect);

        desl.deleteShaders();
    }

    GLuint drawToScreen = glCreateProgram();
    {
        ShaderLoader desl{};
        desl.addScreenSizeTriangleStripVertexShader();
        //desl.addScreenCoordFragmentShader();
        desl.addShaderFromProjectFileName("shaders/drawToScreen.shader", GL_FRAGMENT_SHADER, "Draw to screen shader");

        desl.attachShaders(drawToScreen);

        glLinkProgram(drawToScreen);
        glValidateProgram(drawToScreen);

        desl.deleteShaders();
    }

    //framebuffer
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(2, &canvasTextures[0]);
    for (size_t i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, canvasTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //initial data
    glUseProgram(programId);
    glUniform2i(glGetUniformLocation(programId, "windowSize"), GLint(windowWidth), GLint(windowHeight));

    glUseProgram(drawEffect);
    glUniform2i(glGetUniformLocation(drawEffect, "windowSize"), GLint(windowWidth), GLint(windowHeight));

    glUseProgram(drawToScreen);
    glUniform2i(glGetUniformLocation(drawToScreen, "windowSize"), GLint(windowWidth), GLint(windowHeight));

    
    glUseProgram(programId);
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) + (sizeof PaintSample) * bufferCapacity, NULL, GL_DYNAMIC_DRAW); /*
       i hope it will be initialized with zeros                                                                                                              
    */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint pivotsHandle;
    glGenBuffers(1, &pivotsHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pivotsHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec2) * allPivotCount * 2, &pivotPoints[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pivotsHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //glUniform1i(glGetUniformLocation(programId, "width"), GLint(windowWidth));

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
        if (paintSamples.size() == 0 && isDrawing) {
            paintSamples.push_back(
                PaintSample{
                    PaintSample::PoistionInChain::INTERMEDIATE,
                    isDelete,
                    mousePos
                }
            );
        }

        const auto samplesSize = paintSamples.size();

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        if (bufferCapacity < samplesSize) {
            bufferCapacity = samplesSize * 2;
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned long long) + (sizeof PaintSample) * bufferCapacity, NULL, GL_DYNAMIC_DRAW);
        }

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned long long), &samplesSize);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned long long), (sizeof PaintSample) * samplesSize, paintSamples.data());
        paintSamples.clear();
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pivotsHandle);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec2) * allPivotCount * 2, &pivotPoints[0]);
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pivotsHandle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        //glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(programId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvasTexture.get(), 0);
        glBindTexture(GL_TEXTURE_2D, canvasTexture.getOther());
        glUniform1i(glGetUniformLocation(programId, "canvas_prev"), GLint(0));
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(drawEffect);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvasTexture.getOther(), 0);
        glBindTexture(GL_TEXTURE_2D, canvasTexture.get());
        glUniform1i(glGetUniformLocation(drawEffect, "canvas"), GLint(0));
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(drawToScreen);
        glBindTexture(GL_TEXTURE_2D, canvasTexture.get());
        glUniform1i(glGetUniformLocation(drawToScreen, "canvas"), GLint(0));
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
        glBindTexture(GL_TEXTURE_2D, 0);

        //canvasTexture.doSwitch();

        glfwSwapBuffers(window);

        glfwPollEvents();

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << err << std::endl;
        }

        //std::cout << p1.x << ' ' << p1.y << '&' << p2.x << ' ' << p2.y << std::endl;
 
        int v = 0;
        for (int i = 0; i < 100000; i++) {
            v += i - 1;
            if (uint32_t(std::rand()) == 293089) std::cout << "sic!";
        }
    }
 
    glfwTerminate();
    return 0;
}
