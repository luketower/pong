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

int nums[4][15] = {
    {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        0, 1, 1,
        0, 0, 1,
        1, 1, 1,
    }
};

typedef struct {
    float x;
    float y;
} pos_t;

typedef struct {
    pos_t pos;
    float w;
    float h;
    float speed;
    int score;
} paddle_t;

typedef struct {
    pos_t pos;
    float xv;
    float yv;
    float radius;
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

float lerp(float start, float end, float pct)
{
    return start + pct*(end-start);
}

void update_ball(ball_t *ball, paddle_t *left_paddle, paddle_t *right_paddle, float elapsed_time)
{
    ball->pos.x += ball->xv * (elapsed_time/1000);
    ball->pos.y += ball->yv * (elapsed_time/1000);

    if ((ball->pos.y < 0) ||
        (ball->pos.y > SCREEN_HEIGHT))
    {
        ball->yv = -ball->yv;
    }

    if (ball->pos.x < 0)
    {
        right_paddle->score++;
        ball->pos = get_center();
    }
    else if (ball->pos.x > SCREEN_WIDTH)
    {
        left_paddle->score++;
        ball->pos = get_center();
    }


    if (ball->pos.x < (left_paddle->pos.x + left_paddle->w/2))
    {
        if ((ball->pos.y > (left_paddle->pos.y - left_paddle->h/2)) &&
            (ball->pos.y < (left_paddle->pos.y + left_paddle->h/2)))
        {
            ball->xv = -ball->xv;
        }
    }

    if (ball->pos.x > (right_paddle->pos.x + right_paddle->w/2))
    {
        if ((ball->pos.y > (right_paddle->pos.y - right_paddle->h/2)) &&
            (ball->pos.y < (right_paddle->pos.y + right_paddle->h/2)))
        {
            ball->xv = -ball->xv;
        }
    }
}

void update_paddle(paddle_t *paddle, pressed_t pressed, float elapsed_time)
{
    if (pressed == PRESSED_UP && ((paddle->pos.y - paddle->h/2) > 0))
    {
        paddle->pos.y -= paddle->speed * (elapsed_time/1000);
    }

    if ((pressed == PRESSED_DOWN) &&
        ((paddle->pos.y + paddle->h/2) < SCREEN_HEIGHT))
    {
        paddle->pos.y += paddle->speed * (elapsed_time/1000);
    }
}

void update_ai_paddle(paddle_t *paddle, ball_t ball)
{
    paddle->pos.y = ball.pos.y;
}

void draw_number(pos_t pos, int size, int num, u32 *screen_pixels)
{
    int startX = pos.x - (size*3)/2;
    int startY = pos.y - (size*5)/2;

    for (int i = 0; i < 15; i++)
    {
        if (nums[num][i] == 1)
        {
            for (int y = startY; y < startY+size; y++)
            {
                for (int x = startX; x < startX+size; x++)
                {
                    screen_pixels[(y)*SCREEN_WIDTH + x] = GREEN;
                }
            }
        }
        startX += size;
        if ((i+1)%3 == 0)
        {
            startY += size;
            startX -= size * 3;
        }
    }
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

    int num_x = lerp(paddle.pos.x, get_center().x, 0.2);
    pos_t num_position = { num_x, 35 };
    draw_number(num_position, 5, paddle.score, screen_pixels);
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
                screen_pixels[(int)(row+ball.pos.y)*SCREEN_WIDTH + col + (int)ball.pos.x] = GREEN;
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

    ball_t ball = {
        .pos = { (SCREEN_WIDTH/2), (SCREEN_HEIGHT/2) },
        .xv = 150,
        .yv = 150,
        .radius = 20
    };

    paddle_t player1 = {
        .pos = { 50, (SCREEN_HEIGHT/2) },
        .w = 5,
        .h = 40,
        .speed = 600,
        .score = 0
    };

    paddle_t player2 = {
        .pos = { (SCREEN_WIDTH - 50), (SCREEN_HEIGHT/2) },
        .w = 5,
        .h = 40,
        .speed = 300,
        .score = 0
    };

    u32 frame_start;
    float elapsed_time;

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

        update_paddle(&player1, pressed, elapsed_time);
        update_ai_paddle(&player2, ball); // Just tracking the ball for now.
        update_ball(&ball, &player1, &player2, elapsed_time);

        draw_paddle(player1, screen_pixels);
        draw_paddle(player2, screen_pixels);
        draw_ball(ball, screen_pixels);

        SDL_UpdateTexture(screen, NULL, screen_pixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderPresent(renderer);

        pressed = PRESSED_UNDEFINED;
        elapsed_time = (float)(SDL_GetTicks() - frame_start);

        if (elapsed_time < 6)
        {
            SDL_Delay(6 - (u32)(elapsed_time));
            elapsed_time = (float)(SDL_GetTicks() - frame_start);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
