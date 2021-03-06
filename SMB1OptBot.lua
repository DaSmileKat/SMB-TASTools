-- This is the SMB1 Movement Optimization Bot for use in FCEUX
-- You must start running the bot from the savestate that the bot is supposed to start each attempt at
-- Not all movements can be effectively optimized by this bot, only the movements with an effective isBetter function (see below)
-- Further optimizations are welcomed
-- For some reason GitHub interprets tabs as 8 spaces rather than 4, so the indents are messed up in some places

-- Input codes: 0: B, 1: B+R, 2: B+L, 3: B+L+R, 4: B+A, 5: B+R+A, 6: B+L+A, 7: B+L+R+A, 8~15 is same as 0~7 except you don't hold B, 16: D
-- You can change this, but make sure to also change the findNextValid function
local inputtable = {{ up = false, down = false, left = false, right = false, A = false, B = true, start = false, select = false },
					{ up = false, down = false, left = false, right = true, A = false, B = true, start = false, select = false },
					{ up = false, down = false, left = true, right = false, A = false, B = true, start = false, select = false },
					{ up = false, down = false, left = true, right = true, A = false, B = true, start = false, select = false },
					{ up = false, down = false, left = false, right = false, A = true, B = true, start = false, select = false },
					{ up = false, down = false, left = false, right = true, A = true, B = true, start = false, select = false },
					{ up = false, down = false, left = true, right = false, A = true, B = true, start = false, select = false },
					{ up = false, down = false, left = true, right = true, A = true, B = true, start = false, select = false },
					{ up = false, down = false, left = false, right = false, A = false, B = false, start = false, select = false },
					{ up = false, down = false, left = false, right = true, A = false, B = false, start = false, select = false },
					{ up = false, down = false, left = true, right = false, A = false, B = false, start = false, select = false },
					{ up = false, down = false, left = true, right = true, A = false, B = false, start = false, select = false },
					{ up = false, down = false, left = false, right = false, A = true, B = false, start = false, select = false },
					{ up = false, down = false, left = false, right = true, A = true, B = false, start = false, select = false },
					{ up = false, down = false, left = true, right = false, A = true, B = false, start = false, select = false },
					{ up = false, down = false, left = true, right = true, A = true, B = false, start = false, select = false },
					{ up = false, down = true, left = false, right = false, A = false, B = false, start = false, select = false }}

-- Function copied from https://gist.github.com/tylerneylon/81333721109155b2d244
function copy(obj)
    if type(obj) ~= 'table' then return obj end
    local res = {}
    for k, v in pairs(obj) do res[copy(k)] = copy(v) end
    return res
end

-- Base Movement Optimization Bot (Do not change)

local OptBot = {}

-- frames: the number of frames the bot should bot, initial: the initial optimal end state that the bot would compare its solutions to,
-- debug: the number of cases after which the bot shows the current inputs that it is testing, elapsed time, and cases per second, set it to 1 if you don't want that
function OptBot:new(frames, initial, debug)
	local state = savestate.create(1)
	savestate.save(state)
	local inputs = {}
    local bot = { states = {}, frames = frames, frame = frames, pframe = frames, inputs = {}, starttime = os.clock(),
				  state = state, optimal = initial, optinputs = {}, debug = debug, cases = 0, done = false }
	for i = 1, frames, 1 do
		bot.inputs[i] = 0
	end
    setmetatable(bot, self)
	self.__index = self
    return bot
end

-- All functions from here up to OptBot:run must be overridden
-- This function finds what the current input should become, and if it returns 0, the bot tries to change the previous input
function OptBot:findNextValid(input)
	return 1
end

-- This function gets the current state of the game, only the necessary values should be in the state
function OptBot:getCurrentState()
	return {}
end

-- This function gets a key for the current state, which is used to quickly search for better states,
-- and it should be unique for every set of values that must be equal in order to compare two states (see below)
-- The key should preferably be a decimal (for example by adding 0.01) so as to not waste space from Lua thinking that the states table is an array rather than a hash table
function OptBot:getKeyFromState(cstate)
	return 1
end

-- This function decides if state1 can be considered to be better than state2, it is not required (and in fact, usually impossible) for any two states to be comparable
-- This is arguably the most important customizable function of the entire bot, since this is the main heuristic algorithm
-- The final efficiency of the bot mostly depends on the effectiveness of this function, so try to make this as effective as possible
-- If there exist a reached state that is better than the current state, the bot assumes thats
-- no further inputs from here will result in a better solution, and try to change the previous inputs
-- Return true if state1 is better than state2, false if state2 is better than state1 or if they're not comparable,
-- and return 1 if you want the bot to continue trying these inputs but not add the current state to the reached states table
function OptBot:isBetter(state1, state2)
	return false
