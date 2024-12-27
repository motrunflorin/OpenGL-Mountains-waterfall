#include "Ground.h"
#include "lab_m2/Tema1/Ground.h"

Ground::Ground(int rows, int cols, float height, float lakeRadius) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec2 lakeCenter(0.0f, 0.0f);

    float r_cascada = 2.0f;                

    GenerateGrid(vertices, rows, cols, height, lakeCenter, lakeRadius, r_cascada);

    GenerateIndices(indices, rows, cols);

    groundMesh = CreateMesh("terrain", vertices, indices);  
   
}

Ground::~Ground() {
    if (groundMesh) {
        delete groundMesh;
    }
}

Mesh* Ground::GetMesh() const {
    return groundMesh;
}

void Ground::GenerateGrid(std::vector<VertexFormat>& vertices, int rows, int cols, float height,
                          glm::vec2 lakeCenter, float lakeRadius, float r_cascada)
{

    float gridWidth = static_cast<float>(cols - 1);
    float gridHeight = static_cast<float>(rows - 1);

    float dx = gridWidth / (cols - 1); 
    float dz = gridHeight / (rows - 1);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            
            float x = -gridWidth / 2 + j * dx; 
            float z = -gridHeight / 2 + i * dz;

            float y;

            y = ConcaveConvexDisplacement(x, z, lakeCenter, lakeRadius, height);
            

            float distToLake = glm::distance(glm::vec2(x, z), lakeCenter);
            float lakeFactor = glm::clamp(distToLake / lakeRadius, 0.0f, 1.0f);
            
            
            float distanceFromCenter = sqrtf(x * x + z * z);
            float depthFactor = glm::clamp(distanceFromCenter / lakeRadius, 0.0f, 1.0f); 
            float convexShape = glm::smoothstep(0.0f, 2.0f, depthFactor);                

            float lakeBasin = height * (1.0f - convexShape) * (1.0f - convexShape) * (-0.6f); 

            float mountainNoise = height * 0.2f * ( 
                sinf(x * 0.3f) * cosf(z * 0.6f) +   
                sinf(z * 0.5f) * cosf(x * 0.8f) +   
                sinf(x * 0.2f) * cosf(z * 0.4f)     
            );

            mountainNoise *= convexShape; 

            y += lakeBasin + mountainNoise;
            y += ApplyCascadeDisplacement(x, z, rows, cols, r_cascada, height);


            float u = static_cast<float>(j) / static_cast<float>(cols);
            float v = static_cast<float>(i) / static_cast<float>(rows);


            vertices.emplace_back(
                glm::vec3(x, y, z),
                glm::vec3(0, 1, 0),
                glm::vec3(0, 0, 0),      
                glm::vec2(u, v)
            );
        }
    }
    
    CalculateNormals(vertices, rows, cols);

}

float Ground::ApplyCascadeDisplacement(float x, float z, int rows, int cols, float r_cascada, float height) {
    glm::vec3 point(x, 0.0f, z); 


    glm::vec3 curveStart(rows, height * 0.8f, cols);
    glm::vec3 curveControl1(rows / 2.0f, height * 0.8f, cols / 2.0f); 
    glm::vec3 curveControl2(rows / 4.0f, height * 0.8f, cols / 4.0f); 
    glm::vec3 curveEnd(0.0f, -5.0f, 0.0f); 

    glm::vec3 closestPoint;
    float closestDistance = std::numeric_limits<float>::max();
    float y_b_closest = height;

    for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
        glm::vec3 bezierPoint = glm::mix(
            glm::mix(glm::mix(curveStart, curveControl1, t), glm::mix(curveControl1, curveControl2, t), t),
            glm::mix(glm::mix(curveControl1, curveControl2, t), glm::mix(curveControl2, curveEnd, t), t),
            t
        );

        float distance = glm::distance(glm::vec2(point.x, point.z), glm::vec2(bezierPoint.x, bezierPoint.z));

        if (distance < closestDistance) {
            closestDistance = distance;
            closestPoint = bezierPoint;
            y_b_closest = bezierPoint.y; 
        }
    }

    float d_bezier = closestDistance / r_cascada;
    d_bezier = glm::clamp(d_bezier, 0.0f, 1.0f); 

    float curveAdjustment = 0.8f; 
    float depthFactor = glm::sin(glm::pi<float>() / 2.0f - d_bezier * glm::pi<float>() / 2.0f * curveAdjustment);
    float depthFactorModified = glm::pow(depthFactor, 1.2f); 

    float h = lerp(y_b_closest, height, 1.0f - depthFactorModified);

    h = glm::max(h, height - 4.0f); 

    return h; 
}


void Ground::GenerateIndices(std::vector<unsigned int>& indices, int rows, int cols) {
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {

            int index1 = i * cols + j;         
            int index2 = (i + 1) * cols + j;    
            int index3 = (i + 1) * cols + (j + 1); 
            int index4 = i * cols + (j + 1); 

            indices.push_back(index1);
            indices.push_back(index2);
            indices.push_back(index3);

            indices.push_back(index1);
            indices.push_back(index3);
            indices.push_back(index4);
        }
    }
}

void Ground::CalculateNormals(std::vector<VertexFormat>& vertices, int rows, int cols) {
    for (auto& vertex : vertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            int index1 = i * cols + j;
            int index2 = (i + 1) * cols + j;
            int index3 = (i + 1) * cols + (j + 1);

            glm::vec3 v1 = vertices[index1].position;
            glm::vec3 v2 = vertices[index2].position;
            glm::vec3 v3 = vertices[index3].position;

            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            vertices[index1].normal = normal;
            vertices[index2].normal = normal;
            vertices[index3].normal = normal;
        }
    }

    for (auto& vertex : vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

float Ground::ConcaveConvexDisplacement(float x, float z, glm::vec2 center, float radius, float h_max) {
    float d = glm::distance(glm::vec2(x, z), center) / radius;

    if (d < 1.0f) {
        return (d * d / 2.0f) * h_max; 
    } else {
        return (1.0f - ((2.0f - d) * (2.0f - d)) / 2.0f) * h_max; 
    }
}


Mesh* Ground::CreateMesh(const std::string& name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices) {

    Mesh* mesh = new Mesh(name);
    mesh->InitFromData(vertices, indices);

    return mesh;
}

