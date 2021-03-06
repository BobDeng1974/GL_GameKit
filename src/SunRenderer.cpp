#include "SunRenderer.h"

_SunRenderer SunRenderer;

void _SunRenderer::initalize(const DSFramebuffer& fbo)
{
    ShaderSource source = ShaderSource(INT_SHDR("sun.vert"), INT_SHDR("sun.frag"));
    this->source(source);
    this->bindAttributeLocation(0, "vertex");
	this->bindFragDataLocation(0, "light_out");
	this->bindFragDataLocation(1, "specular_out");
    this->link(source);
    
    this->sun_position = this->getUniform("sun_position");
    this->sun_color = this->getUniform("sun_color");
    this->screen_size = this->getUniform("screen_size");
    this->inv_proj_matrix = this->getUniform("inv_proj_matrix");
    
    this->diffuse_texture = this->getUniform("diffuse_texture");
    this->normal_texture = this->getUniform("normal_texture");
    this->depth_texture = this->getUniform("depth_texture");
    this->brightness_texture = this->getUniform("brightness_texture");
    
    this->framebuffer = &fbo;
}

void _SunRenderer::render()
{
    this->bind();
    Quad::bind();
    
    glm::vec3 sun_position_cameraspace = glm::vec3(Camera::getViewMatrix() * glm::vec4(Sun::position, 1));
    glm::mat4 inverse_projection = glm::inverse(Camera::getProjectionMatrix());
    
    glUniformMatrix4fv(this->inv_proj_matrix, 1, GL_FALSE, &inverse_projection[0][0]);
    
    glUniform2f(this->screen_size, (float) this->framebuffer->getDimensions().x, (float) this->framebuffer->getDimensions().y);
    glUniform3fv(this->sun_position, 1, &sun_position_cameraspace[0]);
    glUniform3fv(this->sun_color, 1, &Sun::color[0]);
    
    this->framebuffer->bindTexture(this->diffuse_texture, DSFramebuffer::DIFFUSE_TEXTURE, 0);
    this->framebuffer->bindTexture(this->normal_texture, DSFramebuffer::NORMAL_TEXTURE, 1);
    this->framebuffer->bindTexture(this->brightness_texture, DSFramebuffer::BRIGHTNESS_TEXTURE, 2);
    this->framebuffer->bindTexture(this->depth_texture, DSFramebuffer::DEPTH_TEXTURE, 3);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
}

void _SunRenderer::destroy()
{
    Shader::destroy();
}
