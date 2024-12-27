#include "lab_m2/Tema1/Tema1.h"
#include "lab_m2/Tema1/Ground.h"


#include <vector>
#include <iostream>
#include "stb/stb_image.h"


using namespace std;
using namespace m2;

struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float delay;
    float initialDelay;
    float lifetime;
    float initialLifetime;

    glm::vec3 bezierStart;
    glm::vec3 bezierControl1;
    glm::vec3 bezierControl2;
    glm::vec3 bezierEnd;

    Particle() {}

    Particle(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed,
        float delay = 0, float lifetime = 0)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;

        this->delay = delay;
        initialDelay = delay;

        this->lifetime = lifetime;
        initialLifetime = lifetime;
    }
};


ParticleEffect<Particle> *particleEffect;



//Generates a random value between 0 and 1.
inline float Rand01()
{
    return rand() / static_cast<float>(RAND_MAX);
}

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{


    angle = 0;

    type = 0;

}


Tema1::~Tema1()
{
     if (ground)
    {
        delete ground;
    }

    

}


void Tema1::Init()
{
    outputType = 0;

    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 2, 3.5), glm::quat(glm::vec3(-20 * TO_RADIANS, 0, 0)));
    camera->Update();
    
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "particle2.png");

    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "ground.jpg");
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "rain.png");
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "water.jpg");

    std::string texturePath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cubemap_night");
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");
    
    {
        Mesh* mesh = new Mesh("lake");
        
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");

        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;

    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    //  Load terrain mesh
    {   
        Mesh* mesh = new Mesh("terrain");
        Ground* ground = new Ground(rows, cols, height, lakeRadius);
        mesh = ground->GetMesh();
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        // load cubemap
        Mesh* mesh = new Mesh("skybox");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath, "pos_x.png"),
        PATH_JOIN(texturePath, "pos_y.png"),
        PATH_JOIN(texturePath, "pos_z.png"),
        PATH_JOIN(texturePath, "neg_x.png"),
        PATH_JOIN(texturePath, "neg_y.png"),
        PATH_JOIN(texturePath, "neg_z.png")
    );
    
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "cubemap_night");

    LoadShader("Composition");
    LoadShader("LightPass");
    LoadShader("Ground");
    LoadShader("Normal");
    LoadShadergeom("Rain", "Particle_rain", "Particle_simple", "Particle", true);

    LoadShader("Lake");

    



    ResetParticlesRain(40,20,40);

    generator_position = glm::vec3(15, 20, 15);
    scene = 0;
    offset = 0.5;

   
    auto resolution = window->GetResolution();

    frameBuffer = new FrameBuffer();
    frameBuffer->Generate(resolution.x, resolution.y, 3);

    lightBuffer = new FrameBuffer();
    lightBuffer->Generate(resolution.x, resolution.y, 1, false);

    // Setup reflection camera
    reflectionFrameBuffer = new FrameBuffer();
    reflectionFrameBuffer->Generate(resolution.x, resolution.y, 3);

    reflectionCamera = new gfxc::Camera();

    //--------------------------------------------------------------------
    // Initializarea luminilor
    {
        int cols = 40;
        int rows = 40;

        float gridWidth = static_cast<float>(cols - 1);  
        float gridHeight = static_cast<float>(rows - 1);

        float dx = gridWidth / (cols - 1);  
        float dz = gridHeight / (rows - 1);

        glm::vec2 center(0.0f, 0.0f);
        float lakeRadius = 10.0f;  
        float height = 17.0f;  

        for (int i = 0; i < rows; i+=5) {
            for (int j = 0; j < cols; j+=5) {
                LightInf lightInf;

               
                rand_info1 = Rand01();
                rand_info2 = Rand01();
                rand_info3 = Rand01();

            
             
                lightInf.position.x =  -gridWidth / 2 + j * dx;
                lightInf.position.z = -gridHeight / 2 + i * dz;
                
                float d = glm::distance(glm::vec2(lightInf.position.x, lightInf.position.z), center) / lakeRadius;

                lightInf.position.y =  Ground::ConcaveConvexDisplacement(lightInf.position.x, lightInf.position.z, center, lakeRadius, height) + 2.0f;

                lightInf.color = glm::vec3(rand_info1, rand_info2, rand_info3);

                lightInf.radius = 3 + Rand01();

                lights.push_back(lightInf);
            }
        }
    }

}