end

-- This function should return true when no further inputs from the current state should result in a better solution
-- This only deals with the cases that are not already dealt with in the state comparison heuristic alrogithm
function OptBot:furtherHeuristics(state)
	return false
end

-- This function should return true when the state after the current sequence of inputs is the best so far
function OptBot:isBestSolution(cstate, optimal)
	return false
end

function OptBot:run()
	emu.speedmode("turbo")
	self.states = {}
	self.done = false

	while true do	
		savestate.load(self.state)
		for i = self.frame + 1, self.frames, 1 do
			self.inputs[i] = 0
		end
		self.cases = self.cases + 1
		if self.cases % self.debug == 1 then
			print("================================")
			print(tostring(self.inputs))
			print(string.format("total cases: %d\n", self.cases))
			print(string.format("elapsed time: %.2f\n", os.clock() - self.starttime))
		end
	
		for i = self.frame, 1, -1 do
			local newinput = self:findNextValid(self.inputs[i])
			self.inputs[i] = newinput
			if newinput ~= 0 then
				pframe = i
				break
			end
			if i == 1 then
				self.done = true
			end
		end
		if self.done then
			break
		end
	
		for i = 1, self.frames, 1 do
	
			self.frame = i
			joypad.set(1, inputtable[self.inputs[self.frame] + 1])
			emu.frameadvance()
	
			local cstate = self:getCurrentState()
			local key = self:getKeyFromState(cstate)
			local keep = true
			local dobreak = false
			local j = 0

			if self:furtherHeuristics(cstate) then
				break
			end

			self.states[key] = self.states[key] or {}
			for k, v in pairs(self.states[key]) do
				if self:isBetter(v, cstate) == true and self.frame >= pframe then
					keep = false
					dobreak = true
					break
				end
				if self:isBetter(cstate, v) == true then
					self.states[key][k] = cstate
					keep = false
					break
				elseif self:isBetter(cstate, v) == 1 then
					keep = false
				end
				j = k
			end
	
			if keep then
				self.states[key][j + 1] = cstate
			end
			if dobreak then
				break
			end			
				
			if self:isBestSolution(cstate, self.optimal) then
				self.optimal = copy(cstate)
				self.optinputs = copy(self.inputs)
				print("*************** NEW BEST ****************")
				print(tostring(cstate))
				print(tostring(self.inputs))
			end
		end
	end

	-- emu.speedmode("normal")
	print("---------------- DONE -----------------")
	print(tostring(self.optimal))
	print(tostring(self.optinputs))
	-- while true do
		-- emu.frameadvance()
	-- end
end

-- Fast Acceleration Bot (Example)

local FABot = OptBot:new(1)

function FABot:findNextValid(input)
	if input >= 0 and input < 6 then
		return input + 1
	elseif input == 6 then
		return 8
	elseif input >= 8 and input < 11 then
		return input + 1
	else
		return 0
	end
end

function FABot:getCurrentState()
	return { frame = self.frame,
			 xpos = memory.readbyte(0x0086) * 16 + memory.readbyte(0x0400) / 16,
			 xspeed = memory.readbytesigned(0x0057) * 64 + memory.readbyte(0x0705) / 4,
			 --ypos = memory.readbyte(0x00ce) * 256 + memory.readbyte(0x0416),
			 --yspeed = memory.readbytesigned(0x009f) * 256 + memory.readbyte(0x0433), (the ypos and yspeed actually doesnt matter for fa)
			 facing = memory.readbyte(0x0033) }
end

function FABot:getKeyFromState(cstate)
	return cstate.facing + 0.01
end

function FABot:isBetter(state1, state2)
	return state1.xpos >= state2.xpos and state1.xspeed >= state2.xspeed and state1.frame <= state2.frame
end

function FABot:furtherHeuristics(state)
	return state.xpos + 40 * (self.frames - self.frame) < self.optimal.xpos
end

function FABot:isBestSolution(cstate)
	return self.frame == self.frames and cstate.xpos > self.optimal.xpos
end

-- FABot:new(35, { xpos = 0 }, 50):run()

-- Flagpole Glitch Bot (Example)

local FPGBot = OptBot:new(1)

