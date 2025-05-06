#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <direct.h>
#include <string>
#include <algorithm> // Để dùng std::max

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SPEED 20
#define BULLET_SPEED 10
#define ENEMY_SPEED 2
using namespace std;

// Struct đại diện cho đối tượng trong game
struct GameObject {
    SDL_Rect rect;
    SDL_Texture* texture;
};

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* playerTexture = nullptr;
SDL_Texture* bulletTexture = nullptr;
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* backgroundTexture = nullptr;
GameObject player;
vector<GameObject> bullets;
vector<GameObject> enemies;
bool running = true;

// Biến điểm số và high score
int score = 0;
int highScore = 0; // Biến lưu điểm cao nhất
SDL_Texture* scoreTexture = nullptr;
SDL_Texture* highScoreTexture = nullptr;

// Trạng thái trò chơi
enum GameState { MENU, PLAYING, GAME_OVER };
GameState currentState = MENU;
int selectedOption = 0;
const char* menuOptions[] = {"Start game", "Exit"};
const char* gameOverOptions[] = {"Restart", "Exit"};
const int numOptions = 2;
SDL_Texture* normalTextures[2];
SDL_Texture* selectedTextures[2];
SDL_Texture* gameOverNormalTextures[2];
SDL_Texture* gameOverSelectedTextures[2];
SDL_Texture* titleTexture = nullptr;
SDL_Texture* gameOverTitleTexture = nullptr;
TTF_Font* font = nullptr;

// Hàm tải texture từ file
SDL_Texture* LoadTexture(const char* file) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        cout << "Khong the tao texture: " << SDL_GetError() << endl;
    }
    SDL_FreeSurface(surface);
    return texture;
}

// Hàm tạo texture từ văn bản
SDL_Texture* CreateTextTexture(const string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        cout << "Khong the render van ban: " << text << " - " << TTF_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Hàm cập nhật texture điểm số
void UpdateScoreTexture() {
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
    }
    string scoreText = "Score: " + to_string(score);
    scoreTexture = CreateTextTexture(scoreText, {255, 255, 255});
}

// Hàm cập nhật texture high score
void UpdateHighScoreTexture() {
    if (highScoreTexture) {
        SDL_DestroyTexture(highScoreTexture);
    }
    string highScoreText = "High Score: " + to_string(highScore);
    highScoreTexture = CreateTextTexture(highScoreText, {255, 255, 255});
}

// Hàm tạo kẻ địch ngẫu nhiên
void SpawnEnemy() {
    GameObject enemy;
    enemy.rect = {rand() % (SCREEN_WIDTH - 50), 0, 50, 50};
    enemy.texture = enemyTexture;
    enemies.push_back(enemy);
}

// Hàm cập nhật logic game
void Update() {
    // Cập nhật vị trí đạn và xóa đạn ra ngoài màn hình
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->rect.y -= BULLET_SPEED;
        if (it->rect.y < 0) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }

    // Cập nhật vị trí kẻ địch và xóa kẻ địch ra ngoài màn hình
    for (auto it = enemies.begin(); it != enemies.end();) {
        it->rect.y += ENEMY_SPEED;
        if (it->rect.y > SCREEN_HEIGHT) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }

    // Kiểm tra va chạm giữa đạn và kẻ địch
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
        bool bulletHit = false;
        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
            if (SDL_HasIntersection(&bulletIt->rect, &enemyIt->rect)) {
                enemyIt = enemies.erase(enemyIt);
                bulletHit = true;
                score += 10; // Cộng 10 điểm khi tiêu diệt kẻ địch
                UpdateScoreTexture(); // Cập nhật texture điểm số
                break;
            } else {
                ++enemyIt;
            }
        }
        if (bulletHit) {
            bulletIt = bullets.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }

    // Kiểm tra va chạm giữa phi thuyền và kẻ địch
    for (size_t i = 0; i < enemies.size(); i++) {
        if (SDL_HasIntersection(&player.rect, &enemies[i].rect)) {
            highScore = max(highScore, score); // Cập nhật high score
            UpdateHighScoreTexture(); // Cập nhật texture high score
            currentState = GAME_OVER;
            bullets.clear();
            enemies.clear();
            player.rect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
            selectedOption = 0; // Đặt lại lựa chọn cho menu Game Over
        }
    }
}

