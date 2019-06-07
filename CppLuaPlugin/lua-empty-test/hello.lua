-- CC_USE_DEPRECATED_API = true
require "cocos.init"

-- cclog
cclog = function(...)
    print(string.format(...))
end

-- for CCLuaEngine traceback
function __G__TRACKBACK__(msg)
    cclog("----------------------------------------")
    cclog("LUA ERROR: " .. tostring(msg) .. "\n")
    cclog(debug.traceback())
    cclog("----------------------------------------")
end

local function initGLView()
    local director = cc.Director:getInstance()
    local glView = director:getOpenGLView()
    if nil == glView then
        glView = cc.GLViewImpl:create("Lua Empty Test")
        director:setOpenGLView(glView)
    end

    director:setOpenGLView(glView)

    glView:setDesignResolutionSize(480, 320, cc.ResolutionPolicy.NO_BORDER)

    --turn on display FPS
    director:setDisplayStats(true)

    --set FPS. the default value is 1.0/60 if you don't call this
    director:setAnimationInterval(1.0 / 60)
end

local function main()
    -- avoid memory leak
    collectgarbage("setpause", 100)
    collectgarbage("setstepmul", 5000)

    initGLView()
-----------------------
    require "hello2"
    cclog("result is " .. myadd(1, 1))
	-----------------------
	cclog("result is " .. Add(1, 1))
	cclog(SayString("DouYOU"))

	local function callBack(str)
		 cclog("result is " .. Add(1, 1))
	end
	TestCall("China",callBack)

	-----------------------
	local function tick()
        
	end
	 -- run
	cc.Director:getInstance():getScheduler():scheduleScriptFunc(tick,0,false);
    local sceneGame = cc.Scene:create()
    cc.Director:getInstance():runWithScene(sceneGame)
end

xpcall(main, __G__TRACKBACK__)