function FPGBot:findNextValid(input)
	if input >= 0 and input < 6 then
		return input + 1
	elseif input == 6 then
		return 8
	elseif input >= 8 and input < 11 then
		return input + 1
	else
		return 0
	end
end

function FPGBot:getCurrentState()
	return { frame = self.frame,
			 xpos = memory.readbyte(0x0086) * 16 + memory.readbyte(0x0400) / 16,
			 xspeed = memory.readbytesigned(0x0057) * 64 + memory.readbyte(0x0705) / 4,
			 xaccel = memory.readbyte(0x0701) * 64 + memory.readbyte(0x0702) / 4,
			 facing = memory.readbyte(0x0033),
			 playerstate = memory.readbyte(0x001d),
			 startfacing = self.facing,
			 startsubspeed = self.subspeed }
end

function FPGBot:getKeyFromState(cstate)
	return (cstate.xspeed + 0x700) * 0x800 + cstate.xaccel * 4 + cstate.facing + 0.01
end

function FPGBot:isBetter(state1, state2)
	return state1.xpos >= state2.xpos and state1.frame <= state2.frame
end

function FPGBot:furtherHeuristics(state)
	return false
end

function FPGBot:isBestSolution(cstate)
	return self.frame == self.frames and cstate.playerstate == 3 and cstate.xpos > self.optimal.xpos
end

function FPGBot:run2()
	self.facing = 1
	self.subspeed = 0
	while self.facing <= 2 do
		while self.subspeed <= 252 do
			savestate.load(self.state)
			memory.writebyte(0x0033, self.facing)
			memory.writebyte(0x0705, self.subspeed)
			local newstate = savestate.create(1)
			savestate.save(newstate)
			self.state = newstate
			local optimalprev = copy(self.optimal)
			self:run()
			self.subspeed = self.subspeed + 4
		end
		self.facing = self.facing + 1
	end
	emu.speedmode("normal")
	print("---------------- REALLY DONE -----------------")
	print(tostring(self.optimal))
	print(tostring(self.optinputs))
	while true do
		emu.frameadvance()
	end
end

-- FPGBot:new(6, { xpos = 0, playerstate = 0 }, 500):run2()

-- 4-2 Coin Clip Bot

local CCBot = OptBot:new(1)

function CCBot:findNextValid(input)
	if self.frame <= 19 then
		if input < 2 then
			return input + 1
		else
			return 0
		end
	else
		if input >= 0 and input < 6 then
			return input + 1
		elseif input == 6 then
			return 8
		elseif input >= 8 and input < 11 then
			return input + 1
		else
			return 0
	end
	end
end

function CCBot:getCurrentState()
	return { frame = self.frame,
			 screenx = memory.readbyte(0x03ad) * 16 + memory.readbyte(0x0400) / 16,
			 xspeed = memory.readbytesigned(0x0057) * 64 + memory.readbyte(0x0705) / 4,
			 xaccel = memory.readbyte(0x0701) * 64 + memory.readbyte(0x0702) / 4,
			 facing = memory.readbyte(0x0033),
			 coltimer = memory.readbyte(0x0785),
			 startsubpix = self.subpix,
			 startsubspeed = self.subspeed }
end

function CCBot:getKeyFromState(cstate)
	return cstate.xaccel * 4 + cstate.facing
end

function CCBot:isBetter(state1, state2)
	return state1.screenx >= state2.screenx and state1.xspeed >= state2.xspeed and state1.frame <= state2.frame
end

function CCBot:furtherHeuristics(state)
	return false
end

function CCBot:isBestSolution(cstate)
	return cstate.coltimer == 1 and cstate.screenx > self.optimal.screenx
end

function CCBot:run2()
	self.subpix = 0
	self.subspeed = 0
	while self.subpix < 16 do
		while self.subspeed <= 252 do
			savestate.load(self.state)
			memory.writebyte(0x0400, self.subpix)
			memory.writebyte(0x0705, self.subspeed)
			local newstate = savestate.create(1)
			savestate.save(newstate)
			self.state = newstate
			local optimalprev = copy(self.optimal)
			self:run()
			self.subspeed = self.subspeed + 4
		end
		self.subpix = self.subpix + 1
	end
	emu.speedmode("normal")
	print("---------------- REALLY DONE -----------------")
	print(tostring(self.optimal))
	print(tostring(self.optinputs))
	while true do
		emu.frameadvance()
	end
end

CCBot:new(24, { screenx = 0 }, 500):run2()