void Tema1::FrameStart() {

}




void Tema1::RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
    glDepthFunc(GL_LEQUAL);

    Shader* skyboxShader = shaders["Normal"];
    skyboxShader->Use();

    viewMatrix = glm::mat4(glm::mat3(viewMatrix)); 
    glUniformMatrix4fv(skyboxShader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(skyboxShader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);

    meshes["skybox"]->Render();
    glDepthFunc(GL_LESS);
}

void Tema1::RenderParticles(float deltaTimeSeconds) {
    glLineWidth(3);

    auto shader = shaders["Rain"];
    shader->Use();

    TextureManager::GetTexture("rain.png")->BindToTextureUnit(GL_TEXTURE0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    particleEffect->Render(GetSceneCamera(), shader);

    glDepthMask(GL_FALSE);

    glUniform1f(glGetUniformLocation(shader->program, "deltaTime"), deltaTimeSeconds);
    glUniform1f(glGetUniformLocation(shader->program, "offset"), offset);
    glUniform3fv(glGetUniformLocation(shader->program, "generator_position"), 1, glm::value_ptr(generator_position));

    glm::vec3 curveStart = glm::vec3(rows, height * 0.8f, cols);
    glm::vec3 curveControl1 = glm::vec3(rows / 2.0f, height * 0.8f, cols / 2.0f);
    glm::vec3 curveControl2 = glm::vec3(rows / 4.0f, height * 0.8f, cols / 4.0f);
    glm::vec3 curveEnd = glm::vec3(0.0f, -5.0f, 0.0f);

    glUniform1f(glGetUniformLocation(shader->program, "h_max"), height);
    glUniform2fv(glGetUniformLocation(shader->program, "lakeCenter"), 1, glm::value_ptr(lakeCenter));
    glUniform1f(glGetUniformLocation(shader->program, "lakeRadius"), lakeRadius);

    glUniform3fv(glGetUniformLocation(shader->program, "curveStart"), 1, glm::value_ptr(curveStart));
    glUniform3fv(glGetUniformLocation(shader->program, "curveControl1"), 1, glm::value_ptr(curveControl1));
    glUniform3fv(glGetUniformLocation(shader->program, "curveControl2"), 1, glm::value_ptr(curveControl2));
    glUniform3fv(glGetUniformLocation(shader->program, "curveEnd"), 1, glm::value_ptr(curveEnd));
}

void Tema1::RenderTerrain(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    auto shader = shaders["Ground"];
    shader->Use();

    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    TextureManager::GetTexture("ground.jpg")->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["terrain"], shader, glm::vec3(0, 0, 0), glm::vec3(1.0f));
}


void Tema1::RenderLights(float curr_time) {
    auto shader = shaders["Ground"];
    shader->Use();

    for (auto& light : lights) {
        bool willMove = Rand01() < 0.5f;
        if (willMove) {
            float speed = 0.5f;
            float amplitude = 0.05f;

            int movementAxis = rand() % 3;

            switch (movementAxis) {
                case 0:
                    light.position.x += amplitude * sin(speed * curr_time);
                    break;
                case 1:
                    light.position.y += amplitude * sin(speed * curr_time);
                    break;
                case 2:
                    light.position.z += amplitude * sin(speed * curr_time);
                    break;
            }
        }

        auto model = glm::translate(glm::mat4(1), light.position);
        model = glm::scale(model, glm::vec3(0.2f));
        light.radius = 3 + 1.0f * sin(curr_time * glm::pi<float>());
    }
}