// Hàm vẽ đối tượng lên màn hình
void Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    if (player.texture) {
        SDL_RenderCopy(renderer, player.texture, NULL, &player.rect);
    }

    for (auto& bullet : bullets) {
        if (bullet.texture) {
            SDL_RenderCopy(renderer, bullet.texture, NULL, &bullet.rect);
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.texture) {
            SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect); // Sửa lỗi: dùng enemy.rect thay vì bullet.rect
        }
    }

    // Vẽ điểm số
    if (scoreTexture) {
        int texW, texH;
        SDL_QueryTexture(scoreTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {10, 10, texW, texH}; // Góc trên bên trái
        SDL_RenderCopy(renderer, scoreTexture, NULL, &dst);
    }

    SDL_RenderPresent(renderer);
}

// Hàm vẽ menu chính
void RenderMenu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    // Vẽ tiêu đề
    if (titleTexture) {
        int texW, texH;
        SDL_QueryTexture(titleTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 100, texW, texH};
        SDL_RenderCopy(renderer, titleTexture, NULL, &dst);
    }

    // Vẽ high score
    if (highScoreTexture) {
        int texW, texH;
        SDL_QueryTexture(highScoreTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 150, texW, texH};
        SDL_RenderCopy(renderer, highScoreTexture, NULL, &dst);
    }

    // Vẽ các tùy chọn menu
    for (int i = 0; i < numOptions; i++) {
        SDL_Texture* tex = (i == selectedOption) ? selectedTextures[i] : normalTextures[i];
        int texW, texH;
        SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 200 + i * 50, texW, texH};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }

    SDL_RenderPresent(renderer);
}

// Hàm vẽ màn hình Game Over
void RenderGameOver() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    // Vẽ tiêu đề "Game Over"
    if (gameOverTitleTexture) {
        int texW, texH;
        SDL_QueryTexture(gameOverTitleTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 100, texW, texH};
        SDL_RenderCopy(renderer, gameOverTitleTexture, NULL, &dst);
    }

    // Vẽ điểm số
    if (scoreTexture) {
        int texW, texH;
        SDL_QueryTexture(scoreTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 150, texW, texH};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &dst);
    }

    // Vẽ high score
    if (highScoreTexture) {
        int texW, texH;
        SDL_QueryTexture(highScoreTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 180, texW, texH};
        SDL_RenderCopy(renderer, highScoreTexture, NULL, &dst);
    }

    // Vẽ các tùy chọn Restart/Exit
    for (int i = 0; i < numOptions; i++) {
        SDL_Texture* tex = (i == selectedOption) ? gameOverSelectedTextures[i] : gameOverNormalTextures[i];
        int texW, texH;
        SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 230 + i * 50, texW, texH};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }

    SDL_RenderPresent(renderer);
}

// Hàm xử lý input cho trạng thái chơi
void HandleInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_a:
            case SDLK_LEFT:
                if (player.rect.x > 0) {
                    player.rect.x -= PLAYER_SPEED;
                }
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                if (player.rect.x < SCREEN_WIDTH - player.rect.w) {
                    player.rect.x += PLAYER_SPEED;
                }
                break;
            case SDLK_SPACE:
                GameObject bullet = {{player.rect.x + 20, player.rect.y, 10, 20}, bulletTexture};
                bullets.push_back(bullet);
                break;
        }
    }
}

// Hàm xử lý input cho menu chính
void HandleMenuInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + numOptions) % numOptions;
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % numOptions;
                break;
            case SDLK_RETURN:
                if (selectedOption == 0) {
                    currentState = PLAYING;
                    bullets.clear();
                    enemies.clear();
                    player.rect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
                    score = 0; // Đặt lại điểm số
                    UpdateScoreTexture();
                } else if (selectedOption == 1) {
                    running = false;
                }
                break;
        }
    }
}

// Hàm xử lý input cho màn hình Game Over
void HandleGameOverInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + numOptions) % numOptions;
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % numOptions;
                break;
            case SDLK_RETURN:
                if (selectedOption == 0) { // Restart
                    currentState = PLAYING;
                    bullets.clear();
                    enemies.clear();
                    player.rect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
                    score = 0; // Đặt lại điểm số
                    UpdateScoreTexture();
                } else if (selectedOption == 1) { // Exit
                    running = false;
                }
                break;
        }
    }
}

