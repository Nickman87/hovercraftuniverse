--[[
--	Pathfollowing AI
--	@author Dirk Delahaye, 23/03/2010
--
-- TODO:
-- (24/03/2010): Implement "arrival" behaviour.
-- DONE:
-- (07/04/2010): Abstracted throttle control
--]]
AI_NAME = "PathAI"; --Not too long, this appears in every logline
dofile("scripts/AI/Utils.lua");

-----------------------------
-- Control enum Constants
-- TODO control via enum
-----------------------------
ACCELERATE	=	1;
BRAKE		=	2;
TURNLEFT	=	3;
TURNRIGHT	=	4;
RESET		=	5;
-----------------------------


-----------------------------
-- STATE Constants
-----------------------------
ST_IDLE 	=	0;
ST_MOVING 	=	1;
-----------------------------
mCurrentState = 0;
mEntity = 0




-----------------------------
-- Path following AI Constants
-----------------------------
PATH_PROBELENGTH = 0.1;
EPSILON = 0.01; -- for avoiding steering oscillations

MINSPEED = 0.1; -- % of speed for accurate steering
MAXSPEED = 1.0; -- % of speed for going in a straight line

RESETTHRESHOLD = 200; -- distance threshold for pressing the reset button
-----------------------------





function registerController(controllerObj)
	game = controllerObj;
end

function println(msg)
	--game:luaLog(AI_NAME .. " :: " .. msg);
end

function setEntity(entity)
	mEntity = entity;
end

function toString(v)
	return "(" .. v.x .. ", " .. v.y .. ", " .. v.z .. ")";
end

--[[
-- Sets the path.
-- @param path A table of Entity objects.
-- @return void
--]]
function setPath(path)
	mPath = path;
	mPathSize = table.getn(mPath);
end

--[[
--	Controls the throttle.
--	@param	percent	The w
--]]
function speedTo(percent)
	local curSpeed = getMySpeed();
	--for now, just accelerate or do nothing
	game:setAction(BRAKE, false);
	if (curSpeed > percent) then
		game:setAction(ACCELERATE, false);
	else
		game:setAction(ACCELERATE, true);
	end
end

function getMySpeed()
	return mEntity:getSpeed() / mEntity:getMaximumSpeed();
end

function slowDown()
	if (getMySpeed() < MINSPEED) then --Do not go slower than 10%!
		speedUp();
	else
		game:setAction(BRAKE, false);
		game:setAction(ACCELERATE, false);
	end
end

function speedUp()
	if (getMySpeed() > MAXSPEED) then
		slowDown();
	else
		game:setAction(BRAKE, false);
		game:setAction(ACCELERATE, true);
	end
end

function turnLeft()
	game:setAction(TURNLEFT, true);
	game:setAction(TURNRIGHT, false);
end

function turnRight()
	game:setAction(TURNLEFT, false);
	game:setAction(TURNRIGHT, true);
end

function goStraight()
	game:setAction(TURNLEFT, false);
	game:setAction(TURNRIGHT, false);
end


--[[
--	Main function
--	Preliminary, pseudostateless AI
--]]
function decide()
	local position = mEntity:getPosition();--Vector3
	local velocity = mEntity:getVelocity();
	println("Position: " .. toString(position));
	--println("Velocity: " .. toString(velocity));
	println("Speed: " .. getMySpeed()*100 .. "%");

	--############################### BOOSTING
	--If boosting, just BOOST, don't try steering or slowing down.
	if (game:getBoosting()) then
		println("Boosting!!!");
		speedUp();
		goStraight();
		return 0;
	else
		println("Not boosting");
	end

	--############################### COLLISION AVOIDANCE
	if (false) then
	--if (mEntity:isInCollisionState()) then
		local speed = mEntity:getSpeed();
		--println("Collision Avoidance! Braking and turning right! Speed is " .. speed);
		--TODO steer the opposite way
		if (speed < 0) then
			--...
		else
			--Brake!!!!
			game:setAction(ACCELERATE, false);
			game:setAction(BRAKE, true);
			turnRight();
		end
		return 0;
	end
	--############################### END COLLISION AVOIDANCE


	--############################### PATH FOLLOWING
	local probe = position + (velocity * PATH_PROBELENGTH);
	println("Probe: " .. toString(probe));
	local distanceToPath = math.huge; --math.huge = +infinity
	local pathIndex = 0;
	for key,value in pairs(mPath) do
		if (key ~= 1) then
			local p0Vector4 = mPath[key-1];
			local p1Vector4 = mPath[key];

			local p0 = Vector3(p0Vector4.x, p0Vector4.y, p0Vector4.z);
			local p1 = Vector3(p1Vector4.x, p1Vector4.y, p1Vector4.z);


			local project = project(probe, p0, p1);
			if (probe:distance(project) < distanceToPath) then
				distanceToPath = probe:distance(project);
				pathIndex = key;
			end
		end
	end
	local p0Vector4 = mPath[pathIndex-1];
	local p1Vector4 = mPath[pathIndex];

	local p0 = Vector3(p0Vector4.x, p0Vector4.y, p0Vector4.z);
	local p1 = Vector3(p1Vector4.x, p1Vector4.y, p1Vector4.z);
	local project = project(probe, p0, p1);
	distanceToPath = probe:distance(project);

	if (distanceToPath > RESETTHRESHOLD) then
		game:setAction(RESET, true);
		println("Resetting!!");
	else
		game:setAction(RESET, false);
	end

	--println("Closest pathline lies between " .. toString(p0) .. " and " .. toString(p1));
	println("projected point ".. toString(project) .." at " .. distanceToPath .. " units distance from probe.");
	println("Closest pathline is segment[" .. (pathIndex-1) .. "->" .. pathIndex .. ", lies between " .. toString(p0) .. " and " .. toString(p1) .. ", at " .. distanceToPath .. " units distance from PROBE.");
	local targetPosition;
	local distanceThreshold = 2;

	local lastV4 = mPath[mPathSize];
	local finish = Vector3(lastV4.x, lastV4.y, lastV4.z);

	local radius = p0Vector4.w;


	if (distanceToPath > radius) then
		--Probe is too far away, steer towards path.
		--targetPosition = project;
		targetPosition = project + (p1 - project); --project: predicted position on the path, p1 = endpoint of closest pathline
		println("Steering towards path!" .. toString(targetPosition));
	else
		--No corrective steering required.
		println("No corrective steering required.");
		goStraight();
		if (position:distance(finish) > distanceThreshold) then
			speedUp();
		else
			slowDown();
		end
		return 0;
	end







	--######################################## STEERING


	--println("Corrective steering required. Target: " .. toString(targetPosition));
	targetOrientation = targetPosition - position;
	targetOrientation:normalise();
	local myOrientation = mEntity:getOrientation();
	local side = myOrientation:crossProduct(mEntity:getUpVector());
	side = side:dotProduct(targetOrientation);


	--println("MyOrientation: " .. toString(myOrientation) .. ", targetOrientation: " .. toString(targetOrientation));
	local angle = angleBetween(myOrientation, targetOrientation);
	--println("Angle is " .. angle);
	println("Side is " .. side);

	if (position:distance(finish) > distanceThreshold) then
		local angleThreshold = 90;
		if (angle > angleThreshold) then
			slowDown();
		else
			speedUp();
		end
		if (side > EPSILON) then
			turnRight();
		elseif (side < -EPSILON) then
			turnLeft();
		else
			goStraight();
		end
	else
		--todo arrival behaviour
		slowDown();
		goStraight();
	end

	return 0;
end
