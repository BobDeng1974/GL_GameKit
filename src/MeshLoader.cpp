//
//  MeshLoader.cpp
//
//  Created by Jas S on 2017-02-04.
//  Copyright © 2017 Jas S. All rights reserved.
//

#include "MeshLoader.h"

MeshLoader::MeshLoader(const char* _path, unsigned int parameters)
{
    this->path = std::string(_path);
    this->directory = this->path.substr(0, this->path.find_last_of("/\\") + 1);
    
    Assimp::Importer* importer = new Assimp::Importer();
    const struct aiScene* scene = importer->ReadFile(_path, aiProcess_Triangulate);
	if (!scene) {
		printf("Error: Could not open model file %s.\n", _path);
		throw -1;
	}

    unsigned int vertex_count  = 0; // store the total vertices passed because each mesh's indices start at 0 (so this is a cumulative sum)
    unsigned int texture_index = 0;
    unsigned int material_index = 0;
    
    // start by processing/reading the scene's nodes
    this->process_nodes(scene->mRootNode);

    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        const struct aiMesh* mesh = scene->mMeshes[i];
        const std::string name = std::string(mesh->mName.C_Str());
        
        if(std::find(this->mesh_names.begin(), this->mesh_names.end(), name) == this->mesh_names.end())
            this->mesh_names.push_back(name);
        std::vector<int>& mesh_list_pointer = this->mesh_faces[name];
        
        std::vector<std::string>::const_iterator node_pos = std::find(this->node_names.begin(), this->node_names.end(), name);
        if(node_pos == this->node_names.end()) {
            printf("Error: Node [%s] was not found in collada file [%s]!\n", name.c_str(), _path);
            throw -1;
        }
        const unsigned char node_index = (unsigned char) (node_pos - this->node_names.begin());

        if(scene->mMaterials[mesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString tex_path;
            if (scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
                puts("Assimp texture path could not be read.");
                throw - 1;
            }
            //printf("Debug: Read texture %s\n", tex_path.C_Str());
            std::string tex = std::string(tex_path.C_Str());
            std::vector<std::string>::const_iterator index = std::find(this->textures.begin(), this->textures.end(), tex);
            if (index == this->textures.end()) {
                texture_index = (unsigned int) this->textures.size();
                this->textures.push_back(tex);
            }
            else texture_index = (unsigned int) (index - this->textures.begin());
        }
        
        float shininess = 0;
        if(scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            shininess = 1.0f - shininess / 100.0f;
            printf("Shineness of mesh %s: %f\n", mesh->mName.C_Str(), shininess);
        }
        const std::vector<float>::const_iterator m_it = std::find(this->materials.begin(), this->materials.end(), shininess);
        if(m_it == this->materials.end()) {
            material_index = (unsigned int) this->materials.size();
            this->materials.push_back(shininess);
        } else material_index = (unsigned int) (m_it - this->materials.begin());
        
        for(unsigned int o = 0; o < mesh->mNumVertices; o++)
        {
            if(mesh->HasPositions()) {
                const aiVector3D* v = &mesh->mVertices[o];
                glm::vec3 vec;
                if(parameters & COLLADA_AXIS_SWAP)
                    vec = glm::vec3(v->x, v->z, -v->y);
                else
                    vec = glm::vec3(v->x, v->y, v->z);
                
                this->vertices.push_back(vec);
            }
            if(mesh->HasNormals()) {
                const aiVector3D* v = &mesh->mNormals[o];
                glm::vec3 vec = glm::vec3(v->x, v->y, v->z);
                this->normals.push_back(vec);
            }
			if (mesh->HasTextureCoords(0)) { // ONLY SUPPORTS ONE CHANNEL OF UVS (you can't put more than one texture per face)
				const aiVector3D* v = &mesh->mTextureCoords[0][o];
				glm::vec2 vec = glm::vec2(v->x, v->y);
                this->uvs.push_back(vec);
			}
            else { this->uvs.push_back(glm::vec2(0)); }
        }
        if(mesh->HasFaces()) {
            for(unsigned int o = 0; o < mesh->mNumFaces; o++) {
                const aiFace* face = &mesh->mFaces[o];
                if(face->mNumIndices != 3) {
                    //printf("Warning: Unsupported face index count of %i. Only 3 indices per face are supported. Skipping face.\n", face->mNumIndices);
                    continue;
                }
                for(unsigned int t = 0; t < 3; t++) {
                    mesh_list_pointer.push_back(vertex_count + face->mIndices[t]); // copy the index into the mesh indexing list
                    this->faces.push_back(vertex_count + face->mIndices[t]); // add the triangle vertex for the vbo
					this->texture_indices.push_back(texture_index);
                    this->node_indices.push_back(node_index);
                    this->material_indices.push_back(material_index);
                    
                    //const aiVector3D& vd = mesh->mVertices[face->mIndices[t]];
                    //const glm::vec3& v = this->vertices[vertex_count + face->mIndices[t]];
                    //printf("%f %f %f vs %f %f %f\n", vd.x, vd.y, vd.z, v.x, v.y, v.z);
                }
            }
        }
        vertex_count += mesh->mNumVertices;
    }

    for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        glm::vec4* weights = new glm::vec4[mesh->mNumVertices]();
        BoneIndex* indices = new BoneIndex[mesh->mNumVertices]();
        unsigned int* counts = new unsigned int[mesh->mNumVertices]();
        if(mesh->HasBones()) {
            for(unsigned int o = 0; o < mesh->mNumBones; o++) {
                const aiBone* bone = mesh->mBones[o];
                std::vector<std::string>::const_iterator name_it = std::find(this->bone_names.begin(), this->bone_names.end(), std::string(bone->mName.C_Str()));
                unsigned int bone_index = 0;
                if(name_it == this->bone_names.end()) {
                    bone_index = (unsigned int) this->bone_names.size();
                    this->bone_names.push_back(std::string(bone->mName.C_Str()));
                    glm::mat4 matrix;
                    memcpy(&matrix[0][0], &bone->mOffsetMatrix[0][0], sizeof(float) * 16);
                    this->bone_offsets.push_back(glm::transpose(matrix));
                }
                else bone_index = (unsigned int) (name_it - this->bone_names.begin());
                
                for(unsigned int w = 0; w < bone->mNumWeights; w++) {
                    const struct aiVertexWeight* weight = &bone->mWeights[w];
                    if(counts[weight->mVertexId] < 4) {
                        indices[weight->mVertexId][counts[weight->mVertexId]] = bone_index;
                        weights[weight->mVertexId][counts[weight->mVertexId]] = weight->mWeight;
                        counts[weight->mVertexId]++;
                    } else {
                        puts("Vertex limit of 4 bones per vertex exceeded. Terminating.");
                        throw -1;
                    }
                }
            }
        }
        this->bone_weights.insert(this->bone_weights.end(), &weights[0], &weights[0] + mesh->mNumVertices);
        this->bone_indices.insert(this->bone_indices.end(), &indices[0], &indices[0] + mesh->mNumVertices);
        delete[] weights;
        delete[] indices;
        delete[] counts;
    }
    
    for(unsigned int i = 0; i < scene->mNumAnimations; i++) {
        const aiAnimation* animation = scene->mAnimations[i];
        for(unsigned int n = 0; n < animation->mNumChannels; n++) {
            const aiNodeAnim* node_animation = animation->mChannels[n];
            Animation animation = Animation();
            for(unsigned int o = 0; o < node_animation->mNumPositionKeys; o++) {
                const aiVector3D translation = node_animation->mPositionKeys[o].mValue;
                const aiVector3D scale = node_animation->mScalingKeys[o].mValue;
                const aiQuaternion rotation = node_animation->mRotationKeys[o].mValue;
                animation.translations.push_back(glm::vec3(translation.x, translation.y, translation.z));
                animation.scales.push_back(glm::vec3(scale.x, scale.y, scale.z));
                animation.rotations.push_back(glm::vec4(rotation.x, rotation.y, rotation.z, rotation.w));
            }
            this->bone_animations[std::string(node_animation->mNodeName.C_Str())] = animation;
        }
    }
    
    memcpy(&this->inverse_root[0][0], &scene->mRootNode->mTransformation[0][0], sizeof(float) * 16);
    this->inverse_root = glm::inverse(this->inverse_root);
    delete importer;
}

