//
//  CylinderCollider.h
//  CarDemo
//
//  Created by Jas S on 2017-07-08.
//  Copyright © 2017 Jas S. All rights reserved.
//

#ifndef CylinderCollider_h
#define CylinderCollider_h

#include <vector>

#include "Math3d.h"
#include "Collider.h"
#include "RigidBody.h"
#include "MeshLoader.h"
#include "ColliderData.h"
#include "MeshDescriptor.h"
#include "ColliderUtilities.h"

class CylinderCollider : public RigidBody
{
public:
    CylinderCollider(const MeshDescriptor& descriptor, const float mass = 0, const glm::vec3& inertia = glm::vec3(0));
	CylinderCollider(const MeshDescriptor& descriptor, const std::vector<int>& faces, const float mass = 0, const glm::vec3& inertia = glm::vec3(0));
    ~CylinderCollider();
    
    static ColliderData getShape(const MeshDescriptor& descriptor, const std::vector<int>* faces = NULL);
};

#endif /* CylinderCollider_h */
