#include "HelloWorldScene.h"
#include "AppMacros.h"
#include"CELLMsgStream.hpp"
USING_NS_CC;


Scene* HelloWorld::scene()
{
     return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        CC_CALLBACK_1(HelloWorld::menuCloseCallback,this));
    
    closeItem->setPosition(origin + Vec2(visibleSize) - Vec2(closeItem->getContentSize() / 2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/arial.ttf", TITLE_FONT_SIZE);
    
    // position the label on the center of the screen
    label->setPosition(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height);

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize / 2) + origin);

    // add the sprite as a child to this layer
    this->addChild(sprite);

	//---------------------------------------
	scheduleUpdate();
	this->Connect("192.168.0.107", 4567);
    
    return true;
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

//响应网络消息
 void  HelloWorld::OnNetMsg(netmsg_DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGOUT_RESULT:
	{
		CELLReadStream r(header);
		//读取消息长度
		r.ReadInt16();
		//读取消息命令
		r.getNetCmd();
		auto n1 = r.ReadInt8();
		auto n2 = r.ReadInt16();
		auto n3 = r.ReadInt32();
		auto n4 = r.ReadFloat();
		auto n5 = r.ReadDouble();
		uint32_t n = 0;
		r.onlyRead(n);
		char name[32] = {};
		auto n6 = r.ReadArray(name, 32);
		char pw[32] = {};
		auto n7 = r.ReadArray(pw, 32);
		int ata[10] = {};
		auto n8 = r.ReadArray(ata, 10);
		CELLLog::Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());
	}
	break;
	case CMD_ERROR:
	{
		CELLLog::Info("<socket=%d> recv msgType：CMD_ERROR\n", (int)_pClient->sockfd());
	}
	break;
	default:
	{
		CELLLog::Info("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
	}
	}
}


 void HelloWorld::update(float dt)
 {
	 this->OnRun();

	 CELLWriteStream s(128);
	 s.setNetCmd(CMD_LOGOUT);
	 s.WriteInt8(1);
	 s.WriteInt16(2);
	 s.WriteInt32(3);
	 s.WriteFloat(4.5f);
	 s.WriteDouble(6.7);
	 s.WriteString("client");

	 char a[] = "ahah";
	 s.WriteArray(a, strlen(a));
	 int b[] = { 1,2,3,4,5 };
	 s.WriteArray(b, 5);
	 s.finsh();
	 this->SendData(s.data(), s.length());
 }