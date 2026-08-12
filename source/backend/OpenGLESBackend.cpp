/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * OpenGLESBackend Implementation
 */

#include "OpenGLESBackend.h"
#include "../backend/Logger.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION 0x821B
#endif

#ifndef GL_MINOR_VERSION
#define GL_MINOR_VERSION 0x821C
#endif

namespace FNF {

namespace {

constexpr const char* kVertexShader = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 uViewport;

out vec2 vTexCoord;

void main() {
    vec2 zeroToOne = aPosition / uViewport;
    vec2 clip = zeroToOne * 2.0 - 1.0;
    gl_Position = vec4(clip.x, -clip.y, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform vec4 uColor;

out vec4 oColor;

void main() {
    vec4 tex = texture(uTexture, vTexCoord);
    oColor = vec4(tex.rgb * uColor.rgb * uColor.a, tex.a * uColor.a);
}
)";

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
};

float Clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

void PremultiplyAlpha(SDL_Surface* surface) {
    if (!surface || !surface->pixels || !surface->format || surface->format->BytesPerPixel != 4) {
        return;
    }

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
        return;
    }

    for (int y = 0; y < surface->h; ++y) {
        auto* row = static_cast<std::uint8_t*>(surface->pixels) + (y * surface->pitch);
        for (int x = 0; x < surface->w; ++x) {
            std::uint8_t* px = row + (x * 4);
            const int alpha = px[3];
            px[0] = static_cast<std::uint8_t>((px[0] * alpha + 127) / 255);
            px[1] = static_cast<std::uint8_t>((px[1] * alpha + 127) / 255);
            px[2] = static_cast<std::uint8_t>((px[2] * alpha + 127) / 255);
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

} // namespace

bool OpenGLESBackend::Init(SDL_Window* window, int width, int height, bool vsync) {
    m_Window = window;
    m_Width = width;
    m_Height = height;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

    m_Context = SDL_GL_CreateContext(window);
    if (!m_Context) {
        Logger::Error(std::string("OpenGLESBackend: SDL_GL_CreateContext failed: ") + SDL_GetError());
        return false;
    }

    if (SDL_GL_MakeCurrent(window, m_Context) != 0) {
        Logger::Error(std::string("OpenGLESBackend: SDL_GL_MakeCurrent failed: ") + SDL_GetError());
        Shutdown();
        return false;
    }

    SDL_GL_SetSwapInterval(vsync ? 1 : 0);

    if (!LoadGL() || !CreateShaderProgram()) {
        Shutdown();
        return false;
    }

    GLint major = 0;
    GLint minor = 0;
    p_glGetIntegerv(GL_MAJOR_VERSION, &major);
    p_glGetIntegerv(GL_MINOR_VERSION, &minor);

    const GLubyte* renderer = p_glGetString(GL_RENDERER);
    const GLubyte* version = p_glGetString(GL_VERSION);
    m_RendererName = reinterpret_cast<const char*>(renderer ? renderer : reinterpret_cast<const GLubyte*>("unknown"));

    Logger::Info("OpenGLESBackend initialized");
    Logger::Info("GL renderer: " + m_RendererName);
    Logger::Info("GL version: " + std::string(reinterpret_cast<const char*>(version ? version : reinterpret_cast<const GLubyte*>("unknown"))));
    Logger::Info("GL ES reported version: " + std::to_string(major) + "." + std::to_string(minor));

    p_glGenBuffers(1, &m_VertexBuffer);
    p_glEnable(GL_BLEND);
    p_glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    Resize(width, height);

    std::uint32_t pixel = 0xffffffffu;
    p_glGenTextures(1, &m_WhiteTexture.id);
    m_WhiteTexture.width = 1;
    m_WhiteTexture.height = 1;
    p_glBindTexture(GL_TEXTURE_2D, m_WhiteTexture.id);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);

    m_Ready = true;
    return true;
}

void OpenGLESBackend::Shutdown() {
    if (m_WhiteTexture.IsValid()) {
        DestroyTexture(m_WhiteTexture);
    }

    if (m_VertexBuffer != 0 && p_glDeleteBuffers) {
        p_glDeleteBuffers(1, &m_VertexBuffer);
        m_VertexBuffer = 0;
    }

    if (m_Program != 0 && p_glDeleteProgram) {
        p_glDeleteProgram(m_Program);
        m_Program = 0;
    }

    if (m_Context) {
        SDL_GL_DeleteContext(m_Context);
        m_Context = nullptr;
    }

    m_Ready = false;
}

void OpenGLESBackend::BeginFrame(const RenderColor& clearColor) {
    if (!m_Ready) return;
    p_glClearColor(Clamp01(clearColor.r), Clamp01(clearColor.g), Clamp01(clearColor.b), Clamp01(clearColor.a));
    p_glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLESBackend::EndFrame() {
    if (!m_Ready || !m_Window) return;
    SDL_GL_SwapWindow(m_Window);
}

void OpenGLESBackend::Resize(int width, int height) {
    m_Width = std::max(1, width);
    m_Height = std::max(1, height);
    if (p_glViewport) {
        p_glViewport(0, 0, m_Width, m_Height);
    }
}

TextureHandle OpenGLESBackend::CreateTextureFromSurface(SDL_Surface* surface, const std::string& debugName) {
    TextureHandle handle;
    if (!m_Ready || !surface) return handle;

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) {
        Logger::Error("OpenGLESBackend: surface conversion failed for '" + debugName + "': " + SDL_GetError());
        return handle;
    }

    p_glGenTextures(1, &handle.id);
    handle.width = converted->w;
    handle.height = converted->h;

    p_glBindTexture(GL_TEXTURE_2D, handle.id);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    p_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    p_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    PremultiplyAlpha(converted);
    p_glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        converted->w,
        converted->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        converted->pixels
    );

