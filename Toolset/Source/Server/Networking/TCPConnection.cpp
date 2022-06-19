#include "TCPConnection.h"
#include "TCPHost.h"
#include "../Application/Log.h"
#include "../../Shared/IO/TCPPacket.h"
#include "../Application/Application.h"
#include "../Application/ConfigurationManager.h"
#include "../Networking/StorageManager.h"
#include "../CompilationManager.h"
#include "../../Shared/Platform/Platform.h"

#define buffer_size	 2048

void SessionImporter::ProcessPacket(TCPConnection* connection, void* packet, unsigned int data_size)
{
	ByteStream stream;
	stream.Write<char>(packet, data_size);

	uint8 command;
	stream.Read<uint8>(&command, 1);

	switch ((EventDataType)command) 
	{ 
	case PacketData:
		{
			unsigned int packet_length;
			stream.Read<unsigned int>(&packet_length, 1);
			std::string packet_data;
			packet_data.resize(packet_length);
			stream.Read<char>((void*)packet_data.data(), packet_length);

			session_.Packets.push_back(packet_data);
			connection->TCPLog(">> " + packet_data);

			break;
		}
	case StorageTarget:
		{
			unsigned int name_length;
			stream.Read<unsigned int>(&name_length, 1);
			std::string target_name;
			target_name.resize(name_length);
			stream.Read<char>((void*)target_name.data(), name_length);

			storage_target_ = target_name;

			connection->TCPLog("@ " + storage_target_);

			break;
		}

	case MetaTag:
		{
			unsigned int length;
			stream.Read<unsigned int>(&length, 1);
			std::string name;
			name.resize(length);
			stream.Read<char>((void*)name.data(), length);

			stream.Read<unsigned int>(&length, 1);
			std::string data;
			data.resize(length);
			stream.Read<char>((void*)data.data(), length);

			session_.Meta.insert(std::pair<std::string, std::string>(name, data));
			connection->TCPLog("# " + name + ": " + data);
		break;
		}
	}	
}

void SessionImporter::FlushData(TCPConnection* connection)
{
	ValidateSession(connection);

	if(!session_.Packets.empty())
	{
		IStorageConnection* storage_connection = StorageManager::Get().OpenConnection(storage_target_);
		if(storage_connection != nullptr)
		{
			SessionInfo info = storage_connection->StoreSession(session_);
			storage_connection->Close();

			if (!info.Name.empty()) { connection->TCPLog("Uploaded session '" + info.Name + "' to storage: " + storage_target_); } else{ connection->TCPLog("Failed to upload session to storage: " + storage_target_); }
		} else
		{
			connection->TCPLog("Could not find connection for storage: " + storage_target_);
		}
	}
	else
	{
		connection->TCPLog("No data available to upload");
	}
}

void SessionImporter::ValidateSession(TCPConnection* connection)
{
	if (session_.Packets.empty()) return;

	auto connection_it = ConfigurationManager::Get().Connections.find(storage_target_);
	if (connection_it == ConfigurationManager::Get().Connections.end())
	{
		connection->TCPLog("Could not find connection for storage: " + storage_target_);
		return;
	}

	ConnectionInfo& connection_info = connection_it->second;

	for (int i = 0; i < session_.Packets.size(); i++)
	{
		picojson::value packet_value;
		picojson::parse(packet_value, session_.Packets[i]);
		if(packet_value.is<picojson::value::array>())
		{
			picojson::array packet = packet_value.get<picojson::array>();
			if (packet.size() >= 1)
			{
				if(packet[0].is<std::string>())
				{
					std::string packet_name = packet[0].get<std::string>();
					auto packet_it = connection_info.PacketDefinitions.find(packet_name);
					if (packet_it != connection_info.PacketDefinitions.end())
					{
						PacketDefinition& packet_def = packet_it->second;

						bool valid = true;
						for (int member_id = 1; member_id < packet.size(); member_id++)
						{
							if (!packet[member_id].is<picojson::array>())
							{
								valid = false;
								Log("Invalid packet member");
							}

							picojson::array& packet_member = packet[member_id].get<picojson::array>();

							if (packet_member.size() != 2 && packet_member.size() != 3)
							{
								valid = false;
								Log("Invalid packet member");
							}

							if(packet_member.size() == 2)
							{
								if(packet_def.Members[member_id - 1].Type == packet_member[0].get<std::string>())
								{
									packet_member.insert(packet_member.begin(), picojson::value(packet_def.Members[member_id - 1].Name));
								}
								else
								{
									connection->TCPLog("Mismatched packet definition");
									valid = false;
								}								
							}
						}

						if (valid)
						{
							session_.Packets[i] = picojson::value(packet).serialize();
							continue;
						}
					}
					else
					{
						connection->TCPLog("Unknown packet type: " + packet_name);
					}
				} else
				{
					connection->TCPLog("No Packet name type found");
				}
			}
			else
			{
				connection->TCPLog("No Packet data found");
			}
		} else
		{
			connection->TCPLog("Invalid packet structure");
		}

		connection->TCPLog("Ignoring packet: " + session_.Packets[i]);
		session_.Packets.erase(session_.Packets.begin() + i);
		i--;
	}
}