glm::mat4 MeshLoader::calculate_node(const aiNode* root) {
    glm::mat4 ret = glm::mat4(1.0f);
    const aiNode* node = root;
    while(node != NULL) {
        glm::mat4 mat;
        memcpy(&mat[0][0], &node->mTransformation[0][0], sizeof(float) * 16);
        ret = glm::transpose(mat) * ret;
        node = node->mParent;
    }
    return ret;
}

void MeshLoader::process_nodes(const aiNode* node)
{
    if(node == NULL) return;
    this->node_names.push_back(std::string(node->mName.C_Str()));
    this->node_transforms.push_back(this->calculate_node(node));
    if(node->mParent != NULL) this->node_parents[std::string(node->mName.C_Str())] = std::string(node->mParent->mName.C_Str());
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        this->process_nodes(node->mChildren[i]);
    }
}

void MeshLoader::removeVertexBones(const std::vector<int>& mesh_faces)
{
    for(unsigned int i = 0; i < mesh_faces.size(); i++) {
        this->bone_weights.at(mesh_faces[i]) = glm::vec4(0);
        this->bone_indices.at(mesh_faces[i]) = BoneIndex();
    }
}

void MeshLoader::getDynamicVertexArray(std::vector<glm::vec3>& source) const
{
    source.clear();
    source.reserve(this->faces.size());
    for(unsigned int i = 0; i < this->faces.size(); i++)
        source.push_back(glm::vec3(this->vertices[this->faces[i]]));
}

void MeshLoader::getStaticVertexArray(std::vector<glm::vec3>& source) const
{
    source.clear();
    source.reserve(this->faces.size());
    for(unsigned int i = 0; i < this->faces.size(); i++)
        source.push_back(glm::vec3(this->node_transforms[this->node_indices[this->faces[i]]] * glm::vec4(this->vertices[this->faces[i]], 1)));
}

void MeshLoader::getNormalArray(std::vector<glm::vec3>& source) const
{
    source.clear();
    source.reserve(this->faces.size());
    for(unsigned int i = 0; i < this->faces.size(); i++)
        source.push_back(this->normals[this->faces[i]]);
}

void MeshLoader::getUvArray(std::vector<glm::vec2>& source) const
{
    source.clear();
    source.reserve(this->faces.size());
    for(unsigned int i = 0; i < this->faces.size(); i++)
        source.push_back(this->uvs[this->faces[i]]);
}

void MeshLoader::getMaterialArray(std::vector<unsigned char>& source) const
{
    source.clear();
    source.reserve(this->faces.size());
    for(unsigned int i = 0; i < this->faces.size(); i++)
        source.push_back(this->material_indices[this->faces[i]]);
}

void MeshLoader::genTextures(Texture** array) const {
    Texture* src = new Texture[this->textures.size()];
    for(unsigned int i = 0; i < this->textures.size(); i++)
        src[i] = Texture(this->directory + this->textures[i]);
    *array = src;
}
