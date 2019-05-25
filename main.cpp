#include <SDL2/SDL.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1
#define bool u8
#define GREEN 0x21fb00
#define global static

typedef uint32_t u32;
typedef uint8_t u8;;

struct position
{
    float X;
    float Y;
};

struct game_window
{
    int Width;
    int Height;
    int PaddleDistanceFromWall;
    float VerticalCenter;
    float HorizontalCenter;
    position ScreenCenter;
};

global game_window GameWindow;

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

struct paddle_zone_velocity {
    float OuterTop;
    float InnerTop;
    float Center;
    float InnerBottom;
    float OuterBottom;
};

global paddle_zone_velocity PaddleZoneVelocity;

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

float Lerp(float Start, float End, float Percent)
{
    return Start + (End-Start)*Percent;
}

void SetupGameWindow()
{
    GameWindow.Width = 540;
    GameWindow.Height = 480;
    GameWindow.PaddleDistanceFromWall = 50;
    GameWindow.VerticalCenter = GameWindow.Height/2;
    GameWindow.HorizontalCenter = GameWindow.Width/2;
    GameWindow.ScreenCenter.X = GameWindow.HorizontalCenter;
    GameWindow.ScreenCenter.Y = GameWindow.VerticalCenter;
}

void SetupPaddleZoneVelocity()
{
    PaddleZoneVelocity.OuterBottom = 300;
    PaddleZoneVelocity.InnerBottom = 225;
    PaddleZoneVelocity.Center = 0;
    PaddleZoneVelocity.InnerTop = -PaddleZoneVelocity.InnerBottom;;
    PaddleZoneVelocity.OuterTop = -PaddleZoneVelocity.OuterBottom;;

}

void SetGameToReadyState(paddle *LeftPaddle, paddle *RightPaddle, ball *Ball)
{
    Ball->Position = GameWindow.ScreenCenter;
    Ball->VelocityY = 0;
    GameMode = GameMode_Ready;
    LeftPaddle->Position.Y = GameWindow.VerticalCenter;
    RightPaddle->Position.Y = GameWindow.VerticalCenter;
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

float GetPaddleZoneVelocity(paddle *Paddle, ball *Ball)
{
    float Position = Ball->Position.Y;
    float ZoneWidth = Paddle->Height/5;
    float TopOfPaddle = Paddle->Position.Y - Paddle->Height/2;
    float OuterTopZone = TopOfPaddle + ZoneWidth;
    float InnerTopZone = TopOfPaddle + ZoneWidth*2;
    float CenterZone = TopOfPaddle + ZoneWidth*3;
    float InnerBottomZone = TopOfPaddle + ZoneWidth*4;
    float OuterBottomZone = TopOfPaddle + ZoneWidth*5;

    // Note: Order matters for now
    if(Position <= OuterTopZone)
    {
        printf("OuterTop\n");
        return PaddleZoneVelocity.OuterTop;
    }

    if(Position <= InnerTopZone)
    {
        printf("InnerTop\n");
        return PaddleZoneVelocity.InnerTop;
    }

    if(Position <= CenterZone)
    {
        printf("Center\n");
        return PaddleZoneVelocity.Center;
    }

    if(Position <= InnerBottomZone)
    {
        printf("InnerBottom\n");
        return PaddleZoneVelocity.InnerBottom;
    }

    if(Position <= OuterBottomZone)
    {
        printf("OuterBottom\n");
        return PaddleZoneVelocity.OuterBottom;
    }

    printf("Center\n");
    return PaddleZoneVelocity.Center;
}

void updateBallVelocity(paddle *Paddle, ball *Ball)
{
    if(Paddle->Position.X == (GameWindow.PaddleDistanceFromWall))
    {
        Ball->VelocityX = -Ball->VelocityX;
        // Calculate the PaddleZoneVelocity by determining in which PaddleZone
        // the ball resides
        Ball->VelocityY = GetPaddleZoneVelocity(Paddle, Ball);
    } else
    {
        Ball->VelocityX = -Ball->VelocityX;
    }
}

void UpdateBall(ball *Ball, paddle *LeftPaddle, paddle *RightPaddle, float ElapsedTime)
{
    Ball->Position.X += Ball->VelocityX * (ElapsedTime/1000);
    Ball->Position.Y += Ball->VelocityY * (ElapsedTime/1000);

    if((Ball->Position.Y < 0) ||
       (Ball->Position.Y > GameWindow.Height))
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

    if(Ball->Position.X > GameWindow.Width)
    {
        LeftPaddle->Score++;
        SetGameToReadyState(LeftPaddle, RightPaddle, Ball);
        return;
    }

    if(BallHitsLeftPaddle(LeftPaddle, Ball))
    {
        updateBallVelocity(LeftPaddle, Ball);
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
        ((Paddle->Position.Y + Paddle->Height/2) < GameWindow.Height))
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
    int Index = ((Y*GameWindow.Width) + X);
    int PixelLength = GameWindow.Width*GameWindow.Height;

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

    Ball.Position.X = GameWindow.HorizontalCenter;
    Ball.Position.Y = GameWindow.VerticalCenter;
    Ball.VelocityX = 150;
    Ball.VelocityY = 150;
    Ball.Radius = 30;

    return Ball;
}

paddle InitPaddle(float PaddleX)
{
    paddle Paddle;

    Paddle.Position.X = PaddleX;
    Paddle.Position.Y = GameWindow.VerticalCenter;
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

    int ScoreX = Lerp(Paddle.Position.X, GameWindow.ScreenCenter.X, 0.2);
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
    SetupGameWindow();
    SetupPaddleZoneVelocity();

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *Window = SDL_CreateWindow("Pong Song",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              GameWindow.Width,
                              GameWindow.Height,
                              SDL_WINDOW_SHOWN);

    SDL_assert(Window);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, 0, SDL_RENDERER_SOFTWARE);
    SDL_assert(Renderer);

    SDL_Texture *Screen = SDL_CreateTexture(Renderer,
                                            SDL_PIXELFORMAT_RGB888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            GameWindow.Width,
                                            GameWindow.Height);
    SDL_assert(Screen);

    u32 *ScreenPixels = (u32*) calloc(GameWindow.Width * GameWindow.Height, sizeof(u32));
    SDL_assert(ScreenPixels);

    bool Done = FALSE;
    keyboard_press KeyboardPress = KeyboardPress_Undefined;

    ball Ball = InitBall();

    paddle LeftPaddle = InitPaddle(GameWindow.PaddleDistanceFromWall);
    paddle RightPaddle = InitPaddle(GameWindow.Width - GameWindow.PaddleDistanceFromWall);

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

                if(LeftPaddle.Score == 3 || RightPaddle.Score == 3)
                {
                    LeftPaddle.Score = 0;
                    RightPaddle.Score = 0;
                }
            }
        }

        memset(ScreenPixels, 0, GameWindow.Width * GameWindow.Height * sizeof(u32));

        DrawPaddle(LeftPaddle, ScreenPixels);
        DrawPaddle(RightPaddle, ScreenPixels);
        DrawBall(Ball, ScreenPixels);

        SDL_UpdateTexture(Screen, NULL, ScreenPixels, GameWindow.Width * sizeof(u32));
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
