#include "napi.h"

#include <chrono>
#include <thread>
#include "GreyPawnChess.h"

/**
 * Middleware code to connect Node.js to the C++ chess engine. Purpose and goal of this
 * file is to wrap all Node API code in one place and to be able to develop the chess engine
 * independently without having to deal with any of that.
 */

class GreyPawnChessAddon : public Napi::ObjectWrap<GreyPawnChessAddon> {
public:
	GreyPawnChessAddon(const Napi::CallbackInfo& info) 
		: Napi::ObjectWrap<GreyPawnChessAddon>(info)
	{
		// Parse constructor parameters here if needed.
	}

	// Used for initializing the Node addon.
	static Napi::Object Init(Napi::Env env, Napi::Object exports) 
	{
		Napi::Function func =
			DefineClass(env, "GreyPawnChess", {
				InstanceMethod("setup", &GreyPawnChessAddon::SetupGame),
				InstanceMethod("updateGameState", &GreyPawnChessAddon::UpdateGameState),
				InstanceMethod("startGame", &GreyPawnChessAddon::StartGame),
				InstanceMethod("stopGame", &GreyPawnChessAddon::StopGame)
			});

		Napi::FunctionReference* constructor = new Napi::FunctionReference();
		*constructor = Napi::Persistent(func);
		env.SetInstanceData(constructor);

		exports.Set("GreyPawnChess", func);
		return exports;
	}

	static Napi::Object NewInstance(Napi::Env env, Napi::Value arg)
	{
		Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New({arg});
		return obj;
	}

private:
	void SetupGame(const Napi::CallbackInfo& info)
	{
		// Parse the inputs and forward to the game.setup().
	}

	// Forwards server updates to the engine.
	void UpdateGameState(const Napi::CallbackInfo& info) 
	{
		
	}

	// Starts the engine calculations. It will run on a separate thread on the engine side
	// and call the given callback whenever it wants to make a move.
	void StartGame(const Napi::CallbackInfo& info) 
	{
		tsfn = Napi::ThreadSafeFunction::New(
			info.Env(),
			info[0].As<Napi::Function>(),
			"threadedSolveFunction",
			0,
			1
		);

		game.setMoveCallback(std::bind(
			&GreyPawnChessAddon::HandleEngineMove,
			this, 
			std::placeholders::_1
		));
		game.startGame();
	}

	// Signals the game that it should stop and waits for it to stop.
	void StopGame(const Napi::CallbackInfo& info) 
	{
		game.stopGame();
		tsfn.Release();
	}

	// Calls the JS callback.
	void CallMoveCallback(Napi::Env env, Napi::Function jsCallback, std::string* value)
	{
		jsCallback.Call({Napi::String::New(env, *value)});
		delete value;
	}

	// This callback should be called by the engine when it wants to make a move.
	void HandleEngineMove(const std::string& move)
	{
		std::string* moveStr = new std::string(move);
		// Perform a blocking call
		napi_status status = tsfn.BlockingCall(
			moveStr,
		 	std::bind(
				&GreyPawnChessAddon::CallMoveCallback, 
				this, 
				std::placeholders::_1, 
				std::placeholders::_2, 
				std::placeholders::_3
			)
		);
		if (status != napi_ok)
		{
			// TODO: Handle error
		}
	}

	// Thread safe reference to the JS callback function.
	Napi::ThreadSafeFunction tsfn;

	// The actual engine game.
	GreyPawnChess game;
};

Napi::Object InitGame(Napi::Env env, Napi::Object exports)
{
	return GreyPawnChessAddon::Init(env, exports);
}

NODE_API_MODULE(GreyPawnChessAddon, InitGame)