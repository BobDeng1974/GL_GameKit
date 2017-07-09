//
//  Transform.h
//  CarDemo
//
//  Created by Jas S on 2017-07-03.
//  Copyright © 2017 Jas S. All rights reserved.
//

#ifndef Transform_h
#define Transform_h

#include "Math3d.h"
#include <LinearMath/btTransform.h>

class Transform
{
public:
    glm::vec3 translation;
    glm::quat rotation;
    
    Transform(const glm::vec3& _translation, const glm::vec3& _rotation);
    Transform(const glm::vec3& _translation = glm::vec3(0), const glm::quat& _rotation = glm::quat(0, 0, 0, 1));
    
    inline glm::mat4 toMatrix() const { return glm::translate(this->translation) * glm::toMat4(this->rotation); }
    inline btTransform toBulletTransform() const {
        btTransform transform;
        transform.setOrigin(btVector3(this->translation.x, this->translation.y, this->translation.z));
        transform.setRotation(btQuaternion(this->rotation.x, this->rotation.y, this->rotation.z, this->rotation.w));
        return transform;
    }
};

#endif /* Transform_h */
