#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include"EasyTcpClient.hpp"


class HelloWorld : public cocos2d::Scene , public EasyTcpClient
{
public:
    virtual bool init() override;

    static cocos2d::Scene* scene();

    // a selector callback
    void menuCloseCallback(Ref* sender);

    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

	///-------------------------------
	virtual void OnNetMsg(netmsg_DataHeader* header);

	void update(float dt);
};

#endif // __HELLOWORLD_SCENE_H__
