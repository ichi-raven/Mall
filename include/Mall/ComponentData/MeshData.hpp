#ifndef MALL_COMPONENTDATA_MESHDATA_HPP_
#define MALL_COMPONENTDATA_MESHDATA_HPP_

#include <Cutlass/Cutlass.hpp>
#include <MVECS/IComponentData.hpp>
#include <glm/glm.hpp>

#include <cstdint>

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
            std::vector<Vertex> vertices;
            std::vector<std::uint32_t> indices;
            Cutlass::HBuffer VB;
            Cutlass::HBuffer IB;
        };

        struct RenderingInfo
        {
            struct SceneCBParam
            {
                glm::mat4 world;
                glm::mat4 view;
                glm::mat4 proj;
                float receiveShadow;
                float lighting;
                std::uint32_t useBone;
                std::uint32_t padding;
            };

            Cutlass::HBuffer sceneCB;
        };

        COMPONENT_DATA(MeshData)

        bool loaded;
        TUArray<Mesh> meshes;

        // この行列は描画時にまず掛けられる
        glm::mat4 defaultAxis;

        // 毎F更新されるワールド行列
        glm::mat4 world;

        RenderingInfo renderingInfo;
    };
}  // namespace mall

#endif