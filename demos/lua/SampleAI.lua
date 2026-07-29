--[[
--	Hovercraft Universe AI Script
--	
--	@author Dirk Delahaye
--]]

-----------------------------
--	This script expects functions:
--
--	void output(string message)
--	? getVariable(int data_constant)
-----------------------------


-----------------------------
-- State Constants
-----------------------------
STATE_IDLE		= 0
STATE_RACE		= 1
STATE_FOLLOW	= 2
-----------------------------


-----------------------------
-- Game Data Constants
-----------------------------

-----------------------------


-----------------------------
-- Global Variables
-----------------------------
mCurrentState	= STATE_IDLE
-----------------------------










--[[
--	Main function. Decides the next action, based on the state.
--]]
function decide()
	output("AI Deciding")
	if (mCurrentState == STATE_IDLE) then
		--nothing!
	elseif (mCurrentState == STATE_FOLLOW) then	
		followNext()
	elseif (mCurrentState == STATE_RACE) then
		followTrack()
	end
end

function followNext()
	-- 1) Action
	-- get my position
	-- get position of closest opponent ahead
	-- get my orientation
	-- calculate angle between (my orientation) and vector (nextCP - myPos)
	-- transform this angle into a movement (one of 8 directions: UP, UPRIGHT, RIGHT, RIGHTDOWN, DOWN, ...
	
	-- 2) Next State
	-- if distance(myPos, nextOpponent) >= distanceThreshold
	-- mCurrentState = STATE_RACE
end

function followTrack()
	-- get my position
	-- get next checkpoint position
	-- get my orientation
	-- calculate angle between (my orientation) and vector (nextCP - myPos)
	-- transform this angle into a movement (one of 8 directions: UP, UPRIGHT, RIGHT, RIGHTDOWN, DOWN, ...
	
	-- 2) Next State
	-- if distance(myPos, nextOpponent) < distanceThreshold
	-- mCurrentState = STATE_FOLLOW
end

