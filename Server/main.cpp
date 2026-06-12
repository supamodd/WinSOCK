#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<iostream>
#include<cstring>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define MTU		1500
#define EXIT_COMMAND "exit"

void main()
{
	setlocale(LC_ALL, "");
	cout << "SERVER" << endl;

	INT iResult = 0;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	addrinfo hints;
	addrinfo* binder;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, "27015", &hints, &binder);
	if (iResult != 0)
	{
		cout << "getaddrinfo() failed with error " << iResult << endl;
		WSACleanup();
		return;
	}

	SOCKET listen_socket = socket(binder->ai_family, binder->ai_socktype, binder->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "SOCKET creation failed with error " << WSAGetLastError() << endl;
		freeaddrinfo(binder);
		WSACleanup();
		return;
	}

	iResult = bind(listen_socket, binder->ai_addr, binder->ai_addrlen);
	freeaddrinfo(binder);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind failed with error " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		WSACleanup();
		return;
	}

	if (listen(listen_socket, 1) == SOCKET_ERROR)
	{
		cout << "Listen failed with error " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		WSACleanup();
		return;
	}

	cout << "Ожидание подключения клиента..." << endl;

	SOCKET client_socket = accept(listen_socket, NULL, NULL);	
	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept failed with error " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		WSACleanup();
		return;
	}

	cout << "Клиент подключился." << endl;

	CHAR send_buffer[MTU] = "Hello Client!!!";
	CHAR recv_buffer[MTU] = {};

	do
	{
		ZeroMemory(recv_buffer, MTU);
		iResult = recv(client_socket, recv_buffer, MTU - 1, 0);
		if (iResult > 0)
		{
			recv_buffer[iResult] = '\0';
			cout << iResult << " Bytes received, Message: " << recv_buffer << endl;

			if (strcmp(recv_buffer, EXIT_COMMAND) == 0)
			{
				cout << "Клиент запросил отключение командой '" << EXIT_COMMAND << "'." << endl;
				break;
			}

			INT iSendResult = send(client_socket, send_buffer, strlen(send_buffer), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Send failed with error " << WSAGetLastError() << endl;
				break;
			}
			else cout << iSendResult << " Bytes send" << endl;
		}
		else if (iResult == 0) cout << "Клиент закрыл соединение." << endl;
		else cout << "Receive failed with error " << WSAGetLastError() << endl;
	} while (iResult > 0);

	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult == SOCKET_ERROR) cout << "shutdown failed with error " << WSAGetLastError() << endl;
	closesocket(client_socket);

	closesocket(listen_socket);
	WSACleanup();
}