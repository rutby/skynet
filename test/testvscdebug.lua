-- vscdebug
local skynet = require "skynet"
local vscdebug = require("skynet.vscdebug")
vscdebug.start()

local function test()
    local a = 2
    local myname = {
        a = 1,
        b = 2,
        c = myname, 
    }
    table.insert(myname, "aabb")
    if a == 2 and myname then
        skynet.error("OK")
    end
 end

skynet.start(function()
    skynet.error("Hello   ")
    test()
    for i = 1, 4 do
        print(">>>>>>", i)
    end
end)