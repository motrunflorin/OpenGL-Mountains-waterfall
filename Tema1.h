#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <limits>

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/frame_buffer.h"
#include "core/gpu/particle_effect.h"
#include "components/camera.h"
#include "stb/stb_image.h"



#include "Ground.h"


namespace m2
{
    struct LightInf
    {
        glm::vec3 position;
        glm::vec3 color;
        float radius;
    };

    class Tema1 : public gfxc::SimpleScene
    {
     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        unsigned int UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string &neg_x, const std::string &neg_y, const std::string &neg_z);
        void ResetParticlesRain(int xSize, int ySize, int zSize);
        void LoadShadergeom(const std::string& name, const std::string &VS, const std::string& FS, const std::string& GS,  bool hasGeomtery);
        void InitReflectionFramebuffer();


        


        void RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
        void RenderParticles(float deltaTimeSeconds);
        void RenderLights(float curr_time);

        void RenderLake(float deltaTimeSeconds,glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
        void RenderReflection(float deltaTimeSeconds);
        void RenderTerrain(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
         




        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void LoadShader(const std::string &fileName);

     private:
        FrameBuffer *frameBuffer;
        FrameBuffer *lightBuffer;
        FrameBuffer *reflectionFrameBuffer;
        std::vector<LightInf> lights;

        int outputType;
        float rand_info1;
        float rand_info2;
        float rand_info3;

        int location;

        Ground* ground;  

        int cubeMapTextureID;
        float angle;
        
        unsigned int type;

        
      
        gfxc::Camera* reflectionCamera; 





     protected:
        glm::mat4 modelMatrix;
        glm::vec3 generator_position;
        GLenum polygonMode;

        int scene;
        float offset;

        int rows = 40;
        int cols = 40;
        float height = 10.0f;
        float lakeRadius = 10.0f;
        unsigned int nrParticles = 10000;
        glm::vec2 lakeCenter = glm::vec2(0.0f,0.0f);

        
   

    };
}   // namespace m2