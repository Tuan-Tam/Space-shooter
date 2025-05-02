#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <direct.h> // Để dùng getcwd

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SPEED 8
#define BULLET_SPEED 10
#define ENEMY_SPEED 3
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

// Trạng thái trò chơi
enum GameState { MENU, PLAYING };
GameState currentState = MENU;
int selectedOption = 0;
const char* menuOptions[] = {"Start game", "Exit"};
const int numOptions = 2;
SDL_Texture* normalTextures[2];
SDL_Texture* selectedTextures[2];
SDL_Texture* titleTexture = nullptr;
TTF_Font* font = nullptr;

// Hàm tải texture từ file
SDL_Texture* LoadTexture(const char* file) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        cout << "Không thể tải hình ảnh: " << file << " - " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        cout << "Không thể tạo texture: " << SDL_GetError() << endl;
    }
    SDL_FreeSurface(surface);
    return texture;
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
            currentState = MENU;
            bullets.clear();
            enemies.clear();
            player.rect = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 50};
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
            SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
        }
    }

    SDL_RenderPresent(renderer);
}

// Hàm vẽ menu
void RenderMenu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    if (titleTexture) {
        int texW, texH;
        SDL_QueryTexture(titleTexture, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 100, texW, texH};
        SDL_RenderCopy(renderer, titleTexture, NULL, &dst);
    }

    for (int i = 0; i < numOptions; i++) {
        SDL_Texture* tex = (i == selectedOption) ? selectedTextures[i] : normalTextures[i];
        int texW, texH;
        SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
        SDL_Rect dst = {(SCREEN_WIDTH - texW) / 2, 200 + i * 50, texW, texH};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }

    SDL_RenderPresent(renderer);
}

// Hàm xử lý input
void HandleInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_a:
            case SDLK_LEFT:
                cout << "Nhấn trái (A hoặc Mũi tên trái) - x hiện tại: " << player.rect.x << endl;
                if (player.rect.x > 0) {
                    player.rect.x -= PLAYER_SPEED;
                    cout << "Di chuyển trái - x mới: " << player.rect.x << endl;
                }
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                cout << "Nhấn phải (D hoặc Mũi tên phải) - x hiện tại: " << player.rect.x << endl;
                if (player.rect.x < SCREEN_WIDTH - player.rect.w) {
                    player.rect.x += PLAYER_SPEED;
                    cout << "Di chuyển phải - x mới: " << player.rect.x << endl;
                } else {
                    cout << "Không thể di chuyển phải - x >= " << (SCREEN_WIDTH - player.rect.w) << endl;
                }
                break;
            case SDLK_SPACE:
                cout << "Nhấn Phím cách - Bắn đạn" << endl;
                GameObject bullet = {{player.rect.x + 20, player.rect.y, 10, 20}, bulletTexture};
                bullets.push_back(bullet);
                break;
        }
    }
}

// Hàm xử lý input cho menu
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
                } else if (selectedOption == 1) {
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
    cout << "Thư mục làm việc: " << cwd << endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "Khởi tạo SDL thất bại: " << SDL_GetError() << endl;
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        cout << "Khởi tạo IMG thất bại: " << IMG_GetError() << endl;
        return 1;
    }
    if (TTF_Init() == -1) {
        cout << "Khởi tạo TTF thất bại: " << TTF_GetError() << endl;
        return 1;
    }

    window = SDL_CreateWindow("Space Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        cout << "Tạo cửa sổ thất bại: " << SDL_GetError() << endl;
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Tạo renderer thất bại: " << SDL_GetError() << endl;
        return 1;
    }

    // Tải phông chữ với đường dẫn tuyệt đối để kiểm tra
    font = TTF_OpenFont("D:/Space shooter/bin/Debug/assets/font.ttf", 24);
    if (!font) {
        cout << "Không thể tải phông chữ: " << TTF_GetError() << endl;
        cout << "Kiểm tra tệp tại: D:/Space shooter/bin/Debug/assets/font.ttf" << endl;
        return 1;
    }

    // Tạo texture cho tiêu đề
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Space Shooter", {255, 255, 255});
    if (titleSurface) {
        titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_FreeSurface(titleSurface);
    }

    // Tạo texture cho các tùy chọn menu
    for (int i = 0; i < numOptions; i++) {
        SDL_Surface* normalSurface = TTF_RenderText_Solid(font, menuOptions[i], {255, 255, 255});
        if (normalSurface) {
            normalTextures[i] = SDL_CreateTextureFromSurface(renderer, normalSurface);
            SDL_FreeSurface(normalSurface);
        }
        SDL_Surface* selectedSurface = TTF_RenderText_Solid(font, menuOptions[i], {255, 0, 0});
        if (selectedSurface) {
            selectedTextures[i] = SDL_CreateTextureFromSurface(renderer, selectedSurface);
            SDL_FreeSurface(selectedSurface);
        }
    }

    // Tải textures
    playerTexture = LoadTexture("assets/player.png");
    bulletTexture = LoadTexture("assets/bullet.png");
    enemyTexture = LoadTexture("assets/enemy.png");
    backgroundTexture = LoadTexture("assets/background.png");

    // Kiểm tra textures
    if (!playerTexture || !bulletTexture || !enemyTexture || !backgroundTexture) {
        cout << "Không thể tải một hoặc nhiều texture. Thoát..." << endl;
        cout << "Thử đường dẫn tuyệt đối..." << endl;
        playerTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/player.png");
        bulletTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/bullet.png");
        enemyTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/enemy.png");
        backgroundTexture = LoadTexture("D:/Space shooter/bin/Debug/assets/background.png");

        if (!playerTexture || !bulletTexture || !enemyTexture || !backgroundTexture) {
            cout << "Vẫn thất bại với đường dẫn tuyệt đối. Vui lòng kiểm tra tệp tại D:/Space_shooter/bin/Debug/assets/" << endl;
            for (int i = 0; i < numOptions; i++) {
                SDL_DestroyTexture(normalTextures[i]);
                SDL_DestroyTexture(selectedTextures[i]);
            }
            SDL_DestroyTexture(titleTexture);
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
        }
        SDL_Delay(16);
    }

    // Dọn dẹp bộ nhớ
    for (int i = 0; i < numOptions; i++) {
        SDL_DestroyTexture(normalTextures[i]);
        SDL_DestroyTexture(selectedTextures[i]);
    }
    SDL_DestroyTexture(titleTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
