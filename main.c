#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 480
#define FALSE 0
#define TRUE 1
#define BOOL u32
#define PADDLE_MOVE 4
#define GREEN 0x21fb00

#define ball_t paddle_t
#define FillBall FillPaddle

typedef Uint32 u32;

typedef enum {
    PRESSED_UNDEFINED,
    PRESSED_LEFT,
    PRESSED_RIGHT
} pressed_t;

typedef enum {
    DOWN,
    UP
} ball_direction_t;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} paddle_t;


void FillPaddle(paddle_t paddle, u32 *screen_pixels)
{
    SDL_assert(screen_pixels);
    for (int row = 0; row < paddle.h; row++)
    {
        for (int col = 0; col < paddle.w; col++)
        {
            screen_pixels[(row+paddle.y)*SCREEN_WIDTH + col + paddle.x] = GREEN;
        }
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Pong Song",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    SDL_assert(window);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_SOFTWARE);
    SDL_assert(renderer);

    SDL_Texture *screen = SDL_CreateTexture(renderer,
                                            SDL_PIXELFORMAT_RGB888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            SCREEN_WIDTH,
                                            SCREEN_HEIGHT);
    SDL_assert(screen);

    u32 *screen_pixels = (u32*) calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(u32));
    SDL_assert(screen_pixels);

    BOOL done = FALSE;
    pressed_t pressed = PRESSED_UNDEFINED;
    ball_direction_t direction = DOWN;
    int frame = 0;
    int ball_rate = 2;

    ball_t ball = { 0, 0, 5, 5 };
    ball.x = (SCREEN_WIDTH-ball.w)/2;
    ball.y = (SCREEN_HEIGHT-ball.h)/2;

    paddle_t paddle = { 0, 0, 40, 5 };
    paddle.x = (SCREEN_WIDTH-paddle.w)/2;
    paddle.y = (SCREEN_HEIGHT-paddle.h);

    while (!done)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            SDL_Keycode code = event.key.keysym.sym;

            switch (code)
            {
                case SDLK_ESCAPE:
                case SDLK_q:
                    done = TRUE;
                    break;
                case SDLK_LEFT:
                    pressed = PRESSED_LEFT;
                    break;
                case SDLK_RIGHT:
                    pressed = PRESSED_RIGHT;
                    break;
                default:
                    break;
            }
        }

        memset(screen_pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));

        if (pressed == PRESSED_LEFT && (paddle.x != 0))
        {
            paddle.x -= PADDLE_MOVE;
        }

        if ((pressed == PRESSED_RIGHT) &&
            ((paddle.x + paddle.w) != SCREEN_WIDTH))
        {
            paddle.x += PADDLE_MOVE;
        }

        if (frame % ball_rate == 0)
        {
            if ((direction == DOWN) &&
                (ball.y < (SCREEN_HEIGHT-paddle.h-ball.h)))
            {
                ball.y += 1;
            }
            if ((direction == UP) &&
                (ball.y > 0))
            {
                ball.y -= 1;
            }
        }

        if (ball.y >= (SCREEN_HEIGHT-paddle.h-ball.h))
        {
            direction = UP;
        }

        if (ball.y <= 0)
        {
            direction = DOWN;
        }

        pressed = PRESSED_UNDEFINED;

        FillPaddle(paddle, screen_pixels);
        FillBall(ball, screen_pixels);
        SDL_UpdateTexture(screen, NULL, screen_pixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);
        frame += 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
