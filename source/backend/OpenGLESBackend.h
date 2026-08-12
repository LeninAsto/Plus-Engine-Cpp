/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * OpenGLESBackend - OpenGL ES 3.x renderer backend
 *
 * SDL still owns window/input/audio. This backend owns the GL context,
 * shaders, GPU textures, and draw calls.
 */

#pragma once

#include "RenderTypes.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <string>

namespace FNF {

class OpenGLESBackend {
public:
    bool Init(SDL_Window* window, int width, int height, bool vsync);
    void Shutdown();

    void BeginFrame(const RenderColor& clearColor);
    void EndFrame();
    void Resize(int width, int height);

    TextureHandle CreateTextureFromSurface(SDL_Surface* surface, const std::string& debugName = "");
    void DestroyTexture(TextureHandle& texture);

    void DrawTexture(const SpriteDrawCommand& cmd);
    void FillRect(const RenderRect& rect, const RenderColor& color);
    void SetClipRect(const RenderRect& rect);
    void ClearClipRect();

    bool IsReady() const { return m_Ready; }
    const std::string& RendererName() const { return m_RendererName; }

private:
    using GLActiveTexture = void (GL_APIENTRYP)(GLenum texture);
    using GLAttachShader = void (GL_APIENTRYP)(GLuint program, GLuint shader);
    using GLBindBuffer = void (GL_APIENTRYP)(GLenum target, GLuint buffer);
    using GLBindTexture = void (GL_APIENTRYP)(GLenum target, GLuint texture);
    using GLBlendFunc = void (GL_APIENTRYP)(GLenum sfactor, GLenum dfactor);
    using GLBufferData = void (GL_APIENTRYP)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    using GLClear = void (GL_APIENTRYP)(GLbitfield mask);
    using GLClearColor = void (GL_APIENTRYP)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    using GLCompileShader = void (GL_APIENTRYP)(GLuint shader);
    using GLCreateProgram = GLuint (GL_APIENTRYP)(void);
    using GLCreateShader = GLuint (GL_APIENTRYP)(GLenum type);
    using GLDeleteBuffers = void (GL_APIENTRYP)(GLsizei n, const GLuint* buffers);
    using GLDeleteProgram = void (GL_APIENTRYP)(GLuint program);
    using GLDeleteShader = void (GL_APIENTRYP)(GLuint shader);
    using GLDeleteTextures = void (GL_APIENTRYP)(GLsizei n, const GLuint* textures);
    using GLDisable = void (GL_APIENTRYP)(GLenum cap);
    using GLDrawArrays = void (GL_APIENTRYP)(GLenum mode, GLint first, GLsizei count);
    using GLEnable = void (GL_APIENTRYP)(GLenum cap);
    using GLEnableVertexAttribArray = void (GL_APIENTRYP)(GLuint index);
    using GLGenBuffers = void (GL_APIENTRYP)(GLsizei n, GLuint* buffers);
    using GLGenTextures = void (GL_APIENTRYP)(GLsizei n, GLuint* textures);
    using GLGetIntegerv = void (GL_APIENTRYP)(GLenum pname, GLint* data);
    using GLGetProgramInfoLog = void (GL_APIENTRYP)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    using GLGetProgramiv = void (GL_APIENTRYP)(GLuint program, GLenum pname, GLint* params);
    using GLGetShaderInfoLog = void (GL_APIENTRYP)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    using GLGetShaderiv = void (GL_APIENTRYP)(GLuint shader, GLenum pname, GLint* params);
    using GLGetString = const GLubyte* (GL_APIENTRYP)(GLenum name);
    using GLGetUniformLocation = GLint (GL_APIENTRYP)(GLuint program, const GLchar* name);
    using GLLinkProgram = void (GL_APIENTRYP)(GLuint program);
    using GLPixelStorei = void (GL_APIENTRYP)(GLenum pname, GLint param);
    using GLScissor = void (GL_APIENTRYP)(GLint x, GLint y, GLsizei width, GLsizei height);
    using GLShaderSource = void (GL_APIENTRYP)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    using GLTexImage2D = void (GL_APIENTRYP)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    using GLTexParameteri = void (GL_APIENTRYP)(GLenum target, GLenum pname, GLint param);
    using GLUniform1i = void (GL_APIENTRYP)(GLint location, GLint v0);
    using GLUniform2f = void (GL_APIENTRYP)(GLint location, GLfloat v0, GLfloat v1);
    using GLUniform4f = void (GL_APIENTRYP)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    using GLUseProgram = void (GL_APIENTRYP)(GLuint program);
    using GLVertexAttribPointer = void (GL_APIENTRYP)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    using GLViewport = void (GL_APIENTRYP)(GLint x, GLint y, GLsizei width, GLsizei height);

