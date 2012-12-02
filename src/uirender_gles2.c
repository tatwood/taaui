/**
 * @brief     render command processing implementation for OpenGL ES 2
 * @author    Thomas Atwood (tatwood.net)
 * @date      2012
 * @copyright unlicense / public domain
 ****************************************************************************/
// only compile when included by uirender.c
#ifdef taa_UIRENDER_C_
#include <taa/uirender.h>
#include <GL/glew.h>

//****************************************************************************

typedef struct taa_ui_render_private_s taa_ui_render_private;

struct taa_ui_render_private_s
{
    GLuint poname;
    GLint at_pos;
    GLint at_uv0;
    GLint at_col;
    GLint un_mvp;    
};

//****************************************************************************

static const char* s_taa_ui_render_vs =
    "uniform mat4 un_mvp;"
    "attribute vec2 at_pos;"
    "attribute vec2 at_uv0;"
    "attribute vec4 at_col;"
    "varying vec2 va_uv;"
    "varying vec4 va_color;"
    "void main()"
    "{"
    "    va_uv = at_uv0;"
    "    va_color = at_col;"
    "    gl_Position = un_mvp * vec4(at_pos, 0.0, 1.0);"
    "}";

static const char* s_taa_ui_render_fs =
    "uniform sampler2D un_diff;"
    "varying vec2 va_uv;"
    "varying vec4 va_color;"
    "void main()"
    "{"
    "    gl_FragColor = va_color * texture2D(un_diff, va_uv);"
    "}";

//****************************************************************************
static GLuint taa_compile_ui_render_po(
    const char* vssrc,
    const char* fssrc)
{
    GLuint vsname;
    GLuint fsname;
    GLuint poname;
    vsname = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsname, 1, &vssrc, NULL);
    glCompileShader(vsname);
    fsname = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsname, 1, &fssrc, NULL);
    glCompileShader(fsname);
#ifndef NDEBUG
    {
        int vsresult;
        int fsresult;
        glGetShaderiv(vsname, GL_COMPILE_STATUS, &vsresult);
        glGetShaderiv(fsname, GL_COMPILE_STATUS, &fsresult);
        if(vsresult != GL_TRUE)
        {
            int len;
            char* log;
            glGetShaderiv(vsname, GL_INFO_LOG_LENGTH, &len);
            log = (char*) malloc(len + 1);
            glGetShaderInfoLog(vsname, len, NULL, log);
            log[len] = '\0';
            puts("vertex shader failed to compile:");
            puts(log);
            free(log);
        }
        if(fsresult != GL_TRUE)
        {
            int len;
            char* log;
            glGetShaderiv(fsname, GL_INFO_LOG_LENGTH, &len);
            log = (char*) malloc(len + 1);
            glGetShaderInfoLog(fsname, len, NULL, log);
            log[len] = '\0';
            puts("fragment shader failed to compile:");
            puts(log);
            free(log);
        }
        assert(vsresult == GL_TRUE);
        assert(fsresult == GL_TRUE);
    }
#endif
    poname = glCreateProgram();
    glAttachShader(poname, vsname);
    glAttachShader(poname, fsname);
    glLinkProgram(poname);
    glDeleteShader(fsname);
    glDeleteShader(vsname);
    return poname;
}

//****************************************************************************
void taa_ui_create_render_data(
    taa_ui_render_data** rnd_out)
{
    taa_ui_render_private* prnd;
    GLuint poname;
    prnd = (taa_ui_render_private*) taa_memalign(16, sizeof(*prnd));
    poname = taa_compile_ui_render_po(
        s_taa_ui_render_vs,
        s_taa_ui_render_fs);
    prnd->poname = poname;
    prnd->un_mvp = glGetUniformLocation(poname, "un_mvp");
    prnd->at_pos = glGetAttribLocation(poname, "at_pos");
    prnd->at_uv0 = glGetAttribLocation(poname, "at_uv0");
    prnd->at_col = glGetAttribLocation(poname, "at_col");
    glUseProgram(poname);
    glUniform1i(glGetUniformLocation(poname, "un_diff"), 0);
    glUseProgram(0);
    *rnd_out = (taa_ui_render_data*) prnd;
}

//****************************************************************************
void taa_ui_destroy_render_data(
    taa_ui_render_data* rnd)
{
    taa_ui_render_private* prnd = (taa_ui_render_private*) rnd;
    glDeleteProgram(prnd->poname);
    taa_memalign_free(prnd);
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
    taa_ui_render_private* prnd = (taa_ui_render_private*) rnd;
    const taa_ui_drawlist_cmd* cmditr =  cmds;
    const taa_ui_drawlist_cmd* cmdend = cmditr +  numcmds;
    taa_mat44 mvp;
    int off = 0;
    // set render state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glUseProgram(prnd->poname);
    glEnableVertexAttribArray(prnd->at_pos);
    glEnableVertexAttribArray(prnd->at_uv0);
    glEnableVertexAttribArray(prnd->at_col);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(
        prnd->at_pos,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(taa_ui_vertex),
        (void*) offsetof(taa_ui_vertex, pos));
    glVertexAttribPointer(
        prnd->at_uv0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(taa_ui_vertex),
        (void*) offsetof(taa_ui_vertex, uv));
    glVertexAttribPointer(
        prnd->at_col,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(taa_ui_vertex),
        (void*) offsetof(taa_ui_vertex, color));
    // set default matrix
    taa_vec4_set(2.0f/vieww, 0.0f, 0.0f, 0.0f, &mvp.x);
    taa_vec4_set(0.0f, -2.0f/viewh, 0.0f, 0.0f, &mvp.y);
    taa_vec4_set(0.0f, 0.0f, 1.0f, 0.0f, &mvp.z);
    taa_vec4_set(-1.0f, 1.0f, 0.0f, 1.0f, &mvp.w);
    glUniformMatrix4fv(prnd->un_mvp, 1, GL_FALSE, &mvp.x.x);
    // draw
    while(cmditr != cmdend)
    {
        glBindTexture(GL_TEXTURE_2D, cmditr->texture);
        glDrawArrays(GL_TRIANGLES, off, cmditr->numvertices);
        off += cmditr->numvertices;
        ++cmditr;
    }
    // revert render state
    glUseProgram(0);
    glDisableVertexAttribArray(prnd->at_pos);
    glDisableVertexAttribArray(prnd->at_uv0);
    glDisableVertexAttribArray(prnd->at_col);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

#endif // taa_UIRENDER_C_
