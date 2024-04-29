#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
#include <vector>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "helper/stb/stb_image.h"

#define SHADOW_WIDTH 1024

using glm::vec3;

#define CREATE_BUFFER(buffer, vec, usage)                                              \
    if (!vec.empty()) {                                                                \
        glGenBuffers(1, &buffer);                                                      \
        glBindBuffer(GL_ARRAY_BUFFER, buffer);                                         \
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec[0]) * vec.size(), vec.data(), usage); \
        glBindBuffer(GL_ARRAY_BUFFER, 0);                                              \
    }

void Mesh::Create(){
    CREATE_BUFFER(vboPosition, positions, GL_STATIC_DRAW)
    CREATE_BUFFER(vboNormal, normals, GL_STATIC_DRAW)
    CREATE_BUFFER(vboTexcoord, texcoords, GL_STATIC_DRAW)
    CREATE_BUFFER(vboTransformation, instanceTransformations, GL_DYNAMIC_DRAW)
    CREATE_BUFFER(ibo, indices, GL_STATIC_DRAW)

    //
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if (vboNormal) {
        glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if (vboTexcoord) {
        glBindBuffer(GL_ARRAY_BUFFER, vboTexcoord);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    if (vboTransformation) {
        glBindBuffer(GL_ARRAY_BUFFER, vboTransformation);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, 0);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)16);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)32);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)48);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
    }

    glBindVertexArray(0);
}
void Mesh::Draw(GLuint program) const {
    GLUtils::checkForOpenGLError(__FILE__,__LINE__);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glBindVertexArray(vao);
    if (ibo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        if (instanceTransformations.empty())
            glDrawElements(topology, indices.size(), GL_UNSIGNED_INT, 0);
        else
            glDrawElementsInstanced(topology, indices.size(), GL_UNSIGNED_INT, 0, instanceTransformations.size());
    } else {
        if (instanceTransformations.empty())
            glDrawArrays(topology, 0, positions.size());
        else
            glDrawArraysInstanced(topology, 0, positions.size(), instanceTransformations.size());
    }
    
    GLUtils::checkForOpenGLError(__FILE__,__LINE__);
}

void BuildTerrain(Mesh& mesh, float x0, float y0, float w, float h, int segCountX, int segCountY) {
    mesh.normals.resize((segCountX + 1) * (segCountY + 1), glm::vec3(0, 1, 0));
    float x, y;
    float dx = w / segCountX;
    float dy = h / segCountY;
    for (int j = 0; j <= segCountY; ++j) {
        y = y0 + j * dy;
        for (int i = 0; i <= segCountX; ++i) {
            x = x0 + i * dx;
            mesh.positions.emplace_back(x, 0, y);
            mesh.texcoords.emplace_back(float(i) / segCountX, 1. - float(j) / segCountY);
        }
    }
    mesh.positions.shrink_to_fit();
    mesh.texcoords.shrink_to_fit();
    for (int j = 0; j < segCountY; ++j) {
        for (int i = 0; i < segCountX; ++i) {
            mesh.indices.emplace_back(j * (segCountX + 1) + i);
            mesh.indices.emplace_back((j + 1) * (segCountX + 1) + i);
            mesh.indices.emplace_back((j + 1) * (segCountX + 1) + i + 1);
            mesh.indices.emplace_back(j * (segCountX + 1) + i + 1);
        }
    }
    mesh.indices.shrink_to_fit();
    mesh.topology = GL_PATCHES;
}

//===============================
SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}


unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data;
    bool isHdr=stbi_is_hdr(path);
    if (isHdr) {
        data = (unsigned char*)stbi_loadf(path, &width, &height, &nrComponents, 4);
    } else {
        data = stbi_load(path, &width, &height, &nrComponents, 0);
    }

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        if (isHdr) {
            format = GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


// loads 6 individual texture faces of cubmap
// +X (right)
// -X (left)
// +Y (up)
// -Y (down)
// +Z (front) 
// -Z (back)
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void SceneBasic_Uniform::initScene()
{
    compile();

    std::cout << std::endl;

    prog.printActiveUniforms();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    BuildTerrain(terrainMesh, -500, -500, 1000, 1000, 100, 100);

    terrainMesh.Create();
    stbi_set_flip_vertically_on_load(true);

    heightTexture = loadTexture("resources/textures/Rugged Terrain Height Map EXR.hdr");
    colorTexture = loadTexture("resources/textures/Rugged Terrain Diffuse EXR.hdr");

    glGenTextures(1, &normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4096, 4096, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLUtils::checkForOpenGLError(__FILE__,__LINE__);

    prog2.use();
    auto n = 4096 * 4096 / 1024 + 1;
    glBindImageTexture(0, heightTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, normalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute(n, 1, 1);
    glTextureBarrier();
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLUtils::checkForOpenGLError(__FILE__,__LINE__);

    

    camera.Position = glm::vec3(0, 200, 100);

    //create fbo
    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    glCreateFramebuffers(1, &shadowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "failed to create framebuffer, err %x\n", glGetError());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/terrain.vert");
        prog.compileShader("shader/terrain.tcs");
        prog.compileShader("shader/terrain.tes");
        prog.compileShader("shader/terrain.frag");
        prog.link();
        prog.use();
        prog.setUniform("baseColorTex", 0);
        prog.setUniform("texHeight", 1);
        prog.setUniform("texNormal", 2);
        prog.setUniform("texShadow", 3);
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    try {
        prog2.compileShader("shader/height2normal.cs");
        prog2.link();
        prog2.use();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    try {
        shadowProg.compileShader("shader/terrain.vert");
        shadowProg.compileShader("shader/shadow.tcs");
        shadowProg.compileShader("shader/terrain.tes");
        shadowProg.compileShader("shader/shadow.frag");
        shadowProg.link();
        shadowProg.use();
        shadowProg.setUniform("texHeight", 1);
    } catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    if(isStartRotate)
     angle += 0.1;
    //update your angle here
}


bool SceneBasic_Uniform::processKey(int value) {
    float deltaTime  = 0.03;
    switch (value)
    {
    case 0:
        camera.ProcessKeyboard(FORWARD, deltaTime);
        break;
    case 1:
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        break;
    case 2:
        camera.ProcessKeyboard(LEFT, deltaTime);
        break;
    case 3:
        camera.ProcessKeyboard(RIGHT, deltaTime);
        break;
    case 4:
        isStartRotate = true;
        break;
    case 5:
        isStartRotate = false;
        break;
    case 6: 
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         break;
    case 7: 
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         break;
    default:
        break;
    }

    return true;
}
void SceneBasic_Uniform::processMousePos(float xpos, float ypos) {
    static float xpos0 = xpos, ypos0 = ypos;
    camera.ProcessMouseMovement(xpos - xpos0, -ypos + ypos0);
    xpos0 = xpos;
    ypos0 = ypos;
}
void SceneBasic_Uniform::processWheel(float xoffset, float yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void SceneBasic_Uniform::renderScene(GLSLProgram &program, glm::mat4 const &V,glm::mat4 const &P,glm::mat4 const &lightPV){
    program.use();

    program.setUniform("proj", P);
    program.setUniform("view", V);
    program.setUniform("viewportWidth", 800.f);
    program.setUniform("lightPV", lightPV);
    program.setUniform("lightDirection", glm::normalize(lightPosition));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

    terrainMesh.Draw(program.getHandle());
}
void SceneBasic_Uniform::renderShadow(GLSLProgram &program, glm::mat4 const V, glm::mat4 const P){
    glPolygonOffset(3,3);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_WIDTH);

    program.use();
    renderScene(program, V, P, glm::mat4(1));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);
    glPolygonOffset(0,0);
}
void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, 1.f, 10000.0f);

    // rendershadow
    lightPosition = glm::vec3(2000, 500, 0);

    auto lightV = glm::lookAt(lightPosition, {}, {0, 1, 0});
    float a = 500;
    auto lightP = glm::ortho(-a, a, -a, a, 0.f, 10000.f);
    auto lightPV = lightP * lightV;
    renderShadow(shadowProg, lightV, lightP);

    //create the rotation matrix here and update the uniform in the shader
    view = camera.GetViewMatrix();
    view = glm::rotate(view, glm::radians(angle), glm::vec3(0, 1, 0));
    renderScene(prog, view, projection, lightPV);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}
