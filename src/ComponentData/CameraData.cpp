#include "../../include/ComponentData/CameraData.hpp"

namespace mall
{
    void CameraData::setup(glm::vec3 lookPos_, glm::vec3 up_, float aspect_, float fovY_,float near_, float far_)
    {
        enable = true;
        
        lookPos = lookPos_;
        up = up_;
        aspect = aspect_;
        fovY = fovY_;
        near = near_;
        far = far_;
    }

}