ByteStream AuthorizedUserInterface::ProcessCommand(TCPConnection* connection, void* cmd, unsigned int data_size)
{
	ByteStream stream;
	stream.Write<char>(cmd, data_size);

	uint8 command;
	stream.Read<uint8>(&command, 1);

	switch ((DevCommandTypes)command) {
		case Login:
			{
				uint16 string_length;
				stream.Read<uint16>(&string_length, 1);

				char* string = (char*)malloc(string_length);
				stream.Read<char>(string, string_length);
				std::string username(string, string_length);
				free(string);

				stream.Read<uint16>(&string_length, 1);

				string = (char*)malloc(string_length);
				stream.Read<char>(string, string_length);
				std::string password(string, string_length);
				free(string);

				authorized_ = ConfigurationManager::Get().IsValidUser(username, password);

				PacketHeader header = CreatePacketHeader(DevCommand, 2);

				ByteStream result;

				result.Write<PacketHeader>(&header, 1);
				uint8 command_type = Login;
				result.Write<uint8>(&command_type, 1);

				uint8 result_type = authorized_ ? AUTHORIZED_YES : AUTHORIZED_NO;
				result.Write<uint8>(&result_type, 1);

				if (authorized_) connection->TCPLog("User '" + username + "' logged in"); else connection->TCPLog("Authorization failed for user '" + username + "'");

				return result;
			}
		case Shutdown:
			{

				if (authorized_)
				{
					connection->TCPLog("Request to shutdown accepted");
					Application::Get().RequestShutdown();
				} else
				{
					connection->TCPLog("Request to shutdown denied. User is not authorized.");
				}

				PacketHeader header = CreatePacketHeader(DevCommand, 2);

				ByteStream result;

				result.Write<PacketHeader>(&header, 1);
				uint8 command_type = Shutdown;
				result.Write<uint8>(&command_type, 1);

				uint8 result_type = authorized_ ? REQUEST_ACCEPTED : REQUEST_DENIED;
				result.Write<uint8>(&result_type, 1);


				return result;
			}

		case RequestCompile:
			{
				std::string log_response;
				ResultTypes response_type = REQUEST_DENIED;

				std::string database_blob;
				std::string filter_blob;
				std::string output_name;
				std::vector<CompilationStageInfo> stages;


				if (authorized_)
				{
					connection->TCPLog("Request to compile received");
					response_type = REQUEST_ACCEPTED;

					if (stream.Available() >= 1) 
					{
						uint8 string_length;
						stream.Read<uint8>(&string_length, 1);
						if (stream.Available() >= string_length)
						{
							char* string_data = (char*)malloc(string_length);
							stream.Read<char>(string_data, string_length);
							database_blob = std::string(string_data, string_length);
							free(string_data);
						}
					}

					if (stream.Available() >= 1)
					{
						uint8 string_length;
						stream.Read<uint8>(&string_length, 1);
						if (stream.Available() >= string_length && string_length > 0)
						{
							char* string_data = (char*)malloc(string_length);
							stream.Read<char>(string_data, string_length);
							filter_blob = std::string(string_data, string_length);
							free(string_data);
						}
					}

					if (stream.Available() >= 1)
					{
						uint8 string_length;
						stream.Read<uint8>(&string_length, 1);
						if (stream.Available() >= string_length && string_length > 0)
						{
							char* string_data = (char*)malloc(string_length);
							stream.Read<char>(string_data, string_length);
							output_name = std::string(string_data, string_length);
							free(string_data);
						}
					}

					if (stream.Available() >= 1)
					{
						uint8 stage_count;
						stream.Read<uint8>(&stage_count, 1);

						for (int i = 0; i < stage_count; i++) 
						{
							std::string stage_type, stage_name;
							if (stream.Available() >= 1)
							{
								uint8 string_length;
								stream.Read<uint8>(&string_length, 1);
								if (stream.Available() >= string_length)
								{
									char* string_data = (char*)malloc(string_length);
									stream.Read<char>(string_data, string_length);
									stage_type = std::string(string_data, string_length);
									free(string_data);

									stream.Read<uint8>(&string_length, 1);
									if (stream.Available() >= string_length)
									{
										char* string_data = (char*)malloc(string_length);
										stream.Read<char>(string_data, string_length);
										stage_name = std::string(string_data, string_length);
										free(string_data);
									}
								}

								
							}

							if(!stage_type.empty() && !stage_name.empty()) stages.push_back({ stage_type, stage_name });
						}
					}

					if(database_blob.empty())
					{
						response_type = REQUEST_ERROR;
						log_response += "No databases specified\n";
					}

					if (stages.empty())
					{
						response_type = REQUEST_ERROR;
						log_response += "No compilation stages specified\n";
					}

					if (!database_blob.empty() && !stages.empty())
					{
						CompilationTask task;
						task.ExportName = output_name.empty() ? "DWC_Request" : output_name;
						task.Databases = SplitString(database_blob, ';');
						task.Filters = MetaFilter::Parse(filter_blob);

						for (CompilationStageInfo& stage : stages)
						{
							ICompilationStage* new_stage = ICompilationStageRegistry::CreateStage(stage);
							if (new_stage == nullptr)
							{
								log_response += "Invalid compilation stage: " + stage.Name + " (" + stage.Type + ")";
								response_type = REQUEST_ERROR;
							}
							else
							{
								task.Stages.push_back(new_stage);
							}
						}

						CompilerLogReceiver receiver;
						task.Receivers.push_back(&receiver);

						if (response_type == REQUEST_ACCEPTED) 
						{
							CompilationManager::Get().AddTask(task);

							CompilerLogReceiver::Status status = CompilerLogReceiver::Pending;
							while (status == CompilerLogReceiver::Pending)
							{
								std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(200));
								status = receiver.Receive(log_response);
							}
						}
					}
				}
				else
				{
					connection->TCPLog("Request to compile denied. User is not authorized.");
					response_type = REQUEST_DENIED;
				}

				PacketHeader header = CreatePacketHeader(DevCommand, 4 + log_response.length());

				ByteStream result;

				result.Write<PacketHeader>(&header, 1);
				result.Write<uint8>(RequestCompile);
				result.Write<uint8>(response_type);
				result.Write<uint16>(log_response.length());

				for (char c : log_response)
				{
					result.Write<uint8>(c);
				}

				return result;
			}
	}

	return ByteStream();
}

