#ifndef GROUND_H
#define GROUND_H

#include <string>
#include <vector>
#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/frame_buffer.h"

class Ground {
public:
    Ground(int rows, int cols, float height, float lakeRadius);
    ~Ground();

    Mesh* GetMesh() const;
    static float ConcaveConvexDisplacement(float x, float z, glm::vec2 center, float radius, float h_max);
    static float ApplyCascadeDisplacement(float x, float z, int rows, int cols, float r_cascada, float height);



private:
    Mesh* groundMesh;

    
    void GenerateGrid(std::vector<VertexFormat>& vertices, int rows, int cols, float height,
                          glm::vec2 lakeCenter, float lakeRadius, float r_cascada);

    void GenerateIndices(std::vector<unsigned int>& indices, int rows, int cols);

    Mesh* CreateMesh(const std::string& name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);

    void CalculateNormals(std::vector<VertexFormat>& vertices, int rows, int cols);  
};

#endif // GROUND_H
