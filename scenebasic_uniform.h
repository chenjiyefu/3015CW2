#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/camera.h"
#include "helper/objmesh.h"

struct Mesh{
public:
    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::mat4> instanceTransformations;

    // Eigen::Vector3
    std::vector<unsigned int> indices;

    // Transform transform;
    GLenum topology;

    GLuint vao;
    GLuint vboPosition{0};
    GLuint vboNormal{0};
    GLuint vboTexcoord{0};
    GLuint vboTransformation{0};
    GLuint ibo;

    void Create();
    void Draw(GLuint program) const;
};


class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLSLProgram prog;
    GLSLProgram prog2;

    float angle;
    bool isStartRotate;
    bool wireframe = false;
    Camera camera;

    GLuint heightTexture;
    GLuint colorTexture;
    GLuint normalTexture;

    Mesh terrainMesh;

    void compile();

    void renderScene(GLSLProgram &program, glm::mat4 const &V,glm::mat4 const &P,glm::mat4 const &lightPV);

    void renderShadow(GLSLProgram &program, glm::mat4 const viewMatrix, glm::mat4 const proj);

    glm::vec3 lightPosition;

    GLSLProgram shadowProg;
    GLuint shadowFbo;
    GLuint shadowDepthTex;

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    bool processKey(int value);
    void processMousePos(float xpos, float ypos) override;
    void processWheel(float xoffset, float yoffset) override;
};

#endif // SCENEBASIC_UNIFORM_H