    SDL_FreeSurface(converted);
    return handle;
}

void OpenGLESBackend::DestroyTexture(TextureHandle& texture) {
    if (texture.id != 0 && p_glDeleteTextures) {
        const GLuint id = texture.id;
        p_glDeleteTextures(1, &id);
    }
    texture = {};
}

void OpenGLESBackend::DrawTexture(const SpriteDrawCommand& cmd) {
    if (!cmd.texture.IsValid()) return;
    DrawRaw(cmd.texture, cmd.source, cmd.dest, cmd.color, cmd.angle, cmd.flipX, cmd.flipY);
}

void OpenGLESBackend::FillRect(const RenderRect& rect, const RenderColor& color) {
    if (!m_WhiteTexture.IsValid()) return;
    DrawRaw(m_WhiteTexture, {0.0f, 0.0f, 1.0f, 1.0f}, rect, color, 0.0f, false, false);
}

void OpenGLESBackend::SetClipRect(const RenderRect& rect) {
    if (!m_Ready) return;

    const int x = static_cast<int>(std::floor(rect.x));
    const int y = static_cast<int>(std::floor(static_cast<float>(m_Height) - rect.y - rect.h));
    const int w = static_cast<int>(std::ceil(rect.w));
    const int h = static_cast<int>(std::ceil(rect.h));

    p_glEnable(GL_SCISSOR_TEST);
    p_glScissor(x, y, std::max(0, w), std::max(0, h));
}

void OpenGLESBackend::ClearClipRect() {
    if (!m_Ready) return;
    p_glDisable(GL_SCISSOR_TEST);
}

bool OpenGLESBackend::LoadGL() {
#define LOAD_GL(name) \
    do { \
        p_##name = reinterpret_cast<decltype(p_##name)>(LoadProc(#name)); \
        if (!p_##name) { \
            Logger::Error("OpenGLESBackend: missing GL proc " #name); \
            return false; \
        } \
    } while (false)

    LOAD_GL(glActiveTexture);
    LOAD_GL(glAttachShader);
    LOAD_GL(glBindBuffer);
    LOAD_GL(glBindTexture);
    LOAD_GL(glBlendFunc);
    LOAD_GL(glBufferData);
    LOAD_GL(glClear);
    LOAD_GL(glClearColor);
    LOAD_GL(glCompileShader);
    LOAD_GL(glCreateProgram);
    LOAD_GL(glCreateShader);
    LOAD_GL(glDeleteBuffers);
    LOAD_GL(glDeleteProgram);
    LOAD_GL(glDeleteShader);
    LOAD_GL(glDeleteTextures);
    LOAD_GL(glDisable);
    LOAD_GL(glDrawArrays);
    LOAD_GL(glEnable);
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glGenBuffers);
    LOAD_GL(glGenTextures);
    LOAD_GL(glGetIntegerv);
    LOAD_GL(glGetProgramInfoLog);
    LOAD_GL(glGetProgramiv);
    LOAD_GL(glGetShaderInfoLog);
    LOAD_GL(glGetShaderiv);
    LOAD_GL(glGetString);
    LOAD_GL(glGetUniformLocation);
    LOAD_GL(glLinkProgram);
    LOAD_GL(glPixelStorei);
    LOAD_GL(glScissor);
    LOAD_GL(glShaderSource);
    LOAD_GL(glTexImage2D);
    LOAD_GL(glTexParameteri);
    LOAD_GL(glUniform1i);
    LOAD_GL(glUniform2f);
    LOAD_GL(glUniform4f);
    LOAD_GL(glUseProgram);
    LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glViewport);

#undef LOAD_GL
    return true;
}

