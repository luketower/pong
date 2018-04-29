#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 480
#define FALSE 0
#define TRUE 1
#define BOOL u32
#define PADDLE_MOVE 4
#define GREEN 0x21fb00
#define BALL_RATE 2

typedef Uint32 u32;

typedef enum {
    PRESSED_UNDEFINED,
    PRESSED_LEFT,
    PRESSED_RIGHT
} pressed_t;

typedef struct {
    int x;
    int y;
} ball_direction_t;

typedef struct {
    int x;
    int y;
} pos_t;

typedef struct {
    pos_t pos;
    int w;
    int h;
} paddle_t;

typedef struct {
    pos_t pos;
    int w;
    int h;
} ball_t;

BOOL paddle_hits_ball(paddle_t paddle, ball_t ball)
{
    return ((ball.pos.y >= (SCREEN_HEIGHT-paddle.h-ball.h)) &&
            (ball.pos.x > paddle.pos.x) &&
            (ball.pos.x < (paddle.pos.x + paddle.w)));
}

int ball_angle(paddle_t paddle, ball_t ball) {
    int x = 0;
    if (ball.pos.x < (paddle.pos.x + 15))
    {
        x -= 1;
    }
    else if (ball.pos.x > (paddle.pos.x + 25))
    {
        x += 1;
    }
    return x;
}

void calculate_direction(ball_direction_t *direction, paddle_t paddle, ball_t ball)
{
    if (paddle_hits_ball(paddle, ball))
    {
        direction->y = -1;
        direction->x = ball_angle(paddle, ball);
    }
    if (ball.pos.y <= 0)
    {
        direction->y = 1;
    }
    if (ball.pos.x < 0)
    {
        direction->x = 1;
    }
    if (ball.pos.x >= (SCREEN_WIDTH - ball.w))
    {
        direction->x = -1;
    }
}

void move_ball(ball_t *ball, ball_direction_t direction, int frame)
{
    if (frame % BALL_RATE == 0)
    {
        ball->pos.y += direction.y;
        ball->pos.x += direction.x;
    }
}

void draw_paddle(paddle_t paddle, u32 *screen_pixels)
{
    SDL_assert(screen_pixels);
    for (int row = 0; row < paddle.h; row++)
    {
        for (int col = 0; col < paddle.w; col++)
        {
            screen_pixels[(row+paddle.pos.y)*SCREEN_WIDTH + col + paddle.pos.x] = GREEN;
        }
    }
}

void draw_ball(ball_t ball, u32 *screen_pixels)
{
    SDL_assert(screen_pixels);
    for (int row = 0; row < ball.h; row++)
    {
        for (int col = 0; col < ball.w; col++)
        {
            screen_pixels[(row+ball.pos.y)*SCREEN_WIDTH + col + ball.pos.x] = GREEN;
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
    ball_direction_t direction = { 0, 1 };
    int frame = 0;

    ball_t ball = { { 0, 0 }, 5, 5 };
    ball.pos.x = (SCREEN_WIDTH-ball.w)/2;
    ball.pos.y = (SCREEN_HEIGHT-ball.h)/2;

    paddle_t paddle = { { 0, 0 }, 40, 5 };
    paddle.pos.x = (SCREEN_WIDTH-paddle.w)/2;
    paddle.pos.y = (SCREEN_HEIGHT-paddle.h);

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

        if (pressed == PRESSED_LEFT && (paddle.pos.x > 0))
        {
            if (paddle.pos.x < PADDLE_MOVE)
            {
                paddle.pos.x = 0;
            }
            else
            {
                paddle.pos.x -= PADDLE_MOVE;
            }
        }

        if ((pressed == PRESSED_RIGHT) &&
            ((paddle.pos.x + paddle.w) < SCREEN_WIDTH))
        {
            if ((SCREEN_WIDTH - (paddle.pos.x + paddle.w)) < PADDLE_MOVE)
            {
                paddle.pos.x += SCREEN_WIDTH - paddle.pos.x;
            }
            else
            {
                paddle.pos.x += PADDLE_MOVE;
            }
        }

        calculate_direction(&direction, paddle, ball);
        move_ball(&ball, direction, frame);

        if (ball.pos.y > SCREEN_HEIGHT)
        {
            done = TRUE;
        }

        pressed = PRESSED_UNDEFINED;

        draw_paddle(paddle, screen_pixels);
        draw_ball(ball, screen_pixels);
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
