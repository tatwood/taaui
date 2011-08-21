/**
 * @brief     render command processing implementation for OpenGL 1.1
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
// only compile when included by uirender.c
#ifdef TAA_UIRENDER_C_

void taa_uirender(
    uint32_t vieww,
    uint32_t viewh,
    const taa_uidrawlist* dl)
{
    const uint8_t* verts = (const uint8_t*) (*dl->vb.gl11);
    const taa_uidrawlist_cmd* cmditr = dl->cmds;
    const taa_uidrawlist_cmd* cmdend = cmditr + dl->numcmds;
    // set default matrices
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1.0f,  1.0f, 0.0f);
    glScalef(2.0f/vieww,-2.0f/viewh, 1.0f);
    glTranslatef(0.5f, -0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // set render state
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    while(cmditr != cmdend)
    {
        const taa_uidrawlist_vertex* v;
        v = (const taa_uidrawlist_vertex*) (verts + cmditr->vboffset);
        glBindTexture(GL_TEXTURE_2D, cmditr->texture.gl11);
        glVertexPointer(2, GL_FLOAT, sizeof(*v), &v->pos);
        glTexCoordPointer(2, GL_FLOAT, sizeof(*v), &v->uv);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(*v), &v->color);
        glDrawArrays(GL_TRIANGLES, 0, cmditr->numvertices);
        ++cmditr;
    }
    // revert render state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

#endif // TAA_UIRENDER_C_
