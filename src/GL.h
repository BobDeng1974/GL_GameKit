//
//  GL.h
//  Bobo
//
//  Created by Jas S on 2017-02-04.
//  Copyright © 2017 Jas S. All rights reserved.
//

#ifndef GL_h
#define GL_h

#include <OpenGL/gl3.h>

inline void glUnbindBuffer(){ glBindBuffer(GL_ARRAY_BUFFER, 0); }
inline void glUnbindVertexArray(){ glBindBuffer(GL_ARRAY_BUFFER, 0); }
inline void glUnbindShader(){ glUseProgram(0); }
inline void glUnbindFramebuffer(){ glBindFramebuffer(GL_FRAMEBUFFER, 0); }

#endif /* GL_h */
