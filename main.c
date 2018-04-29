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
    PRESSED_UP,
    PRESSED_DOWN
} pressed_t;

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
    float xv;
    float yv;
    int radius;
} ball_t;

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

pos_t get_center()
{
    pos_t center = { (SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) };
    return center;
}

void update_ball(ball_t *ball, paddle_t left_paddle, paddle_t right_paddle)
{
    ball->pos.x += ball->xv;
    ball->pos.y += ball->yv;

    if ((ball->pos.y < 0) ||
        (ball->pos.y > SCREEN_HEIGHT))
    {
        ball->yv = -ball->yv;
    }

    if ((ball->pos.x < 0) ||
        (ball->pos.x > SCREEN_WIDTH))
    {
        ball->pos = get_center();
    }

    if (ball->pos.x < (left_paddle.pos.x + left_paddle.w/2))
    {
        if ((ball->pos.y > (left_paddle.pos.y - left_paddle.h/2)) &&
            (ball->pos.y < (left_paddle.pos.y + left_paddle.h/2)))
        {
            ball->xv = -ball->xv;
        }
    }

    if (ball->pos.x > (right_paddle.pos.x + right_paddle.w/2))
    {
        if ((ball->pos.y > (right_paddle.pos.y - right_paddle.h/2)) &&
            (ball->pos.y < (right_paddle.pos.y + right_paddle.h/2)))
        {
            ball->xv = -ball->xv;
        }
    }
}

void update_paddle(paddle_t *paddle, pressed_t pressed)
{
    if (pressed == PRESSED_UP && ((paddle->pos.y - paddle->h/2) > 0))
    {
        if (paddle->pos.y < PADDLE_MOVE)
        {
            paddle->pos.y = 0;
        }
        else
        {
            paddle->pos.y -= PADDLE_MOVE;
        }
    }

    if ((pressed == PRESSED_DOWN) &&
        ((paddle->pos.y + paddle->h/2) < SCREEN_HEIGHT))
    {
        if ((SCREEN_HEIGHT - (paddle->pos.y + paddle->h/2)) < PADDLE_MOVE)
        {
            paddle->pos.y += SCREEN_WIDTH - paddle->pos.y;
        }
        else
        {
            paddle->pos.y += PADDLE_MOVE;
        }
    }
}

void update_ai_paddle(paddle_t *paddle, ball_t ball)
{
    paddle->pos.y = ball.pos.y;
}

void draw_paddle(paddle_t paddle, u32 *screen_pixels)
{
    SDL_assert(screen_pixels);

    int startX = paddle.pos.x - paddle.w/2;
    int startY = paddle.pos.y - paddle.h/2;

    for (int row = 0; row < paddle.h; row++)
    {
        for (int col = 0; col < paddle.w; col++)
        {
            screen_pixels[(row+startY)*SCREEN_WIDTH + col + startX] = GREEN;
        }
    }
}

void draw_ball(ball_t ball, u32 *screen_pixels)
{
    SDL_assert(screen_pixels);

    for (int row = -ball.radius; row < ball.radius; row++)
    {
        for (int col = -ball.radius; col < ball.radius; col++)
        {
            if (row*row+col*col < ball.radius)
            {
                screen_pixels[(row+ball.pos.y)*SCREEN_WIDTH + col + ball.pos.x] = GREEN;
            }
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

    ball_t ball = { { (SCREEN_WIDTH/2), (SCREEN_HEIGHT/2) }, 1, 1, 20 };
    paddle_t player1 = { { 50, (SCREEN_HEIGHT/2) }, 5, 40 };
    paddle_t player2 = { { (SCREEN_WIDTH - 50), (SCREEN_HEIGHT/2) }, 5, 40 };
    const int FPS = 120;
    u32 frame_start;
    u32 elapsed_time;

    while (!done)
    {
        frame_start = SDL_GetTicks();
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
                case SDLK_UP:
                    pressed = PRESSED_UP;
                    break;
                case SDLK_DOWN:
                    pressed = PRESSED_DOWN;
                    break;
                default:
                    break;
            }
        }

        memset(screen_pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));

        update_paddle(&player1, pressed);
        update_ai_paddle(&player2, ball); // Just tracking the ball for now.
        update_ball(&ball, player1, player2);

        draw_paddle(player1, screen_pixels);
        draw_paddle(player2, screen_pixels);
        draw_ball(ball, screen_pixels);

        SDL_UpdateTexture(screen, NULL, screen_pixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);

        pressed = PRESSED_UNDEFINED;

        elapsed_time = SDL_GetTicks() - frame_start;

        if (1000/FPS > elapsed_time)
        {
            SDL_Delay(1000/FPS - (SDL_GetTicks() - frame_start));
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
