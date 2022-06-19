#include "ServerConnection.h"
#include "DevConfigManager.h"
#include "../Shared/IO/TCPMessages.h"
#include "../Shared/IO/TCPPacket.h"

bool DevTool::Server::Connect()
{
	socket_.Initialize();
	if (!socket_.Open("127.0.0.1", DevTool::DevtoolConfigurationManager::Get().ServerPort))
	{
		Disconnect();
		return false;
	}

	return true;
}

void DevTool::Server::Disconnect()
{
	socket_.Close();
}

bool DevTool::Server::Login()
{
	ByteStream stream;

	uint8 request_type = DevCommandTypes::Login;
	stream.Write<uint8>(&request_type, 1);

	uint16 string_length = DevtoolConfigurationManager::Get().LoginUserName.length();
	stream.Write<uint16>(&string_length, 1);
	stream.Write<char>((void*)DevtoolConfigurationManager::Get().LoginUserName.c_str(), DevtoolConfigurationManager::Get().LoginUserName.length());

	string_length = DevtoolConfigurationManager::Get().LoginPassword.length();
	stream.Write<uint16>((char*)&string_length, 1);
	stream.Write<char>((void*)DevtoolConfigurationManager::Get().LoginPassword.c_str(), DevtoolConfigurationManager::Get().LoginPassword.length());

	ByteStream response = SendPacket(stream.Data(), stream.Size());

	if (response.Size() == sizeof(PacketHeader) + 2)
	{
		response.Skip<PacketHeader>(1);
		uint8 command_type, result_type;
		response.Read<uint8>(&command_type, 1);
		response.Read<uint8>(&result_type, 1);

		return command_type == DevCommandTypes::Login && result_type == ResultTypes::AUTHORIZED_YES;
	}

	return false;
}

ResultTypes DevTool::Server::RemoteCompile(std::string Database, std::string Filters, std::string OutputName, std::vector<CompilationStageInfo> Stages, std::string& compiler_output)
{
	compiler_output = "";

	ByteStream stream;

	stream.Write<uint8>(DevCommandTypes::RequestCompile);

	stream.Write<uint8>(Database.length());
	stream.Write<char>((void*)Database.c_str(), Database.length());

	stream.Write<uint8>(Filters.length());
	if(!Filters.empty()) stream.Write<char>((void*)Filters.c_str(), Filters.length());

	stream.Write<uint8>(OutputName.length());
	if (!OutputName.empty()) stream.Write<char>((void*)OutputName.c_str(), OutputName.length());

	stream.Write<uint8>(Stages.size());
	for (int i = 0; i < Stages.size(); i++)
	{
		stream.Write<uint8>(Stages[i].Type.length());
		stream.Write<char>((void*)Stages[i].Type.c_str(), Stages[i].Type.length());

		stream.Write<uint8>(Stages[i].Name.length());
		stream.Write<char>((void*)Stages[i].Name.c_str(), Stages[i].Name.length());
	}

	ByteStream response = SendPacket(stream.Data(), stream.Size());

	if (response.Size() >= sizeof(PacketHeader) + 4)
	{
		response.Skip<PacketHeader>(1);
		uint8 command_type, result_type;
		uint16 log_length;

		response.Read<uint8>(&command_type, 1);
		response.Read<uint8>(&result_type, 1);
		response.Read<uint16>(&log_length, 1);

		if(response.Size() == sizeof(PacketHeader) + 4 + log_length)
		{
			for (int i = 0; i < log_length; i++)
			{
				char c;
				response.Read<uint8>(&c, 1);
				compiler_output += c;
			}
		}

		return (ResultTypes)result_type;
	}

	return REQUEST_ERROR;
}

bool DevTool::Server::Stop()
{
	ByteStream stream;

	uint8 request_type = DevCommandTypes::Shutdown;
	stream.Write<uint8>(&request_type, 1);

	ByteStream response = SendPacket(stream.Data(), stream.Size());

	if (response.Size() == sizeof(PacketHeader) + 2)
	{
		response.Skip<PacketHeader>(1);
		uint8 command_type, result_type;
		response.Read<uint8>(&command_type, 1);
		response.Read<uint8>(&result_type, 1);

		return command_type == DevCommandTypes::Shutdown && result_type == ResultTypes::REQUEST_ACCEPTED;
	}

	return false;
}

ByteStream DevTool::Server::SendPacket(unsigned char* data, unsigned int size)
{
	ByteStream stream;

	PacketHeader header = CreatePacketHeader(DevCommand, size);
	stream.Write<PacketHeader>(&header, 1);
	stream.Write<char>(data, size);

	int sent = socket_.Send(stream.Data(), stream.Size());
	if (sent != stream.Size()) return ByteStream();

	int data_size = socket_.Receive(4096);
	if (data_size == 0) return ByteStream();

	ByteStream result;
	result.Write<char>(socket_.GetData(), socket_.GetBytesReceived());

	return result;
}
