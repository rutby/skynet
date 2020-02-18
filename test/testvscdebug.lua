-- vscdebug
require("skynet.vscdebug").start()
local skynet = require "skynet"


local mode = ...

local function dosomething()
    local a = 10
    local s = "hello"
    local t = {
        name = "tom",
        age = 30,
        male = true,
        record = {10, true, "tttttt"}
    }
    local function concat(...)
        local t = {...}
        for i = 1, #t do
            t[i] = tostring(t[i])
        end
        local msg = table.concat(t, '\t')
        skynet.error(msg)
    end
    local addr = skynet.self()
    for i = 1, 10 do
        a = a + i
	end
	local msg = concat(a, s, t)
end

if mode == "slave" then -----------------------------------------

skynet.start(function()
	skynet.dispatch("lua", function(_,_, ...)
		dosomething()
		skynet.fork(function()
			while true do
				skynet.sleep(10)
			end
		end)
		skynet.retpack(...)
	end)
end)

else ------------------------------------------------------------

skynet.start(function()
	skynet.newservice("debug_console",8000)
	local slave = skynet.newservice(SERVICE_NAME, "slave")
	while true do
		local msg = skynet.call(slave, "lua", "hello world")
		skynet.error(msg)
		skynet.sleep(100)
	end
	-- local n = 1
	-- local start = skynet.now()
	-- skynet.error("call salve", n, "times in queue")
	-- for i=1, n do
	-- 	skynet.call(slave, "lua", "hello world")
	-- end
	-- skynet.error("qps = ", n/ (skynet.now() - start) * 100)

	-- start = skynet.now()

	-- local worker = 1
	-- local task = n/worker
	-- skynet.error("call salve", n, "times in parallel, worker = ", worker)

	-- for i=1,worker do
	-- 	skynet.fork(function()
	-- 		for i=1,task do
	-- 			skynet.call(slave, "lua")
	-- 		end
	-- 		worker = worker -1
	-- 		if worker == 0 then
	-- 			skynet.error("qps = ", n/ (skynet.now() - start) * 100)
	-- 		end
	-- 	end)
	-- end
end)

end