    bool LoadGL();
    bool CreateShaderProgram();
    GLuint CompileShader(GLenum type, const char* source, const char* label);
    void DrawRaw(TextureHandle texture, const RenderRect& src, const RenderRect& dst, const RenderColor& color, float angle, bool flipX, bool flipY);

    void* LoadProc(const char* name);

    SDL_Window* m_Window = nullptr;
    SDL_GLContext m_Context = nullptr;
    bool m_Ready = false;
    int m_Width = 0;
    int m_Height = 0;
    std::string m_RendererName;

    GLuint m_Program = 0;
    GLuint m_VertexBuffer = 0;
    TextureHandle m_WhiteTexture;

    GLint m_ViewportUniform = -1;
    GLint m_ColorUniform = -1;
    GLint m_TextureUniform = -1;

    GLActiveTexture p_glActiveTexture = nullptr;
    GLAttachShader p_glAttachShader = nullptr;
    GLBindBuffer p_glBindBuffer = nullptr;
    GLBindTexture p_glBindTexture = nullptr;
    GLBlendFunc p_glBlendFunc = nullptr;
    GLBufferData p_glBufferData = nullptr;
    GLClear p_glClear = nullptr;
    GLClearColor p_glClearColor = nullptr;
    GLCompileShader p_glCompileShader = nullptr;
    GLCreateProgram p_glCreateProgram = nullptr;
    GLCreateShader p_glCreateShader = nullptr;
    GLDeleteBuffers p_glDeleteBuffers = nullptr;
    GLDeleteProgram p_glDeleteProgram = nullptr;
    GLDeleteShader p_glDeleteShader = nullptr;
    GLDeleteTextures p_glDeleteTextures = nullptr;
    GLDisable p_glDisable = nullptr;
    GLDrawArrays p_glDrawArrays = nullptr;
    GLEnable p_glEnable = nullptr;
    GLEnableVertexAttribArray p_glEnableVertexAttribArray = nullptr;
    GLGenBuffers p_glGenBuffers = nullptr;
    GLGenTextures p_glGenTextures = nullptr;
    GLGetIntegerv p_glGetIntegerv = nullptr;
    GLGetProgramInfoLog p_glGetProgramInfoLog = nullptr;
    GLGetProgramiv p_glGetProgramiv = nullptr;
    GLGetShaderInfoLog p_glGetShaderInfoLog = nullptr;
    GLGetShaderiv p_glGetShaderiv = nullptr;
    GLGetString p_glGetString = nullptr;
    GLGetUniformLocation p_glGetUniformLocation = nullptr;
    GLLinkProgram p_glLinkProgram = nullptr;
    GLPixelStorei p_glPixelStorei = nullptr;
    GLScissor p_glScissor = nullptr;
    GLShaderSource p_glShaderSource = nullptr;
    GLTexImage2D p_glTexImage2D = nullptr;
    GLTexParameteri p_glTexParameteri = nullptr;
    GLUniform1i p_glUniform1i = nullptr;
    GLUniform2f p_glUniform2f = nullptr;
    GLUniform4f p_glUniform4f = nullptr;
    GLUseProgram p_glUseProgram = nullptr;
    GLVertexAttribPointer p_glVertexAttribPointer = nullptr;
    GLViewport p_glViewport = nullptr;
};

} // namespace FNF
