#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	//Это нужно когда к проекту одновременно подключаются <WinSock2.h> и <Windows.h>
#endif //!WIN32_LEAN_AND_MEAN

#include<iostream>
#include<string>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")	//подгружает реализации фукций из статической библиотеки для <WS2TCPIP.h>

#define MTU		1500
#define EXIT_COMMAND "exit"

void main()
{
	setlocale(LC_ALL, "");
	INT iResult = 0;	//эта переменная нужна для отслеживания результатов выполнения функций.
	//1) Инициализация WinSOCK:
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);	//MAKEWORD(2,2) - выбираем версию WinSOCK
	if (iResult != 0)
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		return;
	}

	//2) Определяем параметры подключения:
	addrinfo hints;
	addrinfo* target;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;	//Family - семейство протоколов (стек протоколов) INET - TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &target);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error " << iResult << endl;
		WSACleanup();
		return;
	}

	SOCKET connect_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	if (iResult != 0)
	{
		cout << "Connection failed with error: " << WSAGetLastError() << endl;
		closesocket(connect_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	freeaddrinfo(target);

	cout << "Подключение к Серверу установлено." << endl;
	cout << "Введите сообщение и нажмите Enter для отправки." << endl;
	cout << "Для отключения от Сервера введите команду '" << EXIT_COMMAND << "'." << endl << endl;

	CHAR send_buffer[MTU] = {};
	CHAR recv_buffer[MTU] = {};
	string user_input;

	while (true)
	{
		cout << "Client> ";
		if (!getline(cin, user_input))
		{
			break;
		}

		if (user_input.empty()) continue;

		strncpy_s(send_buffer, MTU, user_input.c_str(), _TRUNCATE);

		iResult = send(connect_socket, send_buffer, strlen(send_buffer), NULL);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send failed with error: " << WSAGetLastError() << endl;
			break;
		}
		else cout << "Sent " << iResult << " Bytes" << endl;

		if (user_input == EXIT_COMMAND)
		{
			cout << "Отключение от Сервера по команде '" << EXIT_COMMAND << "'..." << endl;
			break;
		}

		ZeroMemory(recv_buffer, MTU);
		iResult = recv(connect_socket, recv_buffer, MTU, NULL);
		if (iResult > 0)
		{
			cout << "Server> " << recv_buffer << endl;
		}
		else if (iResult == 0)
		{
			cout << "Сервер закрыл соединение." << endl;
			break;
		}
		else
		{
			cout << "Receive failed with error: " << WSAGetLastError() << endl;
			break;
		}
	}

	//6) Разрываем TCP-соединение:
	iResult = shutdown(connect_socket, SD_BOTH);
	if (iResult != 0) cout << "shutdown failed with error " << WSAGetLastError() << endl;

	//?) Освобождаем ресурсы WinSOCK:
	closesocket(connect_socket);
	WSACleanup();
}