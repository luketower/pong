#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 480
#define PADDLE_DISTANCE_FROM_WALL 50
#define FALSE 0
#define TRUE 1
#define bool u8
#define GREEN 0x21fb00
#define global static

typedef uint32_t u32;
typedef uint8_t u8;;

enum keyboard_press
{
    KeyboardPress_Undefined,
    KeyboardPress_Up,
    KeyboardPress_Down,
    KeyboardPress_Space
};

enum game_mode
{
    GameMode_Ready,
    GameMode_Play
};

global game_mode GameMode;

global int ScoreNumbers[4][15] =
{
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

struct position
{
    float X;
    float Y;
};

struct paddle
{
    position Position;
    float Width;
    float Height;
    float Speed;
    int Score;
};

struct ball
{
    position Position;
    float VelocityX;
    float VelocityY;
    float Radius;
};

global int SCREEN_VERTICAL_CENTER = SCREEN_HEIGHT/2;
global int SCREEN_HORIZONTAL_CENTER = SCREEN_WIDTH/2;
global position SCREEN_CENTER = { SCREEN_HORIZONTAL_CENTER, SCREEN_VERTICAL_CENTER };

float Lerp(float Start, float End, float Percent)
{
    return Start + (End-Start)*Percent;
}

void SetGameToReadyState(paddle *LeftPaddle, paddle *RightPaddle, ball *Ball)
{
    Ball->Position = SCREEN_CENTER;
    GameMode = GameMode_Ready;
    LeftPaddle->Position.Y = SCREEN_VERTICAL_CENTER;
    RightPaddle->Position.Y = SCREEN_VERTICAL_CENTER;
}

bool BallHitsLeftPaddle(paddle *LeftPaddle, ball *Ball)
{
    return (Ball->Position.X < (LeftPaddle->Position.X + LeftPaddle->Width/2)) &&
        (Ball->Position.X > (LeftPaddle->Position.X - LeftPaddle->Width/2)) &&
        (Ball->Position.Y > (LeftPaddle->Position.Y - LeftPaddle->Height/2)) &&
        (Ball->Position.Y < (LeftPaddle->Position.Y + LeftPaddle->Height/2));
}

bool BallHitsRightPaddle(paddle *RightPaddle, ball *Ball)
{
    return (Ball->Position.X > (RightPaddle->Position.X - RightPaddle->Width/2)) &&
        (Ball->Position.X < (RightPaddle->Position.X + RightPaddle->Width/2)) &&
        (Ball->Position.Y > (RightPaddle->Position.Y - RightPaddle->Height/2)) &&
        (Ball->Position.Y < (RightPaddle->Position.Y + RightPaddle->Height/2));
}

void UpdateBall(ball *Ball, paddle *LeftPaddle, paddle *RightPaddle, float ElapsedTime)
{
    Ball->Position.X += Ball->VelocityX * (ElapsedTime/1000);
    Ball->Position.Y += Ball->VelocityY * (ElapsedTime/1000);

    if((Ball->Position.Y < 0) ||
       (Ball->Position.Y > SCREEN_HEIGHT))
    {
        Ball->VelocityY = -Ball->VelocityY;
        return;
    }

    if(Ball->Position.X < 0)
    {
        RightPaddle->Score++;
        SetGameToReadyState(LeftPaddle, RightPaddle, Ball);
        return;
    }

    if(Ball->Position.X > SCREEN_WIDTH)
    {
        LeftPaddle->Score++;
        SetGameToReadyState(LeftPaddle, RightPaddle, Ball);
        return;
    }

    if(BallHitsLeftPaddle(LeftPaddle, Ball))
    {
        Ball->VelocityX = -Ball->VelocityX;
        Ball->Position.X = LeftPaddle->Position.X + LeftPaddle->Width/2.0;
    }
    if(BallHitsRightPaddle(RightPaddle, Ball))
    {
        Ball->VelocityX = -Ball->VelocityX;
        Ball->Position.X = RightPaddle->Position.X - RightPaddle->Width/2.0;
    }
}

void UpdatePaddle(paddle *Paddle, keyboard_press KeyboardPress, float ElapsedTime)
{
    if(KeyboardPress == KeyboardPress_Up && ((Paddle->Position.Y - Paddle->Height/2) > 0))
    {
        Paddle->Position.Y -= Paddle->Speed * (ElapsedTime/1000);
    }

    if((KeyboardPress == KeyboardPress_Down) &&
        ((Paddle->Position.Y + Paddle->Height/2) < SCREEN_HEIGHT))
    {
        Paddle->Position.Y += Paddle->Speed * (ElapsedTime/1000);
    }
}

void UpdateAiPaddle(paddle *Paddle, ball Ball)
{
    Paddle->Position.Y = Ball.Position.Y;
}

void SetPixel(int X, int Y, u32 *ScreenPixels)
{
    int Index = ((Y*SCREEN_WIDTH) + X);
    int PixelLength = SCREEN_WIDTH*SCREEN_HEIGHT;

    if(Index < PixelLength && Index > 0)
    {
        ScreenPixels[Index] = GREEN;
    }
}

void DrawScore(position Position, int Size, int Num, u32 *ScreenPixels)
{
    int StartX = Position.X - (Size*3)/2;
    int StartY = Position.Y - (Size*5)/2;

    for(int i = 0; i < 15; i++)
    {
        if(ScoreNumbers[Num][i] == 1)
        {
            for(int Y = StartY; Y < StartY+Size; Y++)
            {
                for(int X = StartX; X < StartX+Size; X++)
                {
                    SetPixel(X, Y, ScreenPixels);
                }
            }
        }
        StartX += Size;
        if((i+1)%3 == 0)
        {
            StartY += Size;
            StartX -= Size * 3;
        }
    }
}

ball InitBall()
{
    ball Ball;

    Ball.Position.X = SCREEN_HORIZONTAL_CENTER;
    Ball.Position.Y = SCREEN_VERTICAL_CENTER;
    Ball.VelocityX = 150;
    Ball.VelocityY = 150;
    Ball.Radius = 30;

    return Ball;
}

paddle InitPaddle(float PaddleX)
{
    paddle Paddle;

    Paddle.Position.X = PaddleX;
    Paddle.Position.Y = SCREEN_VERTICAL_CENTER;
    Paddle.Width = 5;
    Paddle.Height = 40;
    Paddle.Speed = 600;
    Paddle.Score = 0;

    return Paddle;
}

void DrawPaddle(paddle Paddle, u32 *ScreenPixels)
{
    SDL_assert(ScreenPixels);

    int StartX = Paddle.Position.X - Paddle.Width/2;
    int StartY = Paddle.Position.Y - Paddle.Height/2;

    for(int Row = 0; Row < Paddle.Height; Row++)
    {
        for(int Col = 0; Col < Paddle.Width; Col++)
        {
            SetPixel((Col+StartX), (Row+StartY), ScreenPixels);
        }
    }

    int ScoreX = Lerp(Paddle.Position.X, SCREEN_CENTER.X, 0.2);
    position ScorePosition = { ScoreX, 35 };
    DrawScore(ScorePosition, 5, Paddle.Score, ScreenPixels);
}

void DrawBall(ball Ball, u32 *ScreenPixels)
{
    SDL_assert(ScreenPixels);

    for(int Row = -Ball.Radius; Row < Ball.Radius; Row++)
    {
        for(int Col = -Ball.Radius; Col < Ball.Radius; Col++)
        {
            if(Row*Row+Col*Col < Ball.Radius)
            {
                int PixelX = Col+(int)Ball.Position.X;
                int PixelY = (int)Row+Ball.Position.Y;
                SetPixel(PixelX, PixelY, ScreenPixels);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *Window = SDL_CreateWindow("Pong Song",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    SDL_assert(Window);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, 0, SDL_RENDERER_SOFTWARE);
    SDL_assert(Renderer);

    SDL_Texture *Screen = SDL_CreateTexture(Renderer,
                                            SDL_PIXELFORMAT_RGB888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            SCREEN_WIDTH,
                                            SCREEN_HEIGHT);
    SDL_assert(Screen);

    u32 *ScreenPixels = (u32*) calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(u32));
    SDL_assert(ScreenPixels);

    bool Done = FALSE;
    keyboard_press KeyboardPress = KeyboardPress_Undefined;

    ball Ball = InitBall();

    paddle LeftPaddle = InitPaddle(PADDLE_DISTANCE_FROM_WALL);
    paddle RightPaddle = InitPaddle(SCREEN_WIDTH - PADDLE_DISTANCE_FROM_WALL);

    u32 FrameStart;
    float ElapsedTime;
    GameMode = GameMode_Ready;

    while (!Done)
    {
        FrameStart = SDL_GetTicks();
        SDL_Event Event;

        while (SDL_PollEvent(&Event))
        {
            SDL_Keycode KeyCode = Event.key.keysym.sym;

            switch (KeyCode)
            {
                case SDLK_ESCAPE:
                case SDLK_q:
                    Done = TRUE;
                    break;
                case SDLK_UP:
                    KeyboardPress = KeyboardPress_Up;
                    break;
                case SDLK_DOWN:
                    KeyboardPress = KeyboardPress_Down;
                    break;
                case SDLK_SPACE:
                    KeyboardPress = KeyboardPress_Space;
                default:
                    break;
            }
        }


        if(GameMode == GameMode_Play)
        {
            UpdatePaddle(&LeftPaddle, KeyboardPress, ElapsedTime);
            UpdateAiPaddle(&RightPaddle, Ball); // Just tracking the ball for now.
            UpdateBall(&Ball, &LeftPaddle, &RightPaddle, ElapsedTime);
        }
        else if(GameMode == GameMode_Ready)
        {
            if(KeyboardPress == KeyboardPress_Space)
            {
                GameMode = GameMode_Play;

                if (LeftPaddle.Score == 3 || RightPaddle.Score == 3)
                {
                    LeftPaddle.Score = 0;
                    RightPaddle.Score = 0;
                }
            }
        }

        memset(ScreenPixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));

        DrawPaddle(LeftPaddle, ScreenPixels);
        DrawPaddle(RightPaddle, ScreenPixels);
        DrawBall(Ball, ScreenPixels);

        SDL_UpdateTexture(Screen, NULL, ScreenPixels, SCREEN_WIDTH * sizeof(u32));
        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer, Screen, NULL, NULL);
        SDL_RenderPresent(Renderer);

        KeyboardPress = KeyboardPress_Undefined;
        ElapsedTime = (float)(SDL_GetTicks() - FrameStart);

        if(ElapsedTime < 6)
        {
            SDL_Delay(6 - (u32)(ElapsedTime));
            ElapsedTime = (float)(SDL_GetTicks() - FrameStart);
        }
    }

    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}
