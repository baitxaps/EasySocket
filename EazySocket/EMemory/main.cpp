#include<iostream>
#include"Alloctor.h"

int main()
{
	//char* dat = new char[128];
	//delete[] dat;

	//char*_dat_ = new char;
	//delete _dat_;

	//char*__dat__ = (char*)malloc(64);
	//free(__dat__);

	char* dat[12] ;
	for (int i = 0; i < 12; i++)
	{
		dat[i] = new char[60];
	}

	for (int i = 0; i < 12; i++)
	{
		delete[] dat[i];
	}

	std::cout << "hello,main "<< std::endl;
	system("pause");

	return 0;
}