void Tema1::RenderReflection(float deltaTimeSeconds) {

    reflectionFrameBuffer->Bind();

    glViewport(0, 0, 1024, 1024); 

    ClearScreen();                

    auto camera = GetSceneCamera();
    glm::vec3 cameraPos = camera->m_transform->GetWorldPosition();
    glm::quat cameraRot = camera->m_transform->GetWorldRotation();

    float lakeY = 5.0f;  
    glm::vec3 reflectedPos = glm::vec3(cameraPos.x, 2 * lakeY - cameraPos.y, cameraPos.z);
    glm::vec3 rotAngles = glm::eulerAngles(cameraRot); 
    glm::quat reflectedRot = glm::quat(glm::vec3(-rotAngles.x, rotAngles.y, -rotAngles.z));

    reflectionCamera->SetPositionAndRotation(reflectedPos, reflectedRot);
    reflectionCamera->Update(); 

    glEnable(GL_CLIP_DISTANCE0);


    glm::vec4 clipPlane = glm::vec4(0.0f, 1.0f, 0.0f, -lakeY); 

    GLuint groundShader = shaders["Ground"]->program; 
    glUniform4fv(glGetUniformLocation(groundShader, "clipPlane"), 1, glm::value_ptr(clipPlane));

    RenderTerrain(reflectionCamera->GetViewMatrix(), reflectionCamera->GetProjectionMatrix());
    RenderSkybox(reflectionCamera->GetViewMatrix(), reflectionCamera->GetProjectionMatrix());

    glDisable(GL_CLIP_DISTANCE0);

    FrameBuffer::BindDefault();
}



void Tema1::RenderLake(float deltaTimeSeconds,glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
    auto shader = shaders["Lake"];
    shader->Use(); 

    glActiveTexture(GL_TEXTURE0);
    reflectionFrameBuffer->BindTexture(2, GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader->program, "reflection_texture"), 0);

    float lakeY = 5.0f;
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, lakeY, 0.0f)); 
    model = glm::scale(model, glm::vec3(0.48f));                                     
    glUniformMatrix4fv(glGetUniformLocation(shader->program, "Model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    
    RenderMesh(meshes["lake"], shader, glm::vec3(0.0f, lakeY, 0.0f));
}



void Tema1::Update(float deltaTimeSeconds)
{
    ClearScreen(); 

    float curr_time = static_cast<float>(glfwGetTime());
    auto camera = GetSceneCamera();

    reflectionFrameBuffer->Bind();
    // glViewport(0, 0, 1024, 1024);

    RenderReflection(deltaTimeSeconds);
    
    // reflectionFrameBuffer->Bind();
    // glViewport(0, 0, reflectionFrameBuffer->GetWidth(), reflectionFrameBuffer->GetHeight());
    // ClearScreen(); // Clear depth and color buffers
    // RenderReflection(deltaTimeSeconds);
    // FrameBuffer::BindDefault();

    


    frameBuffer->Bind();
    // glViewport(0, 0, 1024, 1024);

    RenderLake(deltaTimeSeconds, camera->GetViewMatrix(), camera->GetProjectionMatrix());


    RenderParticles(deltaTimeSeconds);
    RenderTerrain(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    RenderSkybox(camera->GetViewMatrix(), camera->GetProjectionMatrix());

    lightBuffer->Bind();
    RenderLights(curr_time);

    // ------------------------------------------------------------------------
    // Lighting pass
    {
        glm::vec3 ambientLight(0.2f);

        //Set the initial light accumulation in each pixel to be equal to the ambient light.
        lightBuffer->SetClearColor(glm::vec4(ambientLight.x, ambientLight.y, ambientLight.z, 1.0f));
        lightBuffer->Bind();
        glClearColor(0, 0, 0, 1);

        // Enable buffer color accumulation
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        auto shader = shaders["LightPass"];
        shader->Use();

        {
            int texturePositionsLoc = shader->GetUniformLocation("texture_position");
            glUniform1i(texturePositionsLoc, 0);
            frameBuffer->BindTexture(0, GL_TEXTURE0);
        }

        {
            int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
            glUniform1i(textureNormalsLoc, 1);
            frameBuffer->BindTexture(1, GL_TEXTURE0 + 1);
        }

        auto camera = GetSceneCamera();
        glm::vec3 cameraPos = camera->m_transform->GetWorldPosition();
        int loc_eyePosition = shader->GetUniformLocation("eye_position");
        glUniform3fv(loc_eyePosition, 1, glm::value_ptr(cameraPos));

        auto resolution = window->GetResolution();
        int loc_resolution = shader->GetUniformLocation("resolution");
        glUniform2i(loc_resolution, resolution.x, resolution.y);

        //Front face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (auto& lightInf: lights)
        {
          
            
            location = glGetUniformLocation(shader->program, "light_position");
            glUniform3fv(location, 1, glm::value_ptr(lightInf.position));

            location = glGetUniformLocation(shader->program, "light_color");
            glUniform3fv(location, 1, glm::value_ptr(lightInf.color));
            
            location = glGetUniformLocation(shader->program, "light_radius");
            glUniform1f(location, lightInf.radius);

            RenderMesh(meshes["sphere"], shader, lightInf.position, glm::vec3(2 * lightInf.radius));

        }

        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

    }


    // ------------------------------------------------------------------------
    // Composition pass
    {   

        FrameBuffer::BindDefault();

        auto shader = shaders["Composition"];
        shader->Use();

        glUniform1i(shader->GetUniformLocation("output_type"), outputType);


        {
            int texturePositionsLoc = shader->GetUniformLocation("texture_position");
            glUniform1i(texturePositionsLoc, 1);
            reflectionFrameBuffer->BindTexture(2, GL_TEXTURE0 + 1);
        }

        {
            int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
            glUniform1i(textureNormalsLoc, 2);
            frameBuffer->BindTexture(1, GL_TEXTURE0 + 2);
        }

        {
            int textureColorLoc = shader->GetUniformLocation("texture_color");
            glUniform1i(textureColorLoc, 3);
            frameBuffer->BindTexture(2, GL_TEXTURE0 + 3);
        }

        {
            int textureDepthLoc = shader->GetUniformLocation("texture_depth");
            glUniform1i(textureDepthLoc, 4);
            frameBuffer->BindDepthTexture(GL_TEXTURE0 + 4);
        }

        {
            int textureLightLoc = shader->GetUniformLocation("texture_light");
            glUniform1i(textureLightLoc, 5);
            lightBuffer->BindTexture(0, GL_TEXTURE0 + 5);
        }
        
        // Render the object again but with different properties
        RenderMesh(meshes["quad"], shader, glm::vec3(0, 0, 0));
    }

}

void Tema1::FrameEnd()
{
    // DrawCoordinateSystem();
}

unsigned int Tema1::UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    unsigned int textureID = 0;

    // TODO(student): Create the texture
    glGenTextures(1, &textureID);

    // TODO(student): Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO(student): Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
    }

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}


