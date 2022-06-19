#pragma once
#include "../Shared/ByteStream.h"
#include "../Shared/Dependencies/clsocket/ActiveSocket.h"
#include "../Shared/IO/TCPMessages.h"
#include "../Shared/Compilation/CompilationStage.h"

namespace DevTool {

	class Server
	{
	public:
		bool Connect();
		void Disconnect();
		bool Login();
		ResultTypes RemoteCompile(std::string Database, std::string Filters, std::string OutputName, std::vector<CompilationStageInfo> Stages, std::string& compiler_output);

		bool Stop();

	private:
		CActiveSocket socket_;

		ByteStream SendPacket(unsigned char* data, unsigned int size);
	};
}