bool OpenGLESBackend::CreateShaderProgram() {
    const GLuint vertex = CompileShader(GL_VERTEX_SHADER, kVertexShader, "sprite vertex");
    if (vertex == 0) return false;

    const GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader, "sprite fragment");
    if (fragment == 0) {
        p_glDeleteShader(vertex);
        return false;
    }

    m_Program = p_glCreateProgram();
    p_glAttachShader(m_Program, vertex);
    p_glAttachShader(m_Program, fragment);
    p_glLinkProgram(m_Program);

    p_glDeleteShader(vertex);
    p_glDeleteShader(fragment);

    GLint linked = 0;
    p_glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::array<char, 1024> log = {};
        p_glGetProgramInfoLog(m_Program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        Logger::Error(std::string("OpenGLESBackend: shader link failed: ") + log.data());
        return false;
    }

    m_ViewportUniform = p_glGetUniformLocation(m_Program, "uViewport");
    m_ColorUniform = p_glGetUniformLocation(m_Program, "uColor");
    m_TextureUniform = p_glGetUniformLocation(m_Program, "uTexture");
    return true;
}

GLuint OpenGLESBackend::CompileShader(GLenum type, const char* source, const char* label) {
    const GLuint shader = p_glCreateShader(type);
    p_glShaderSource(shader, 1, &source, nullptr);
    p_glCompileShader(shader);

    GLint compiled = 0;
    p_glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::array<char, 1024> log = {};
        p_glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        Logger::Error(std::string("OpenGLESBackend: ") + label + " shader compile failed: " + log.data());
        p_glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void OpenGLESBackend::DrawRaw(TextureHandle texture, const RenderRect& src, const RenderRect& dst, const RenderColor& color, float angle, bool flipX, bool flipY) {
    if (!m_Ready || !texture.IsValid() || dst.w == 0.0f || dst.h == 0.0f) return;

    float u0 = src.x / static_cast<float>(texture.width);
    float v0 = src.y / static_cast<float>(texture.height);
    float u1 = (src.x + src.w) / static_cast<float>(texture.width);
    float v1 = (src.y + src.h) / static_cast<float>(texture.height);

    if (flipX) std::swap(u0, u1);
    if (flipY) std::swap(v0, v1);

    const float cx = dst.x + dst.w * 0.5f;
    const float cy = dst.y + dst.h * 0.5f;
    const float radians = angle * 3.1415926535f / 180.0f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);

    auto point = [&](float localX, float localY, float u, float v) {
        const float rx = localX * c - localY * s;
        const float ry = localX * s + localY * c;
        return Vertex{cx + rx, cy + ry, u, v};
    };

    const float hw = dst.w * 0.5f;
    const float hh = dst.h * 0.5f;

    const std::array<Vertex, 6> vertices = {
        point(-hw, -hh, u0, v0),
        point( hw, -hh, u1, v0),
        point( hw,  hh, u1, v1),
        point(-hw, -hh, u0, v0),
        point( hw,  hh, u1, v1),
        point(-hw,  hh, u0, v1)
    };

    p_glUseProgram(m_Program);
    p_glUniform2f(m_ViewportUniform, static_cast<float>(m_Width), static_cast<float>(m_Height));
    p_glUniform4f(m_ColorUniform, Clamp01(color.r), Clamp01(color.g), Clamp01(color.b), Clamp01(color.a));
    p_glUniform1i(m_TextureUniform, 0);

    p_glActiveTexture(GL_TEXTURE0);
    p_glBindTexture(GL_TEXTURE_2D, texture.id);

    p_glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    p_glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), vertices.data(), GL_STREAM_DRAW);
    p_glEnableVertexAttribArray(0);
    p_glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(0));
    p_glEnableVertexAttribArray(1);
    p_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(sizeof(float) * 2));
    p_glDrawArrays(GL_TRIANGLES, 0, 6);
}

void* OpenGLESBackend::LoadProc(const char* name) {
    return SDL_GL_GetProcAddress(name);
}

} // namespace FNF
