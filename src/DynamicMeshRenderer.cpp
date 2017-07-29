//
//  DynamicMeshRenderer.h
//  BobosPuzzle
//
//  Created by Jas S on 2017-05-09.
//  Copyright © 2017 Jas S. All rights reserved.
//

#include "DynamicMeshRenderer.h"

_DynamicMeshRenderer DynamicMeshRenderer;

void _DynamicMeshRenderer::initalize()
{
    ShaderSource source = ShaderSource(INT_SHDR("dynamic.vert"), INT_SHDR("dynamic.frag"));
    this->source(source);
    this->bindAttributeLocation(0, "vertex");
    this->bindAttributeLocation(1, "normal");
    this->bindAttributeLocation(2, "uv");
    this->bindAttributeLocation(3, "texture_id");
    this->bindAttributeLocation(4, "node_index");
    this->bindAttributeLocation(5, "bone_weights");
    this->bindAttributeLocation(6, "bone_indices");
	this->bindFragDataLocation(DSFramebuffer::DIFFUSE_TEXTURE, "diffuse_out");
	this->bindFragDataLocation(DSFramebuffer::NORMAL_TEXTURE, "normal_out");
	this->bindFragDataLocation(DSFramebuffer::LIGHT_TEXTURE, "light_out");
    this->link(source);
    this->textures[0] = this->getUniform("textures[0]");
    this->textures[1] = this->getUniform("textures[1]");
    this->textures[2] = this->getUniform("textures[2]");
    this->textures[3] = this->getUniform("textures[3]");
    this->bones = this->getUniform("bones");
    this->nodes = this->getUniform("nodes");
    this->mvp_matrix = this->getUniform("mvp_matrix");
	this->normal_matrix = this->getUniform("normal_matrix");
}

void _DynamicMeshRenderer::bind()
{
    Shader::bind();
}

void _DynamicMeshRenderer::render(const DynamicMesh& mesh)
{
    mesh.bind();
    
    for(unsigned int i = 0; i < mesh.getTextureCount(); i++)
        mesh.getTextures()[i].bind(this->textures[i], i);
    
    glm::mat4 model_matrix = mesh.getMatrix();
    glm::mat4 vmatrix = Camera::getMatrix() * model_matrix;
    glm::mat4 nmatrix = glm::inverse(glm::transpose(Camera::getViewMatrix() * model_matrix));
    
    static glm::mat4 node_buffer[DYNAMIC_MESH_MAX_NODE_COUNT];
    static glm::mat4 bone_buffer[DYNAMIC_MESH_MAX_BONE_COUNT];
    
    for(unsigned int i = 0; i < mesh.getNodes().size(); i++) {
        node_buffer[i] = mesh.getNodes()[i].matrix * mesh.getNodes()[i].transform.toMatrix();
    }
    
    // FOR MORE INFO, SEE:
    // https://stackoverflow.com/questions/29184311/how-to-rotate-a-skinned-models-bones-in-c-using-assimp
    for(unsigned int i = 0; i < mesh.getBones().size(); i++) {
        // TODO: Add interpolation
        bone_buffer[i] = mesh.getInverseRoot() * mesh.getBones()[i].node->matrix * mesh.getBones()[i].node->transform.toMatrix() /* MULTIPLIED BY THE INTERPOLATED ANIMATION */ * mesh.getBones()[i].offset_matrix;
    }
    
    glUniformMatrix4fv(this->mvp_matrix, 1, GL_FALSE, &vmatrix[0][0]);
    glUniformMatrix4fv(this->bones, (unsigned int) mesh.getBones().size(), GL_FALSE, &bone_buffer[0][0][0]);
    glUniformMatrix4fv(this->nodes, (unsigned int) mesh.getNodes().size(), GL_FALSE, &node_buffer[0][0][0]);
	glUniformMatrix4fv(this->normal_matrix, 1, GL_FALSE, &nmatrix[0][0]);
    
    glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
    glBindVertexArray(0); // unbind the character's vao
}

void _DynamicMeshRenderer::destroy()
{
    Shader::destroy();
}