void TCPConnection::Start()
{
	TCPLog("Client connected");
}

void TCPConnection::Loop()
{
	int received = socket->Receive(buffer_size);
	if (received == 0) { RequestEnd(); return; } // Client has disconnected
	if (received == -1) return; // No data
	if (received < 0) { RequestEnd(); return; } // Error occured

	// Copy data into buffer
	data_.Write<uint8>(socket->GetData(), socket->GetBytesReceived());

	while(data_.Size() > sizeof(PacketHeader))
	{
		PacketHeader header;
		data_.Read<PacketHeader>(&header, 1);

		if(header.id[0]=='D' && header.id[1] == 'W')	// Verify packet header
		{
			if (data_.Size() < header.packet_size) break;	// Not all data has been received yet

			uint16 data_size = header.packet_size - sizeof(PacketHeader);
			void* packet_data = malloc(data_size);
			data_.Read<unsigned char>(packet_data, data_size);

			ByteStream result;

			switch (static_cast<PacketType>(header.packet_type))
			{
			case DevCommand: { result = dev_command_.ProcessCommand(this, packet_data, data_size); break; }
			case SessionEventData: { session_importer_.ProcessPacket(this, packet_data, data_size); break; }
			default: {}
			}

			free(packet_data);

			if (result.Size() != 0)
			{
				socket->Send(result.Data(), result.Size());
			}

			data_.Erase(0, header.packet_size);	// Erase packet from buffer
		} 
		else
		{
			data_.Erase(0, 1); // Skip byte that is not a packet header
		}
	}
}

void TCPConnection::Cleanup()
{
	TCPLog("Client disconnected");

	session_importer_.FlushData(this);

	TCPLog("Closing connection");

	socket->Close();
	delete socket;
	socket = nullptr;

	parent_thread->Lock();
	const std::vector<TCPConnection*>::iterator pos = std::find(parent_thread->connections.begin(), parent_thread->connections.end(), this);
	if (pos != parent_thread->connections.end()) parent_thread->connections.erase(pos);
	parent_thread->Unlock();
}

void TCPConnection::TCPLog(std::string text)
{
	Log("\t" + (socket == nullptr ? "" : ("(" + std::string(socket->GetClientAddr()) + ":" + std::to_string(socket->GetClientPort()) + ") ")) + text);
}
