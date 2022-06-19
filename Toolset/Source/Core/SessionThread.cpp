#include "SessionThread.h"
#include "Log.h"
#include "RecordableSession.h"
#include "../Shared/Dependencies/clsocket/ActiveSocket.h"
#include "../Shared/IO/TCPPacket.h"
#include <sstream>
#include "../Shared/ByteStream.h"

struct SocketInfo
{
	CActiveSocket socket;
};

void SessionThread::Start()
{
	socket_info = new SocketInfo();
	socket_info->socket.Initialize();
	if(!socket_info->socket.Open(parent_session->address_.c_str(), parent_session->port_))
	{
		DWLog("Could not connect to: " + parent_session->address_ + ":" + std::to_string(parent_session->port_), Error);
		RequestEnd();

		return;
	}

	ByteStream stream;
	stream.Write<uint8>(EventDataType::StorageTarget);
	stream.Write<std::string>(parent_session->storage_target_);

	Send(SessionEventData, stream);

	DWLog("Started session thread");
}

void SessionThread::Loop()
{
	bool is_recording, has_data, has_meta_data;

	is_recording = parent_session->IsRecording();
	Lock();
	has_data = parent_session->data_.size() > 0;
	has_meta_data = parent_session->queued_meta_data_.size() > 0;
	Unlock();

	if (!is_recording && !has_data && !has_meta_data)
	{
		RequestEnd(); 
		return;
	}

	if (has_data) 
	{
		Lock();
		std::string entry = parent_session->data_[0];
		parent_session->data_.erase(parent_session->data_.begin());
		Unlock();

		ByteStream stream;
		stream.Write<uint8>(EventDataType::PacketData);
		stream.Write<std::string>(entry);

		if (Send(SessionEventData, stream)) DWLog("Uploaded packet:\n\t" + entry);
	}

	if (has_meta_data)
	{
		Lock();
		auto it = parent_session->queued_meta_data_.begin();
		std::string name = it->first;
		std::string data = it->second;
		parent_session->queued_meta_data_.erase(it);
		Unlock();

		ByteStream stream;
		stream.Write<uint8>(EventDataType::MetaTag);
		stream.Write<std::string>(name);
		stream.Write<std::string>(data);

		if (Send(SessionEventData, stream)) DWLog("Uploaded meta tag:\n\t" + name + ": " + data);
	}
}

void SessionThread::Cleanup()
{
	if (socket_info != nullptr) 
	{
		socket_info->socket.Close();
		delete socket_info;
		socket_info = nullptr;
	}

	Lock();
	parent_session->data_ = {};
	Unlock();

	DWLog("Stopped session thread");
}

bool SessionThread::Send(uint8_t type, ByteStream& in_stream)
{
	ByteStream out_stream;
	out_stream.Write<PacketHeader>(CreatePacketHeader((PacketType)type, in_stream.Size()));
	while (in_stream.Available()) 
	{
		char c;
		in_stream.Read<char>(&c, 1);
		out_stream.Write<char>(c);
	}

	int sent = socket_info->socket.Send((unsigned char*)out_stream.Data(), out_stream.Size());
	if (sent != out_stream.Size())
	{
		DWLog("Could not send data to: " + parent_session->address_ + ":" + std::to_string(parent_session->port_), Error);
		RequestEnd();
		return false;
	}

	return true;
}
