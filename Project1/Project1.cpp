// Project1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "TestCqClass.h"
#include <iostream>
#include <conio.h>

TestCqClass* cqclass;

void thSendMsg()
{
	auto group = 904511841;
	auto _friend = 1391525377;
	std::string msg;
	while (true)
	{
		char ch = _getche();
		switch (ch)
		{
		case 'p':
		case 'P':
			std::getline(std::cin, msg);
			cqclass->send_private_msg(_friend, 0, msg);
			break;
		case 'g':
		case 'G':
			std::getline(std::cin, msg);
			cqclass->send_group_msg(group, msg);
			break;
		default:
			break;
		}
	}
}

int main()
{
	cqclass = new TestCqClass("127.0.0.1", "6700");
	std::thread th(thSendMsg);
	th.join();
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