void Tema1::ResetParticlesRain(int xSize, int ySize, int zSize)
{

    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());


    int xhSize = xSize / 2;
    int yhSize = ySize / 2;
    int zhSize = zSize / 2;

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec4 pos(1);
        pos.x = (rand() % xSize - xhSize) / 10.0f;
        pos.y = (rand() % ySize - yhSize) / 10.0f;
        pos.z = (rand() % zSize - zhSize) / 10.0f;

        glm::vec4 speed(0);
        speed.x = - (rand() % 20 - 10) / 10.0f;
        speed.z = - (rand() % 20 - 10) / 10.0f;
        speed.y = - (rand() % 2 + 2.0f);

        float delay = (rand() % 100 / 100.0f) * 3.0f;

        data[i].SetInitial(pos, speed, delay);
    }

    particleSSBO->SetBufferData(data);
}


void Tema1::LoadShader(const std::string &name)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Tema1::LoadShadergeom(const std::string& name, const std::string &VS, const std::string& FS, const std::string& GS,  bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, VS + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, FS + ".FS.glsl"), GL_FRAGMENT_SHADER);
        if (hasGeomtery)
        {
            shader->AddShader(PATH_JOIN(shaderPath, GS + ".GS.glsl"), GL_GEOMETRY_SHADER);
        }

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Tema1::OnInputUpdate(float deltaTime, int mods)
{

    if (window->KeyHold(GLFW_KEY_K)) {
        for (auto& lightInf : lights) {
            lightInf.radius += 0.1f;
        }
    }
    else if (window->KeyHold(GLFW_KEY_L)) {
        for (auto& lightInf : lights) {
            lightInf.radius = std::max(0.1f, lightInf.radius - 0.1f);
        } 
    }
}


void Tema1::OnKeyPress(int key, int mods)
{
    int index = key - GLFW_KEY_0;
    if (index >= 0 && index <= 6)
    {
        outputType = index;
    }

    // Add key press event
    if (key == GLFW_KEY_7)
    {
        type=1;
    }

    if (key == GLFW_KEY_8)
    {
        type=0;
    }

    
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
    frameBuffer->Resize(width, height, 32);
    lightBuffer->Resize(width, height, 32);
}
