/**
 * @brief     render command processing implementation for OpenGL 1.1
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
// only compile when included by uirender.c
#ifdef taa_UIRENDER_C_
#include <taa/uirender.h>
#include <GL/gl.h>

//****************************************************************************
void taa_ui_create_render_data(
    taa_ui_render_data** rnd_out)
{
    *rnd_out = NULL;
}

//****************************************************************************
void taa_ui_destroy_render_data(
    taa_ui_render_data* rnd)
{
}

//****************************************************************************
void taa_ui_render(
    taa_ui_render_data* rnd,
    int vieww,
    int viewh,
    taa_vertexbuffer vb,
    const taa_ui_drawlist_cmd* cmds,
    size_t numcmds)
{
    const taa_ui_vertex* v = (const taa_ui_vertex*) (*((void**) vb));
    const taa_ui_drawlist_cmd* cmditr =  cmds;
    const taa_ui_drawlist_cmd* cmdend = cmditr +  numcmds;
    int off = 0;
    // set default matrices
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1.0f, 1.0f, 0.0f);
    glScalef(2.0f/vieww,-2.0f/viewh, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // set render state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(*v), &v->pos);
    glTexCoordPointer(2, GL_FLOAT, sizeof(*v), &v->uv);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(*v), &v->color);
    while(cmditr != cmdend)
    {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) cmditr->texture);
        glDrawArrays(GL_TRIANGLES, off, cmditr->numvertices);
        off += cmditr->numvertices;
        ++cmditr;
    }
    // revert render state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

#endif // taa_UIRENDER_C_
