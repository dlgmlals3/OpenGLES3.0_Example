#include <GLES3/gl3.h>
#include "model.h"
#include "GLutils.h"
//#include "base/glutils.h"
//#include "MapHelper.h"
//#include "../Basic/QIPoint.hpp"

bool Model::useProgram(char* programName) 
{
	return false;
}

void Model::CheckOpenGLError(const char *file, int line) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        LOGI("OpenGL Error in file %s at line %d: %d", file, line, error);
    }
}