#ifndef PYGAMEPP_H
#define PYGAMEPP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <chrono>
#include <iostream>


namespace PG {

    enum EventType {
        QUIT = SDL_QUIT,
        KEYDOWN = SDL_KEYDOWN,
        KEYUP = SDL_KEYUP,
        CONTROLLERBUTTONDOWN = SDL_CONTROLLERBUTTONDOWN
    };

    enum KeyCode {
        KEY_LEFT = SDLK_LEFT,
        KEY_RIGHT = SDLK_RIGHT,
        KEY_UP = SDLK_UP,
        KEY_DOWN = SDLK_DOWN,
        KEY_SPACE = SDLK_SPACE,
        KEY_R = SDLK_r,
        KEY_C = SDLK_c,
        KEY_W = SDL_SCANCODE_W,
        KEY_S = SDL_SCANCODE_S,
        KEY_UP_ARROW = SDL_SCANCODE_UP,
        KEY_DOWN_ARROW = SDL_SCANCODE_DOWN,
    };

    bool is_key_down(KeyCode key) {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[(int)key];
    }

    enum ButtonCode {
        BUTTON_A = SDL_CONTROLLER_BUTTON_A,
        BUTTON_B = SDL_CONTROLLER_BUTTON_B,
        DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
        DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
    };

    struct Event {
        Uint32 type;
        SDL_Keycode key; 
        Uint8 button;    
    };

    bool poll_event(Event& pg_event) {
        SDL_Event sdl_event;
        if (SDL_PollEvent(&sdl_event)) {
            pg_event.type = sdl_event.type;
            if (sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP) {
                pg_event.key = sdl_event.key.keysym.sym;
            }
            if (sdl_event.type == SDL_CONTROLLERBUTTONDOWN || sdl_event.type == SDL_CONTROLLERBUTTONUP) {
              pg_event.button = sdl_event.cbutton.button; 
            }
            return true;
        }
        return false;
    }

    struct Rect {
        float x, y, w, h;
        Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), w(w), h(h) {}

        bool colliderect(const Rect& other) const {
            return (x < other.x + other.w && x + w > other.x &&
                    y < other.y + other.h && y + h > other.y);
        }

