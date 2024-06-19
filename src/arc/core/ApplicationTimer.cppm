//
// Created by Matrix on 2024/6/10.
//

export module Core.ApplicationTimer;

export import Core.Unit;
import std;
import ext.RuntimeException;

export namespace Core{
	class ApplicationTimer{
		float deltaTime_internal{};    //InTick | No Pause
		float deltaTick_internal{};    //InTick | NO Pause

		float updateDeltaTime_internal{};    //InTick
		float updateDeltaTick_internal{};    //InTick

		float globalTime_internal{};     //Sec  | No Pause
		float globalTick_internal{};     //Tick | NO Pause

		float updateTime_internal{};     //Sec
		float updateTick_internal{};     //Tick
		
		Core::DeltaSetter deltaSetter{nullptr};
		Core::TimerSetter timerSetter{nullptr};

		bool paused = false;

	public:
		static constexpr float TicksPerSecond{Core::Tick::period::den};

		constexpr void pause() noexcept {paused = true;}

		[[nodiscard]] constexpr bool isPaused() const noexcept{return paused;}

		constexpr void resume() noexcept {paused = false;}

		//TODO fire an event?
		constexpr void setPause(const bool v) {paused = v;}
		
		[[nodiscard]] constexpr Core::DeltaSetter getDeltaSetter() const noexcept{ return deltaSetter; }

		constexpr void setDeltaSetter(const Core::DeltaSetter deltaSetter) noexcept{ this->deltaSetter = deltaSetter; }

		[[nodiscard]] constexpr Core::TimerSetter getTimerSetter() const noexcept{ return timerSetter; }

		constexpr void setTimerSetter(const Core::TimerSetter timerSetter) noexcept{ this->timerSetter = timerSetter; }

		void fetchGlobalTime(){
#if DEBUG_CHECK
			if(!timerSetter || !deltaSetter){
				throw ext::NullPointerException{"Missing Timer Function Pointer"};
			}
#endif

			deltaTime_internal = deltaSetter(globalTime_internal);
			globalTime_internal = timerSetter();

			updateDeltaTime_internal = !paused * deltaTime_internal;
			updateTime_internal += updateDeltaTime_internal;

			globalTick_internal = globalTime_internal * TicksPerSecond;
			deltaTick_internal = deltaTime_internal * TicksPerSecond;
			updateDeltaTick_internal = !paused * deltaTick_internal;

			updateTick_internal = updateTime_internal * TicksPerSecond;

			if(getGlobalTick() > 1E10){
				resetTime();
			}
		}

		void resetTime() {
			globalTime_internal = globalTick_internal = updateTime_internal = updateTick_internal = 0;
		}

		//TODO all uses tick?

		[[nodiscard]] constexpr Core::Tick getDeltaTick() const noexcept{return {deltaTick_internal};}

		[[nodiscard]] constexpr Core::Tick delta() const noexcept{return getDeltaTick();}

		[[nodiscard]] constexpr Core::Sec getDeltaTime() const noexcept{return {deltaTime_internal};}

		[[nodiscard]] constexpr Core::Sec getUpdateDelta() const noexcept{return updateDeltaTime_internal;}

		[[nodiscard]] constexpr Core::Tick getUpdateDeltaTick() const noexcept{return {updateDeltaTick_internal};}

		[[nodiscard]] constexpr Core::Sec getGlobalTime() const noexcept{return globalTime_internal;}

		[[nodiscard]] constexpr Core::Tick getGlobalTick() const noexcept{return {globalTick_internal};}

		[[nodiscard]] constexpr Core::Sec getUpdateTime() const noexcept{return updateTime_internal;}

		[[nodiscard]] constexpr Core::Tick getUpdateTick() const noexcept{return {updateTick_internal};}
	};
	

	
}
