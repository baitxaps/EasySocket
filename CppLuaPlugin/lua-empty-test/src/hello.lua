--IDE tool: babelua
-- CC_USE_DEPRECATED_API = true
require "cocos.init"
require "CellClient"
require	"CellWriteStream"
require	"CellReadStream"

NetCmd = 
{
	LOGIN = 10,
	LOGIN_RESULT = 11,
	LOGOUT = 12,
	LOGOUT_RESULT = 13,
	NEW_USER_JOIN = 14,
	C2S_HEART = 15,
	S2C_HEART = 16,
	ERROR = 17
}

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
	---------------
    require "hello2"
    cclog("result is " .. myadd(1, 1))
	---------------Test
	cclog("result is " .. Add(1,2))

	local str = SayHi("DuoYou")
	cclog(str)

	local function callback(str)
		cclog(str)
    end

	TestCall("China",callback);
    ---------------CELLClient
	--处理网络消息
	local function callback_OnNetMsg(data,len)
		cclog("callback_OnNetMsg: len=" .. len)
		local rs = CellReadStream.new(data);
		--读取消息长度
		rs:ReadInt16(rs);
		--读取消息命令
		rs:getNetCmd(rs);
		cclog(rs:ReadInt8(rs));
		cclog(rs:ReadInt16(rs));
		cclog(rs:ReadInt32(rs));
		cclog(rs:ReadFloat(rs));
		cclog(rs:ReadDouble(rs));
		cclog(rs:ReadString(rs));
		cclog(rs:ReadString(rs));
		local arr,len = rs:ReadInt32s(rs)
		for n=1,len do
			cclog(arr[n])
		end
		rs:release()
    end

	local client = CellClient.new(callback_OnNetMsg, 51200,51200);

	client:Connect("192.168.0.107", 4567);



	--every frame
    local function tick()
		local ws = CellWriteStream.new(128);
		ws:setNetCmd(NetCmd.LOGOUT);
		ws:WriteInt8(1);
		ws:WriteInt16(2);
		ws:WriteInt32(3);
		ws:WriteFloat(4.5);
		ws:WriteDouble(6.7);
		ws:WriteString("username1");
		ws:WriteString("password1");
		--写入数组
		local arr = {5,6,7,8,9,10}
		ws:WriteInt32s(arr);
		client:SendWriteStream(ws)
		ws:release()
		client:OnRun();
    end

    cc.Director:getInstance():getScheduler():scheduleScriptFunc(tick, 0, false)
    -- run
    local sceneGame = cc.Scene:create()
    cc.Director:getInstance():runWithScene(sceneGame)
end

xpcall(main, __G__TRACKBACK__)
