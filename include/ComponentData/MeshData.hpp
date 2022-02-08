#ifndef MALL_COMPONENTDATA_MESHDATA_HPP_
#define MALL_COMPONENTDATA_MESHDATA_HPP_

#include <Cutlass/Cutlass.hpp>
#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>

#include "../Utility.hpp"

namespace mall
{
#define EQ(MEMBER) (MEMBER == another.MEMBER)  //便利

    struct MeshData : public mvecs::IComponentData
    {
        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
            glm::vec4 joint;
            glm::vec4 weight;

            bool operator==(const Vertex& another) const
            {
                return EQ(pos) && EQ(normal) && EQ(uv) && EQ(joint) && EQ(weight);
            }

            bool operator!=(const Vertex& another) const
            {
                return !(*this == another);
            }
        };

        struct Mesh
        {
            Mesh()
            {
                vertices.reserve(32);
                indices.reserve(32);
            };

            std::vector<Vertex> vertices;
            std::vector<std::uint32_t> indices;
            Cutlass::HBuffer VB;
            Cutlass::HBuffer IB;
        };

        COMPONENT_DATA(MeshData)

        bool loaded;
        TUArray<Mesh> meshes;
    };
}  // namespace mall

#endif