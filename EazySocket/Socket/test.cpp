#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
#endif
#include<iostream>
#include<windows.h>
#include<WinSock2.h>
#include<functional>

//#pragma comment(lib,"ws2_32.lib")
using namespace std;

void funT()
{
	printf("funT\n");
}

int funp(int t)
{
	printf("funP\n");
	return t;
}

int funm(int t,int m)
{
	printf("funP\n");
	return m;
}

void lambda()
{
	printf("funP\n");
}

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSAData dat;
	WSAStartup(ver, &dat);

	/* 
	lambda sentance express:拉曼达表达式 匿名函数
	[caputure](parmas) opt-> ret｛body;｝
	[外部变量捕获列表](参数表) 特殊操作符->返回值类型{ 函数体; };

	捕获列表：lambda表达式的捕获列表精细控制了lambda表达式能够访问的外部变量，以及如何访问这些变量。
	1) []不捕获任何变量。
	2) [&]捕获外部作用域中所有变量，并作为引用在函数体中使用（按引用捕获）。
	3) [=]捕获外部作用域中所有变量，并作为副本在函数体中使用(按值捕获)。
	4) [=, &foo]按值捕获外部作用域中所有变量，并按引用捕获foo变量。
	5) [bar]按值捕获bar变量，同时不捕获其他变量。
	6) [this]捕获当前类中的this指针，让lambda表达式拥有和当前类成员函数同样的访问权限。
	如果已经使用了&或者 = ，就默认含有此选项。
	捕获this的目的是可以在lamda中使用当前类的成员函数和成员变量。

	1).capture是捕获列表；
	2).params是参数表；(选填)
	3).opt是函数选项；可以填mutable,exception,attribute（选填）
	mutable说明lambda表达式体内的代码可以修改被捕获的变量，并且可以访问被捕获的对象的non-const方法。
	exception说明lambda表达式是否抛出异常以及何种异常。
	attribute用来声明属性。
	4).ret是返回值类型。(选填)
	5).body是函数体。
	*/
	int n=10;
	std::function<int()>call;
	call = [n]() -> int {
		printf("lambda= %d\n",n);
		return 2;
	};
	int p = call();

	//function< void() > call = funT;
	//call();

	//function<int (int)> callp = funp;
	//int n =callp(0);

	//function<int(int,int)> callm = funm;
	//int m = callm(0,0);

	WSACleanup();
	system("pause");
	return 0;
}
