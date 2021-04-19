#include "napi.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "../engine/GreyPawnChess.h"
#include "../engine/GameState.h"

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
		std::cout << "Setup game" << std::endl;

		// Called like this: setup(myColor, msg.clock.initial, msg.clock.increment, msg.variant.key)
		// Parse the inputs and forward to the game.setup().
		Napi::String color = info[0].As<Napi::String>();
		Napi::Number initialClock = info[1].As<Napi::Number>();
		Napi::Number incrementClock = info[2].As<Napi::Number>();
		Napi::String variant = info[3].As<Napi::String>();
		
		std::string colorStr = (std::string)color;

		game.setup(colorStr.front(), initialClock.Int32Value(), incrementClock.Int32Value(), (std::string)variant);
	}

	// Forwards server updates to the engine.
	void UpdateGameState(const Napi::CallbackInfo& info) 
	{
		Napi::Object stateObj = info[0].As<Napi::Object>();
		Napi::Value movesVal = stateObj.Get("moves");
		Napi::Value wTimeMs = stateObj.Get("wtime");
		Napi::Value bTimeMs = stateObj.Get("btime");
		Napi::Value wIncMs = stateObj.Get("winc");
		Napi::Value bIncMs = stateObj.Get("binc");
		Napi::Value statusVal = stateObj.Get("status");
		Napi::Value winnerVal = stateObj.Get("winner");

		std::cout << "Update game state" << std::endl;

		std::string statusString = (std::string)statusVal.As<Napi::String>();
		GameStatus status;
		if (statusString.compare("created"))
			status = GameStatus::CREATED;
		else if (statusString.compare("started"))
			status = GameStatus::STARTED;
		else if (statusString.compare("aborted"))
			status = GameStatus::ABORTED;
		else if (statusString.compare("mate"))
			status = GameStatus::MATE;
		else if (statusString.compare("resign"))
			status = GameStatus::RESIGN;
		else if (statusString.compare("staleMate"))
			status = GameStatus::STALEMATE;
		else if (statusString.compare("timeout"))
			status = GameStatus::TIMEOUT;
		else if (statusString.compare("draw"))
			status = GameStatus::DRAW;
		else if (statusString.compare("outOfTime"))
			status = GameStatus::OUTOFTIME;
		else if (statusString.compare("cheat"))
			status = GameStatus::CHEAT;
		else if (statusString.compare("noStart"))
			status = GameStatus::NOSTART;
		else if (statusString.compare("unknownfinish"))
			status = GameStatus::UNKNOWN_FINISH;
		else if (statusString.compare("variantEnd"))
			status = GameStatus::VARIANT_END;

		Color winnerColor;
		if (winnerVal.IsUndefined())
		{
			winnerColor = Color::NONE;
		}
		else
		{
			std::string winnerString = (std::string)winnerVal.As<Napi::String>();
			winnerColor = winnerString.front() == 'w' ? Color::WHITE : Color::BLACK;
		}

		std::string movesString = (std::string)movesVal.As<Napi::String>();

		GameState state(
			wTimeMs.As<Napi::Number>().Int32Value(),
			bTimeMs.As<Napi::Number>().Int32Value(),
			wIncMs.As<Napi::Number>().Int32Value(),
			bIncMs.As<Napi::Number>().Int32Value(),
			status,
			winnerColor,
			movesString
		);

		game.updateGameState(state);
	}

	// Starts the engine calculations. It will run on a separate thread on the engine side
	// and call the given callback whenever it wants to make a move.
	void StartGame(const Napi::CallbackInfo& info) 
	{
		std::cout << "Start game" << std::endl;

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