// Hàm chính
int main(int argc, char* argv[]) {
    srand(time(0));

    // In thư mục làm việc để gỡ lỗi
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    cout << "Thu muc lam viec: " << cwd << endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "Khoi tao SDL that bai: " << SDL_GetError() << endl;
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        cout << "Khoi tao IMG that bai: " << IMG_GetError() << endl;
        return 1;
    }
    if (TTF_Init() == -1) {
        cout << "Khoi tao TTF that bai: " << TTF_GetError() << endl;
        return 1;
    }

    window = SDL_CreateWindow("Space Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        cout << "Tao cua so that bai: " << SDL_GetError() << endl;
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Tao renderer that bai: " << SDL_GetError() << endl;
        return 1;
    }

    // Tải phông chữ
    font = TTF_OpenFont("D:/Space shooter/bin/Debug/assets/font.ttf", 24);
    if (!font) {
        cout << "Khong the tai phong chu: " << TTF_GetError() << endl;
        cout << "Kiem tra tep: D:/Space shooter/bin/Debug/assets/font.ttf" << endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Tạo texture cho menu chính
    titleTexture = CreateTextTexture("Space Shooter", {255, 255, 255});
    for (int i = 0; i < numOptions; i++) {
        normalTextures[i] = CreateTextTexture(menuOptions[i], {255, 255, 255});
        selectedTextures[i] = CreateTextTexture(menuOptions[i], {255, 0, 0});
    }

    // Tạo texture cho màn hình Game Over
    gameOverTitleTexture = CreateTextTexture("Game Over", {255, 0, 0});
    for (int i = 0; i < numOptions; i++) {
        gameOverNormalTextures[i] = CreateTextTexture(gameOverOptions[i], {255, 255, 255});
        gameOverSelectedTextures[i] = CreateTextTexture(gameOverOptions[i], {255, 0, 0});
    }

    // Khởi tạo texture điểm số và high score
    UpdateScoreTexture();
    UpdateHighScoreTexture();

    // Tải textures
    playerTexture = LoadTexture("assets/player.png");
    bulletTexture = LoadTexture("assets/bullet.png");
    enemyTexture = LoadTexture("assets/enemy.png");
    backgroundTexture = LoadTexture("assets/background.png");

    // Kiểm tra textures
    if (!playerTexture || !bulletTexture || !enemyTexture || !backgroundTexture ||
        !titleTexture || !normalTextures[0] || !normalTextures[1] ||
        !selectedTextures[0] || !selectedTextures[1] || !scoreTexture ||
        !gameOverTitleTexture || !gameOverNormalTextures[0] || !gameOverNormalTextures[1] ||
        !gameOverSelectedTextures[0] || !gameOverSelectedTextures[1] || !highScoreTexture) {
        playerTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/player.png");
        bulletTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/bullet.png");
        enemyTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/enemy.png");
        backgroundTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/background.png");

        if (!playerTexture || !bulletTexture || !enemyTexture || !backgroundTexture) {
            cout << "Van that bai voi duong dan tuyet doi. Vui long kiem tra tep tai D:/Space shooter/bin/Debug/assets/" << endl;
            for (int i = 0; i < numOptions; i++) {
                SDL_DestroyTexture(normalTextures[i]);
                SDL_DestroyTexture(selectedTextures[i]);
                SDL_DestroyTexture(gameOverNormalTextures[i]);
                SDL_DestroyTexture(gameOverSelectedTextures[i]);
            }
            SDL_DestroyTexture(titleTexture);
            SDL_DestroyTexture(gameOverTitleTexture);
            SDL_DestroyTexture(scoreTexture);
            SDL_DestroyTexture(highScoreTexture);
            TTF_CloseFont(font);
            TTF_Quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return 1;
        }
    }

    // Thiết lập nhân vật
    player.rect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
    player.texture = playerTexture;

    SDL_Event e;
    int spawnTimer = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (currentState == MENU) {
                HandleMenuInput(e);
            } else if (currentState == PLAYING) {
                HandleInput(e);
            } else if (currentState == GAME_OVER) {
                HandleGameOverInput(e);
            }
        }

        if (currentState == MENU) {
            RenderMenu();
        } else if (currentState == PLAYING) {
            Update();
            Render();
            if (++spawnTimer > 60) {
                SpawnEnemy();
                spawnTimer = 0;
            }
        } else if (currentState == GAME_OVER) {
            RenderGameOver();
        }
        SDL_Delay(16);
    }

    // Dọn dẹp bộ nhớ
    for (int i = 0; i < numOptions; i++) {
        SDL_DestroyTexture(normalTextures[i]);
        SDL_DestroyTexture(selectedTextures[i]);
        SDL_DestroyTexture(gameOverNormalTextures[i]);
        SDL_DestroyTexture(gameOverSelectedTextures[i]);
    }
    SDL_DestroyTexture(titleTexture);
    SDL_DestroyTexture(gameOverTitleTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(highScoreTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
