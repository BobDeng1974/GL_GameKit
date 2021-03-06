//
//  SoftBody.cpp
//  CarDemo
//
//  Created by Jas S on 2017-07-01.
//  Copyright © 2017 Jas S. All rights reserved.
//

#include "SoftBody.h"


/* SOFT BODY MANAGER */

std::vector<SoftBody*> SoftBodyManager::soft_bodies;
btSoftRigidDynamicsWorld* SoftBodyManager::dynamics_world;

void SoftBodyManager::addSoftBody(SoftBody* soft_body) {
	std::vector<SoftBody*>::const_iterator it = std::find(soft_bodies.begin(), soft_bodies.end(), soft_body);
	if (it == soft_bodies.end()) {
		soft_bodies.push_back(soft_body);
		dynamics_world->addSoftBody(soft_body->getSoftBodyPointer());
	}
}

void SoftBodyManager::removeSoftBody(SoftBody* soft_body) {
	std::vector<SoftBody*>::const_iterator it = std::find(soft_bodies.begin(), soft_bodies.end(), soft_body);
	if (it != soft_bodies.end()) {
		soft_bodies.erase(it);
		dynamics_world->removeSoftBody(soft_body->getSoftBodyPointer());
	}
}

void SoftBodyManager::UpdateBodies() {
	for (unsigned int i = 0; i < soft_bodies.size(); i++) {
		if (soft_bodies[i]->hasTransformationPointer()) {
			soft_bodies[i]->hasTransformationPointer();
		}
	}
}

/* SOFT BODY */

SoftBody::SoftBody(MeshDescriptor& descriptor, const std::vector<int>& face_data, const Transform& transform, Transform* ptr)
{
    std::vector<glm::mat4> bone_cache;
    MeshLoader& loader = descriptor.getMeshLoader();
    bone_cache.reserve(loader.getBoneNames().size());
    for(unsigned int i = 0; i < loader.getBoneNames().size(); i++) {
        const glm::mat4& bindpose = loader.getNodeTransforms()[i];
        const glm::mat4& offset = loader.getBoneOffsets()[i];
        bone_cache.push_back(bindpose * offset);
    }
    const glm::mat4 local_transform = transform.toMatrix() * glm::scale(descriptor.getScale());
    
    this->face_array = std::vector<int>();
    //this->face_array = face_data;
    //this->face_array.shrink_to_fit();
    
    VectorTree vertex_tree;
    for(unsigned int i = 0; i < face_data.size(); i++)
    {
        const glm::vec3& vertex = loader.getVertices()[face_data[i]];
        const MeshLoader::BoneIndex& index = loader.getBoneIndices()[face_data[i]];
        const glm::vec4& weight = loader.getBoneWeights()[face_data[i]];
        glm::mat4 joint =
            weight.x * bone_cache[index.b0] +
            weight.y * bone_cache[index.b1] +
            weight.z * bone_cache[index.b2] +
            weight.w * bone_cache[index.b3];
        glm::vec3 v = glm::vec3(local_transform * joint * glm::vec4(vertex, 1));
        this->face_array.push_back(vertex_tree.insert(v));
    }
    
    std::vector<glm::vec3> vertex_array;
    vertex_tree.toArray(vertex_array);
    
    this->scale = glm::vec3(1.0f / descriptor.getScale().x, 1.0f / descriptor.getScale().y, 1.0f / descriptor.getScale().z);
    //this->soft_body = btSoftBodyHelpers::CreateFromTriMesh(PhysicsConfiguration::softbody_info, &vertex_array[0][0], &this->face_array[0], (unsigned int) this->face_array.size() / 3);
    this->bind();
    
    // finally update the loader
    loader.removeVertexBones(face_data);
}

SoftBody::~SoftBody()
{
    this->unbind();
}

void SoftBody::bind()
{
	SoftBodyManager::addSoftBody(this);
}

void SoftBody::unbind()
{
	SoftBodyManager::removeSoftBody(this);
}

void SoftBody::getVertexData(std::vector<glm::vec3>& vertex_data)
{
    btSoftBody::tNodeArray& nodes = this->soft_body->m_nodes;
    vertex_data.clear();
    vertex_data.reserve(this->face_array.size());
    
    for(unsigned int i = 0; i < this->face_array.size(); i++)
    {
        const btVector3& x = nodes[this->face_array[i]].m_x;
        glm::vec3 vec = glm::vec3(x.getX(), -x.getZ(), x.getY());
        vertex_data.push_back(glm::vec3(glm::scale(this->scale) * glm::vec4(vec, 1)));
    }
}

// OLD FUNCTION FOR SKINNED SOFT BODIES:
// VERY SLOW - NOT GOING TO BE USED FOR SOFT BODIES
// PLUS OPTIMIZATIONS SUCH AS COMPUTE SHADERS AND CACHING WILL PROBABLY OVER COMPLICATE THIS CODE BASE
/*
void SoftBody::getVertexData(std::vector<glm::vec3>& vertex_data)
{
    btSoftBody::tNodeArray& nodes = ((btSoftBody*) body)->m_nodes;
    vertex_data.clear();
    vertex_data.reserve(this->face_array.size());
    
    for(unsigned int i = 0; i < this->face_array.size(); i++)
    {
        const btVector3& x = nodes[this->face_array[i]].m_x;
        glm::vec3 vec = glm::vec3(x.getX(), x.getY(), x.getZ());
        const glm::uvec4& index = this->data->getBoneIndices()[face_array[i]];
        const glm::vec4& weight = this->data->getBoneWeights()[face_array[i]];
        glm::mat4 joint =
            weight.x * this->bone_cache[index.x] +
            weight.y * this->bone_cache[index.y] +
            weight.z * this->bone_cache[index.z] +
            weight.w * this->bone_cache[index.w];
        vertex_data.push_back(glm::vec3(glm::inverse(joint) * glm::scale(this->scale) * glm::vec4(vec, 1)));
    }
}
 */
