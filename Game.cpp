#include "Game.h"

const int thickness = 15;
const float paddleH = 100.0f;

Game::Game() : mWindow(nullptr), mRenderer(nullptr), mIsRunning(true), mTicksCount(0), mPaddleDir(0)
{
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO); // Initialize only the video system
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow(
		"Pong", // Window title
		100,  // Top left x-coord of window
		100,  // Top left y-coord of window
		1024, // Width
		768,  // Height
		0     // Flags (0 for no flags set)
	); // Returns nullptr if failed
	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,      // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC // Use graphics hardware | Enable vsync
	);
	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}


	mPaddlePos.x = 10.0f;
	mPaddlePos.y = 768.0f / 2.0f;
	mBallPos.x = 1024.0f / 2.0f;
	mBallPos.y = 768.0f / 2.0f;
	mBallVel.x = -200.0f;
	mBallVel.y = 235.0f;

	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) // While there are still events in the queue
	{
		switch (event.type)
		{
			case SDL_QUIT: // Hit "X" button on window
				mIsRunning = false;
				break;
		}
	}

	// Get state of the keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// If escape is pressed, also end loop
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	// Update paddle direction based on W/S keys
	mPaddleDir = 0; // Not moving
	if (state[SDL_SCANCODE_W])
	{
		mPaddleDir -= 1; // Up (negative y)
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddleDir += 1; // Down (positive y)
	}
}

void Game::UpdateGame()
{
	// Wait until 16ms has elapsed since last frame (Target ~ 60 FPS)
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	// Delta time is the difference in ticks from the last frame (converted to seconds)
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	// Update tick counts (for the next frame)
	mTicksCount = SDL_GetTicks();

	// Clamp maximum delta time value (for debugging purposes)
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// TODO: Update game objects in game world as function of delta time!
	if (mPaddleDir != 0)
	{
		mPaddlePos.y += mPaddleDir * 300.0f * deltaTime; // Update paddle position at 300.0f pixels/second

		// Make sure paddle doesn't move off screen
		if (mPaddlePos.y < (paddleH / 2.0f + thickness))
		{
			mPaddlePos.y = paddleH / 2.0f + thickness;
		}
		else if (mPaddlePos.y > (768.0f - paddleH / 2.0f - thickness))
		{
			mPaddlePos.y = 768.0f - paddleH / 2.0f - thickness;
		}
	}

	// Update ball position based on ball velocity
	mBallPos.x += mBallVel.x * deltaTime;
	mBallPos.y += mBallVel.y * deltaTime;

	// Bounce if needed
	// Did we intersect with the paddle?
	float diff = mPaddlePos.y - mBallPos.y;
	// Take absolute value of difference
	diff = (diff > 0.0f) ? diff : -diff;
	if (
		// Our y-difference is small enough
		diff <= paddleH / 2.0f &&
		// Ball is at the correct x-position
		mBallPos.x <= 25.0f && mBallPos.x >= 20.0f &&
		// The ball is moving to the left
		mBallVel.x < 0.0f)
	{
		mBallVel.x *= -1.0f;
	}
	else if (mBallPos.x >= (1024.0f - thickness) && mBallVel.x > 0.0f) // Collision of ball with the right
	{
		mBallVel.x *= -1;
	}
	else if (mBallPos.x <= 0.0f) // If the ball goes off screen on the left, game over
	{
		mIsRunning = false;
	}
	

	if (mBallPos.y <= thickness && mBallVel.y < 0.0f) // Collision of ball with the top
	{
		mBallVel.y *= -1;
	}
	else if (mBallPos.y >= (768.0f - thickness) && mBallVel.y > 0.0f) // Collision of ball with the bottom
	{
		mBallVel.y *= -1;
	}
	
}

void Game::GenerateOutput()
{
	// 1. Clear the back buffer to a color (the game's current buffer)
	SDL_SetRenderDrawColor(
		mRenderer,
		39,   // R
		58,   // G
		93,   // B
		255   // A
	);
	SDL_RenderClear(mRenderer); // Clear the back buffer to the current draw color

	// 2. Draw the entire game scene
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	SDL_Rect wall{
		0,        // Top left x
		0,        // Top left y
		1024,     // Width
		thickness // Height
	};
	SDL_RenderFillRect(mRenderer, &wall); // Draw the rectangle (top wall)
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall); // Bottom wall
	wall.y = 0;
	wall.x = 1024 - thickness;
	wall.w = thickness;
	wall.h = 768;
	SDL_RenderFillRect(mRenderer, &wall); // Right wall

	// Draw paddle
	SDL_Rect paddle{
		static_cast<int>(mPaddlePos.x),
		static_cast<int>(mPaddlePos.y - paddleH / 2),
		thickness,
		static_cast<int>(paddleH)
	};
	SDL_RenderFillRect(mRenderer, &paddle);

	// Draw ball
	SDL_Rect ball{
		static_cast<int>(mBallPos.x - thickness / 2),
		static_cast<int>(mBallPos.y - thickness / 2),
		thickness,
		thickness
	};
	SDL_RenderFillRect(mRenderer, &ball);


	// 3. Swap the front buffer and back buffer
	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}