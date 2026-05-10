#include "PygamePP.h"
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


void draw_game_over(PG::Window& app, PG::Font& font, int score, PG::Sprite& bg) {
    for (int i = 30; i > 0; i--) {
        app.clear();

        int offsetX = (rand() % i) - (i / 2);
        int offsetY = (rand() % i) - (i / 2); 
        bg.draw(app, 0, 0, 400, 600);
        font.render(app, "GAME OVER", 110 + offsetX, 250 + offsetY, {255, 0, 0, 255});
        font.render(app, "Score: " + std::to_string(score), 140 + offsetX, 300 + offsetY, {0, 0, 0, 255});
        
        app.display();
        
        #ifdef __EMSCRIPTEN__
        emscripten_sleep(16);
        #else
        PG::Window::delay(16);
        #endif

    }

    app.clear();

    bg.draw(app, 0, 0, 400, 600);
    font.render(app, "GAME OVER", 110, 250, {255, 0, 0, 255});
    font.render(app, "Score: " + std::to_string(score), 140, 300, {0, 0, 0, 255});
    font.render(app, "Press SPACE to Restart", 60, 350, {50, 50, 50, 255});
    app.display();
}

int main() {
    srand(time(0));

   
    PG::Window app("Sheep Jump", 400, 600);
    app.set_icon("icon.jpg");

    PG::Music bg_music("sheep.wav");
    bg_music.play(-1);
    bg_music.set_volume(50);

    PG::Sound baa_sound("baa.wav");
    baa_sound.set_volume(55);

    PG::Sprite bg(app, "sky.jpg");
    PG::Sprite bg_night(app, "night.jpg");
    PG::Sprite sheep_img(app, "sheep_idle.png");
    PG::Sprite plat_img(app, "platform.png");
    PG::Sprite cloud_img(app, "cloud.png");


    PG::Font font("Ubuntu-Regular.ttf", 28);
    PG::Rect player(180, 400, 50, 50);

    PG::Controller gamepad;

    std::vector<PG::Rect> clouds;
    std::vector<PG::Rect> platforms;


    for (int i = 0; i < 4; i++) {
       clouds.push_back(PG::Rect(rand() % 300, rand() % 600, 200, 100));
    }

    
    for(int i = 0; i < 8; i++) {
        platforms.push_back(PG::Rect(rand() % 320, i * 80, 64, 32));
    }

    float dy = 0;
    int score = 0;
    int high_score = 0;
    bool game_over = 0;
    bool needs_shake = false;

    while (app.is_running()) {
        PG::Event event;
        while (PG::poll_event(event)) {
            if (event.type == PG::QUIT) app.stop();

            if (event.type == PG::KEYDOWN || event.type == PG::CONTROLLERBUTTONDOWN) {

            if (game_over && (event.key == PG::KEY_SPACE || event.button == PG::BUTTON_A)) { 
                    game_over = false;
                    score = 0;
                    dy = 0;
                    player.y = 400;
                    player.x = 180;
                    bg_music.play(-1);
                    platforms.clear();
                    for(int i = 0; i < 8; i++) 
                        platforms.push_back(PG::Rect(rand() % 320, i * 80, 64, 32));
                }

                else if (!game_over) {
                    if (event.key == PG::KEY_LEFT) player.x -= 10;
                    if (event.key == PG::KEY_RIGHT) player.x += 10;

                    if (gamepad.get_button(PG::DPAD_LEFT)) player.x -= 10;
                    if (gamepad.get_button(PG::DPAD_RIGHT)) player.x += 10;

                    if (gamepad.get_axis(SDL_CONTROLLER_AXIS_LEFTX) < -8000) player.x -= 10;
                    if (gamepad.get_axis(SDL_CONTROLLER_AXIS_LEFTX) > 8000) player.x += 10;
                }
            }
        }

        if (game_over) {
           if (needs_shake) {
              draw_game_over(app, font, score, (score < 50 ? bg : bg_night));
               needs_shake = false;
            } else {
              PG::Sprite& current_bg = (score < 50 ? bg : bg_night);
              app.clear();
              current_bg.draw(app, 0, 0, 400, 600);
              font.render(app, "GAME OVER", 110, 250, {255, 0, 0, 255});
              font.render(app, "Score: " + std::to_string(score), 140, 300, {0, 0, 0, 255});
              font.render(app, "Press SPACE to Restart", 60, 350, {50, 50, 50, 255});
              app.display(); 
            }

            continue;
        }

        if (player.x > 400) player.x = 0;
        if (player.x < -50) player.x = 400;

        dy += 0.25f;
        player.y += dy;

        for (auto& p : platforms) {
            if ((player.x + 40 > p.x) && (player.x + 10 < p.x + p.w) &&
                (player.y + 50 > p.y) && (player.y + 50 < p.y + p.h) &&
                (dy > 0)) {
                dy = -14;
                
                if (rand() % 5 == 0) {
                    baa_sound.play();
                }
            }
        }

        if (player.y < 250) {
            float diff = 250 - player.y;
            player.y = 250;

            for (auto& c : clouds) {
                c.y += diff * 0.5f;
                if (c.y > 600) {
                    c.y = -100;
                    c.x = rand() % 300;
                }
            }

            for (auto& p : platforms) {
                p.y += diff;
                if (p.y > 600) {
                    p.y = 0;
                    p.x = rand() % 320;
                    score++;
                }
            }
        }

        if (player.y > 600) {
            if (!game_over) {
                bg_music.stop();
                baa_sound.play();
                needs_shake = true;
            }
            game_over = true;
            if (score > high_score) high_score = score;
        }

        app.clear();
        
        if ((score / 50) % 2 == 0) {
            bg.draw(app, 0, 0, 400, 600);
        } else {
            bg_night.draw(app, 0, 0, 400, 600);
        }

        for (auto& c : clouds) cloud_img.draw(app, c);
        for (auto& p : platforms) plat_img.draw(app, p);

        sheep_img.draw(app, player);

        std::string scoreStr = "Score: " + std::to_string(score);
        font.render(app, scoreStr, 10, 10, {0,0,0,255});

        app.display();
        #ifdef __EMSCRIPTEN__
        emscripten_sleep(16);
        #else
        PG::Window::delay(16);
        #endif
    }

    return 0;
}
