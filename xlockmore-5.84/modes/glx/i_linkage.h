#include <GL/gl.h>
#ifndef WIN32
#include <GL/glx.h>
#endif

typedef struct {
  GLfloat     view_rotx, view_roty, view_rotz;
  GLuint      frames;
  int         numsteps;
  int         construction;
  int         time;
  char       *partlist;
  int         forwards;
  Window      window;
#ifdef WIN32
  HGLRC       glx_context;
#else
  GLXContext *glx_context;
#endif
} spherestruct;


#ifdef __cplusplus
extern "C" {
#endif
  Bool invert_draw(spherestruct *gp);
#ifdef __cplusplus
}
#endif