        SDL_Rect to_sdl() const { return { (int)x, (int)y, (int)w, (int)h }; }
    };

    class Window {
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        bool running;

        std::chrono::high_resolution_clock::time_point start_time;
        int frame_count = 0;
        float last_fps_report = 0;

    public:
        Window(std::string title, int w, int h) : running(true) {
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
            TTF_Init();
        

            if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                std::cerr << "Mixer Error: " << Mix_GetError() << std::endl;
            }
            window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

            if (window) {
               std::cout << "[Pygame++] Window created: " << title << " (" << w << "x" << h << ")" << std::endl;  
               std::cout << "\033[95m[Pygame++] Thank you for using Pygame++! Keep building.\033[0m" << std::endl;
            }
        }

        ~Window() {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            Mix_CloseAudio();
            TTF_Quit();
            SDL_Quit();

            if (SDL_WasInit(0) == 0) {
                std::cout << "[Pygame++] Closed successfully: Safe Close verified." << std::endl;
            } else {
                std::cerr << "[Pygame++] Warning: Some subsystems did not close properly. " << std::endl;
            }
        }

        bool is_running() const { return running; }
        SDL_Renderer* get_renderer() { return renderer; }
        void stop() { running = false; }

        void clear(SDL_Color color = {0, 0, 0, 255}) {
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderClear(renderer);
        }

        void draw_rect(const Rect& rect, SDL_Color color) {
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_Rect r = rect.to_sdl();
            SDL_RenderFillRect(renderer, &r);
        }

        void log_performance() {
            frame_count++;
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = now - start_time;

            if (elapsed.count() - last_fps_report >= 1.0f) {
                float fps = frame_count / (elapsed.count() - last_fps_report);
                std::cout << "[Pygame++] Speed: " << fps << " FPS | Time: " << elapsed.count() << "s" << std::endl;

                frame_count = 0;
                last_fps_report = elapsed.count();
            }
        }

        void display() { 
            SDL_RenderPresent(renderer);
            log_performance();
         }

        void set_icon(std::string path) {
            SDL_Surface* icon_surface = IMG_Load(path.c_str());
            if (icon_surface) {
                std::cout << "[Pygame++] Loaded sprite/icon: " << path << std::endl;
                SDL_SetWindowIcon(window, icon_surface);
                SDL_FreeSurface(icon_surface);
            } else {
                std::cerr << "Failed to load icon: " << IMG_GetError() << std::endl;
            } 
        }

        static void delay(Uint32 ms) {
            SDL_Delay(ms);
        }
    };

    class Font {
    private:
       TTF_Font* font;
    public:
        Font(std::string path, int size) {
            font = TTF_OpenFont(path.c_str(), size);
            if (!font) {
                std::cerr << "[Pygame++] Failed to load font: " << TTF_GetError() << std::endl;
            } else {
                std::cout << "[Pygame++] Loaded font asset: " << path << std::endl;
            }
        }

        ~Font() {
            if (font) TTF_CloseFont(font);
        }

        void render(Window& app, std::string text, int x, int y, SDL_Color color = {255, 255, 255, 255}) {
            if (!font) return;

            SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(app.get_renderer(), surface);

            SDL_Rect dest = { x, y, surface->w, surface->h };
            SDL_RenderCopy(app.get_renderer(), texture, NULL, &dest);

            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    };

    class Sound {
    private:
        Mix_Chunk* chunk;
    public:
        Sound(std::string path) {
            chunk = Mix_LoadWAV(path.c_str());
            if (!chunk) {
                std::cerr << "[Pygame++] Failed to load sound: " << Mix_GetError() << std::endl;
            } else {
                std::cout << "[Pygame++] Loaded sound effect: " << path << std::endl;
            }
        }

        ~Sound() {
            if (chunk) Mix_FreeChunk(chunk);
        }

        void play() {
            if (chunk) Mix_PlayChannel(-1, chunk, 0);
        }

        void set_volume(int volume) {
            if (chunk) Mix_VolumeChunk(chunk, volume);
        }
    };

    class Music {
    private:
        Mix_Music* music;
    public:
        Music(std::string path) {
            music = Mix_LoadMUS(path.c_str());
            if (!music) {
                std::cerr << "[Pygame++] Failed to load music: " << Mix_GetError() << std::endl;
            } else {
                std::cout << "[Pygame++] Loaded music: " << path << std::endl;
            }
        }

        ~Music() {
            if (music) Mix_FreeMusic(music);
        }

        void play(int loops = -1) {
            if (music) Mix_PlayMusic(music, loops);
        }

        static void stop() { Mix_HaltMusic(); }
        static void pause() { Mix_PauseMusic(); }
        static void resume() { Mix_ResumeMusic(); }

        static void set_volume(int volume) {
            Mix_VolumeMusic(volume);
        }
    };

    class Sprite {
    private:
        SDL_Texture* texture;
        int width, height;
    public: 
        Sprite(Window& app, std::string path) : texture(nullptr), width(0), height(0) {
           SDL_Surface* surface = IMG_Load(path.c_str());
           if (!surface) {
               std::cerr << "[Pygame++] Failed to load sprite: " << IMG_GetError() << std::endl;
               return;
            }

            width = surface->w;
            height = surface->h;
            texture = SDL_CreateTextureFromSurface(app.get_renderer(), surface);
            SDL_FreeSurface(surface);

            if (texture) {
                std::cout << "[Pygame++] Loaded sprite texture: " << path << " (" << width << "x" << height << ")" << std::endl;
            }
        }

        ~Sprite() {
            if (texture) SDL_DestroyTexture(texture);
        }

        void draw(Window& app, int x, int y, int w = -1, int h = -1) {
            if (!texture) return;

            SDL_Rect dest = { x, y, (w == -1) ? width : w, (h == -1) ? height : h };
            SDL_RenderCopy(app.get_renderer(), texture, NULL, &dest);
        }

        void draw(Window& app, const Rect& rect) {
            if (!texture) return;
            SDL_Rect dest = rect.to_sdl();
            SDL_RenderCopy(app.get_renderer(), texture, NULL, &dest);
        }

        int get_width() const { return width; }
        int get_height() const { return height; }

    };

    class Controller {
    private:
       SDL_GameController* controller = nullptr;
    public:
        Controller() {
            for (int i = 0; i < SDL_NumJoysticks(); ++i) {
                if (SDL_IsGameController(i)) {
                    controller = SDL_GameControllerOpen(i);
                    if (controller) {
                       std::cout << "[Pygame++] Controller connected: " << SDL_GameControllerName(controller) << std::endl;
                       break;
                    }
                }
            }
        }

        ~Controller() {
            if (controller) {
               SDL_GameControllerClose(controller);
               std::cout << "[Pygame++] Controller disconnected." << std::endl;
            } 
        }

        bool is_connected() const { return controller != nullptr; }

        bool get_button(PG::ButtonCode button) {
            if (!controller) return false;
            return SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)button);
        }

        int get_axis(SDL_GameControllerAxis axis) {
            if (!controller) return false;
            return SDL_GameControllerGetAxis(controller, axis);
        }

    };
}

#endif