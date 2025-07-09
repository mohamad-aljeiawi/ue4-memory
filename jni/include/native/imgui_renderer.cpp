#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "native/imgui_renderer.h"
#include "language/arabic.h"

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static EGLContext egl_context;
static ANativeWindow *g_window = nullptr;
static int g_width = 0, g_height = 0;
static bool g_screenshot_mode = true;

void Renderer::Init(ANativeWindow *window, int width, int height)
{
    g_window = window;
    g_width = width;
    g_height = height;

    const EGLint config_attr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE};

    const EGLint context_attr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE};

    EGLConfig egl_config;
    EGLint num_configs;
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(egl_display, nullptr, nullptr);
    eglChooseConfig(egl_display, config_attr, &egl_config, 1, &num_configs);
    egl_surface = eglCreateWindowSurface(egl_display, egl_config, window, nullptr);
    egl_context = eglCreateContext(egl_display, egl_config, nullptr, context_attr);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplAndroid_Init(window);
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    static const ImWchar ar_ranges[] = {
        0x0020,
        0x00FF, // Basic Latin + Latin Supplement
        0x0400,
        0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0,
        0x2DFF, // Cyrillic Extended-A
        0xA640,
        0xA69F, // Cyrillic Extended-B
        0xE000,
        0xE226, // icons
        0x2010,
        0x205E, // Punctuations
        0x0600,
        0x06FF, // Arabic
        0xFE00,
        0xFEFF,
        0,
    };

    ImFont *arabic = io.Fonts->AddFontFromMemoryCompressedTTF(font_arabic_data, font_arabic_size, 24.0f, NULL, ar_ranges);
    io.FontDefault = arabic;
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(3.0f);
    style.WindowRounding = 5.0f;
}

void Renderer::StartFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
}

void Renderer::EndFrame()
{
    ImGui::Render();

    int display_w = g_width > 0 ? g_width : 1920;
    int display_h = g_height > 0 ? g_height : 1080;

    glViewport(0, 0, display_w, display_h);

    if (g_screenshot_mode)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(egl_display, egl_surface);
}

void Renderer::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);
}
