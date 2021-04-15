#include "napi.h"

#include <chrono>
#include <thread>
#include "GreyPawnChess.h"


class GreyPawnChessAddon : public Napi::ObjectWrap<GreyPawnChessAddon> {
public:
	GreyPawnChessAddon(const Napi::CallbackInfo& info) 
		: Napi::ObjectWrap<GreyPawnChessAddon>(info)
	{
		// Parse constructor parameters here if needed.
	}

	static Napi::Object Init(Napi::Env env, Napi::Object exports) 
	{
		Napi::Function func =
			DefineClass(env, "GreyPawnChess", {
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
	Napi::Value UpdateGameState(const Napi::CallbackInfo& info) 
	{
		return Napi::Number::New(info.Env(), ++callCount);
	}

	Napi::Value StartGame(const Napi::CallbackInfo& info) 
	{
		tsfn = Napi::ThreadSafeFunction::New(
			info.Env(),
			info[0].As<Napi::Function>(),
			"threadedSolveFunction",
			0,
			1
		);

		moveCallBack = std::bind(&GreyPawnChessAddon::CallMoveCallback, this, std::placeholders::_1);
		game.setMoveCallback(moveCallBack);
		game.startGame();
		//solverThread = std::thread(&GreyPawnChess::startGame, &game);
		/*solverThread = std::thread([this] {
			auto callback = [](Napi::Env env, Napi::Function jsCallback, std::string* value) {
				// Transform native data into JS data, passing it to the provided
				// `jsCallback` -- the TSFN's JavaScript function.
				jsCallback.Call({Napi::String::New(env, *value)});

				// We're finished with the data.
				delete value;
			};

			game.setMoveCallback([this, callback](std::string &move) {
				std::string *moveParam = new std::string(move);
				// Perform a blocking call
				napi_status status = tsfn.BlockingCall(moveParam, callback);
				if ( status != napi_ok )
				{
					// Handle error
				}
			});

			game.startGame();

			// Release the thread-safe function
			tsfn.Release();
		});*/

		return Napi::Number::New(info.Env(), ++callCount);
	}

	Napi::Value StopGame(const Napi::CallbackInfo& info) 
	{
		game.stopGame();
		tsfn.Release();
		return Napi::Boolean::New(info.Env(), true);
	}

	void CallMoveCallback(const std::string& move)
	{
		static auto callback = [](Napi::Env env, Napi::Function jsCallback, std::string* value) {
			// Transform native data into JS data, passing it to the provided
			// `jsCallback` -- the TSFN's JavaScript function.
			jsCallback.Call({Napi::String::New(env, *value)});

			// We're finished with the data.
			delete value;
		};

		std::string *moveParam = new std::string(move);
		// Perform a blocking call
		napi_status status = tsfn.BlockingCall(moveParam, callback);
		if ( status != napi_ok )
		{
			// Handle error
		}
	}

	std::function<void(const std::string&)> moveCallBack;
	// This will be used by the worker thread to call the JS side callback.
	Napi::ThreadSafeFunction tsfn;
	std::thread solverThread;
	int callCount = 0;

	GreyPawnChess game;
};

Napi::Object InitGame(Napi::Env env, Napi::Object exports)
{
	return GreyPawnChessAddon::Init(env, exports);
}

NODE_API_MODULE(GreyPawnChessAddon, InitGame)