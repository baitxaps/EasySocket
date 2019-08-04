#ifndef _NETWORKMANAGER_HPP_
#define _NETWORKMANAGER_HPP_

#include<iostream>

std::mutex resourceLock;
std::once_flag g_flag;

class NetworkManager
{
	static void CreateInstance()
	{
		m_instance = new NetworkManager();
		static CGDealloc de;
	}

private:
	
	NetworkManager() {}
private:
	static NetworkManager *m_instance;
	
public:
	static NetworkManager* GetInstance()
	{
		if (m_instance == NULL)
		{
			std::unique_lock<std::mutex> lock(resourceLock);
			if (m_instance == NULL)
			{
				m_instance = new NetworkManager();
				static CGDealloc de;
			}
		}
		return m_instance;
	}

	static NetworkManager* GetCallonceInstance()
	{
		std::call_once(g_flag, CreateInstance);
		return m_instance;
	}

	class CGDealloc
	{
	public:
		~CGDealloc()//单例释放
		{
			delete NetworkManager::m_instance;
			NetworkManager::m_instance = NULL;
		}
	};

	void test()
	{
		std::cout << "test" << std::endl;
	}
};


#endif