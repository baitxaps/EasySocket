#ifndef _CPP_NETDLL_H_

extern "C"
{
	int __declspec(dllexport) Add(int a, int b)
	{
		return a + b;
	}
}

#endif // !_CPP_NETDLL_H_