#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	//Это нужно когда к проекту одновременно подключаются <WinSock2.h> и <Windows.h>
#endif //!WIN32_LEAN_AND_MEAN

#include<iostream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<FormatLastError.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")	//подгружает реализации фукций из статической библиотеки для <WS2TCPIP.h>
#pragma comment(lib, "FormatLastError.lib")

#define MTU		1500

VOID Receive(SOCKET connect_socket);

void main()
{
	setlocale(LC_ALL, "");
	INT iResult = 0;	//эта переменная нужна для отслеживания результатов выполнения функций.
	DWORD dwError = 0;
	CHAR szError[USHRT_MAX + 1] = {};

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

	iResult = getaddrinfo("192.168.1.101", "27015", &hints, &target);//По символьному имени получает числовой адрес целевого узла (IP-адрес)
	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) Создаем сокет для подключения к Серверу:
	SOCKET connect_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	dwError = WSAGetLastError();
	if (connect_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error: " << WSAGetLastError() << endl;
		cout << FormatLastError(szError, dwError) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//4) Подключаемся к Серверу:
	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	dwError = WSAGetLastError();
	if (iResult != 0)
	{
		cout << "Connection failed with error: " << dwError << endl;
		cout << FormatLastError(szError, dwError) << endl;
		/*LPSTR lpError;
		FormatMessage
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, LANG_SYSTEM_DEFAULT),
			(LPSTR)&lpError,
			NULL,
			NULL
		);
		cout << lpError << endl;
		LocalFree(lpError);*/
		closesocket(connect_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	freeaddrinfo(target);
	DWORD dwThreadID = 0;
	HANDLE hReceiveThread = CreateThread
	(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)Receive,
		(LPVOID)connect_socket,
		NULL,
		&dwThreadID
	);
	//5) Отправка данных Серверу:
	CHAR send_buffer[MTU] = "Hello Server!!!";
	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), NULL);
		dwError = WSAGetLastError();
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send failed with error: " << WSAGetLastError() << endl;
			cout << FormatLastError(szError, dwError) << endl;
			closesocket(connect_socket);
			WSACleanup();
			return;
		}
		else cout << "Sent " << iResult << " Bytes" << endl;


		//cout << "Введите сообщение: ";
		SetConsoleCP(1251);
		cin.getline(send_buffer, MTU);
		SetConsoleCP(866);
	} while (strcmp(send_buffer, "exit") != 0);

	//7) Разрываем TCP-соединение:
	iResult = shutdown(connect_socket, SD_BOTH);
	if (iResult != 0)cout << FormatLastError(szError, WSAGetLastError()) << endl;;;;;;;;;;//cout << "shutdown failed with error " << WSAGetLastError() << endl;

	//?) Освобождаем ресурсы WinSOCK:
	closesocket(connect_socket);
	WSACleanup();
}
VOID Receive(SOCKET connect_socket)
{
	//6) Получение данных от Сервера:
	INT iResult = 0;
	CHAR recv_buffer[MTU] = {/*initializer_list*/ };
	do
	{
		ZeroMemory(recv_buffer, sizeof(recv_buffer));
		iResult = recv(connect_socket, recv_buffer, MTU, NULL);
		if (iResult > 0)cout << recv_buffer << endl;
		else if (iResult == 0) cout << "Nothing received from Server" << endl;
		else cout << "Receive failed with error: " << WSAGetLastError() << endl;
	} while (true);
}