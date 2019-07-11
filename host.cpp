#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>
#include <GL/gl.h>

extern "C" {
  #include <airkit-api/api.h>

  struct AKHostSDL {
    SDL_GLContext glContext = nullptr;
    SDL_Window* win = nullptr;
    bool running = true;

    AKBackendType init(AKApi* api) {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

      uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

      static const int kStencilBits = 8;
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, kStencilBits);
      SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        throw std::runtime_error(std::string(SDL_GetError()));
      }

      win = SDL_CreateWindow("AirKit Application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, windowFlags);
      if (!win) {
        throw std::runtime_error(std::string(SDL_GetError()));
      }

      glContext = SDL_GL_CreateContext(win);
      if (!glContext) {
        throw std::runtime_error(std::string(SDL_GetError()));
      }

      if (SDL_GL_MakeCurrent(win, glContext) != 0) {
        throw std::runtime_error(std::string(SDL_GetError()));
      }
      
      uint32_t windowFormat = SDL_GetWindowPixelFormat(win);
      int contextType;
      SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &contextType);

      return AK_BACKEND_TYPE_GL3;
    }

    void shutdown(AKApi* api) {
      SDL_GL_DeleteContext(glContext);
      SDL_DestroyWindow(win);
      SDL_Quit();
      free(api);
    }

    void prepare(AKApi* api) {
      glClearColor(1, 1, 1, 1);
      glClearStencil(0);
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void flush(AKApi* api) {
      SDL_GL_SwapWindow(win);
    }

    void title(AKApi* api, const char* title) {
      SDL_SetWindowTitle(win, title);
    }

    void run(AKApi* api) {
      while (running) {
        if (!api->cb_work(api))
          break;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
          switch (e.type) {
            case SDL_WINDOWEVENT:
              {
                SDL_WindowEvent* we = (SDL_WindowEvent*) &e;
                switch (we->event) {
                  case SDL_WINDOWEVENT_RESIZED:
                    api->cb_resize(api, we->data1, we->data2);
                    break;
                  default:
                    break;
                }
              }
              break;
            case SDL_QUIT:
              api->cb_quit_requested(api);
              break;
            default:
              break;
          }
        }
      }
    }

    void quit(AKApi* api) {
      running = false;
    }
  };

  static AKBackendType aksdl_init(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->init(api);
  }

  static void aksdl_shutdown(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->shutdown(api);
  }

  static void aksdl_prepare(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->prepare(api);
  }

  static void aksdl_flush(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->flush(api);
  }

  static void aksdl_title(AKApi* api, const char* title) {
    return ((AKHostSDL*) api->hostdata)->title(api, title);
  }

  static void aksdl_run(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->run(api);
  }

  static void aksdl_quit(AKApi* api) {
    return ((AKHostSDL*) api->hostdata)->quit(api);
  }

  void ak_host_init_api(AKApi* api) {
    AKHostSDL* h = new AKHostSDL();
    api->hostdata = (void*) h;

    api->mt_init = aksdl_init;
    api->mt_shutdown = aksdl_shutdown;
    api->mt_prepare = aksdl_prepare;
    api->mt_flush = aksdl_flush;
    api->mt_title = aksdl_title;
    api->mt_run = aksdl_run;
    api->mt_quit = aksdl_quit;
  }

}