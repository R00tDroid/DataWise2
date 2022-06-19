#include "PacketBuilder.h"

PacketBuilder::PacketBuilder(RecordableSession* session, std::string packet_id)
{
	session_ = session;
	data_ = "[\"" + packet_id + "\"";
}

void PacketBuilder::AddTimestamp()
{
	AddData("time", ToString(session_->GetTime()));
}

void PacketBuilder::Send()
{
	data_ += "]";
	session_->ReportEvent(data_);
}

void PacketBuilder::AddData(std::string type, std::string data, bool stringify)
{
	data_ += ",[\"" + type + "\"," + (stringify ? "\"" : "") + data + (stringify ? "\"" : "") + "]";
}
