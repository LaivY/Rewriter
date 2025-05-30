#include "Stdafx.h"
#include "ClientSocket.h"
#include "SocketManager.h"
#include "User.h"
#include "UserManager.h"
#ifdef _IMGUI
#include "Common/Time.h"
#endif

SocketManager::SocketManager() :
	m_iocp{ INVALID_HANDLE_VALUE },
	m_listenSocket{ INVALID_SOCKET },
	m_acceptSocket{ INVALID_SOCKET },
	m_acceptBuffer{},
	m_acceptOverlappedEx{}
{
	WSADATA wsaData{};
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		assert(false && "WSA INIT FAIL");
		return;
	}

	m_listenSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		assert(false && "CREATE LISTEN SOCKET FAIL");
		return;
	}

	auto prop{ Resource::Get(L"Server.dat/Login") };
	if (!prop)
	{
		assert(false && "CAN NOT FIND SERVER CONFIG");
		return;
	}

	SOCKADDR_IN addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(prop->GetInt(L"Port"));
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	if (::bind(m_listenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		assert(false && "LISTEN SOCKET BIND FAIL");
		return;
	}

	BOOL option{ TRUE };
	if (::setsockopt(m_listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, reinterpret_cast<char*>(&option), sizeof(option)) == SOCKET_ERROR)
	{
		assert(false && "LISTEN SOCKET SETSOCKOPT FAIL");
		return;
	}

	if (::listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		assert(false && "LISTEN SOCKET LISTEN FAIL");
		return;
	}

	m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (!m_iocp)
	{
		assert(false && "SOCKET MANAGER CREATE IOCP FAIL");
		return;
	}

	if (m_iocp != ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listenSocket), m_iocp, 0, 0))
	{
		assert(false && "REGISTER IOCP FAIL");
		return;
	}

	for (auto& thread : m_threads)
		thread = std::jthread{ std::bind_front(&SocketManager::Run, this) };

	Accept();
}

SocketManager::~SocketManager()
{
	for (auto& thread : m_threads)
		thread.request_stop();
	::CloseHandle(m_iocp);
	::WSACleanup();
}

void SocketManager::Render()
{
#ifdef _IMGUI
	if (ImGui::Begin("Socket Manager"))
	{
		for (const auto& log : m_logs)
			ImGui::TextUnformatted(log.c_str());
	}
	ImGui::End();
#endif
}

bool SocketManager::Register(ISocket* socket) const
{
	auto handle{ ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(static_cast<SOCKET>(*socket)), m_iocp, reinterpret_cast<ULONG_PTR>(socket), 0) };
	return m_iocp == handle;
}

void SocketManager::Disconnect(ISocket* socket)
{
	Log(std::format("Socket is disconnected | Type : {} | IP : {}", static_cast<int>(socket->GetType()), socket->GetIP()));

	socket->OnDisconnect();
	std::erase_if(m_sockets, [socket](const auto& s) { return s.get() == socket; });
}

std::shared_ptr<ISocket> SocketManager::GetSocket(ISocket::ID id) const
{
	auto it{ std::ranges::find_if(m_sockets, [id](const auto& s) { return s->GetID() == id; }) };
	if (it == m_sockets.end())
		return nullptr;
	return *it;
}

void SocketManager::Log(const std::string& log)
{
#ifdef _IMGUI
	Time now{ Time::Now() };
	std::string prefix{ std::format("[{}-{:02}-{:02} {:02}:{:02}:{:02}] ", now.Year(), now.Month(), now.Day(), now.Hour(), now.Min(), now.Sec()) };
	m_logs.push_back(prefix + log);
#endif
}

void SocketManager::Run(std::stop_token stoken)
{
	DWORD ioSize{};
	ISocket* socket{};
	ISocket::OverlappedEx* overlappedEx{};
	while (!stoken.stop_requested())
	{
		if (::GetQueuedCompletionStatus(m_iocp, &ioSize, reinterpret_cast<PULONG_PTR>(&socket), reinterpret_cast<OVERLAPPED**>(&overlappedEx), INFINITE))
		{
			if (!overlappedEx)
				continue;

			if (ioSize == 0)
			{
				switch (overlappedEx->op)
				{
				case ISocket::IOOperation::Connect:
				{
					if (socket)
						OnConnect(socket, true);
					break;
				}
				case ISocket::IOOperation::Accept:
				{
					OnAccept();
					break;
				}
				case ISocket::IOOperation::Send:
				case ISocket::IOOperation::Receive:
				{
					if (socket)
						Disconnect(socket);
					break;
				}
				default:
					break;
				}
				continue;
			}

			if (!socket)
				continue;

			switch (overlappedEx->op)
			{
			case ISocket::IOOperation::Send:
			{
				socket->OnSend(overlappedEx);
				break;
			}
			case ISocket::IOOperation::Receive:
			{
				socket->OnReceive(static_cast<Packet::Size>(ioSize));
				break;
			}
			default:
				assert(false && "INVALID SOCKET STATE");
				continue;
			}
			continue;
		}

		int error{ ::WSAGetLastError() };
		switch (error)
		{
		case ERROR_NETNAME_DELETED: // 클라이언트에서 강제로 연결 끊음
		{
			if (socket)
				Disconnect(socket);
			continue;
		}
		case ERROR_ABANDONED_WAIT_0: // IOCP 핸들 닫힘
		case ERROR_OPERATION_ABORTED: // IO 작업 취소됨
		{
			continue;
		}
		case ERROR_CONNECTION_REFUSED: // 연결 실패
		{
			if (socket)
				OnConnect(socket, false);
			continue;
		}
		case ERROR_CONNECTION_ABORTED: // 내가 연결 끊음
		{
			continue;
		}
		default:
			assert(false && "IOCP ERROR");
			continue;
		}
	}
}

void SocketManager::OnConnect(ISocket* socket, bool success)
{
	socket->OnConnect(success);
	if (success)
		Log(std::format("Socket is connected | Type : {} | IP : {}", static_cast<int>(socket->GetType()), socket->GetIP()));
}

void SocketManager::OnAccept()
{
	std::lock_guard lock{ m_acceptMutex };

	do
	{
		// 소켓 옵션 설정
		if (::setsockopt(m_acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&m_listenSocket), sizeof(m_listenSocket)) == SOCKET_ERROR)
		{
			assert(false && "UPDATE ACCEPT CONTEXT FAIL");
			break;
		}

		// 소켓 추가
		auto clientSocket{ std::make_shared<ClientSocket>(m_acceptSocket) };
		if (!Register(clientSocket.get()))
		{
			assert(false && "REGISTER SOCKET TO IOCP FAIL");
			break;
		}

		clientSocket->Receive();
		m_sockets.push_back(clientSocket);

		Log(std::format("Socket is connected | Type : {} | IP : {}", static_cast<int>(clientSocket->GetType()), clientSocket->GetIP()));
	} while (false);

	Accept();
}

void SocketManager::Accept()
{
	std::lock_guard lock{ m_acceptMutex };

	m_acceptBuffer.fill(0);
	m_acceptOverlappedEx = { .op = ISocket::IOOperation::Accept };
	m_acceptSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (m_acceptSocket == INVALID_SOCKET)
	{
		assert(false && "CREATE ACCEPT SOCKET FAIL");
		return;
	}

	if (!::AcceptEx(m_listenSocket, m_acceptSocket, m_acceptBuffer.data(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, nullptr, &m_acceptOverlappedEx))
	{
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			assert(false && "ACCEPT FAIL");
			return;
		}
	}
}
