#include "StorageConnectionFTP.h"
#include "../Application/Log.h"
#include "../../Shared/Platform/Platform.h"
#include "../../Shared/ByteStream.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

void StorageConnectionFTP::Open()
{
	connection_ = new ftplib();

	if (!connection_->Connect((address_ + ":" + std::to_string(port_)).c_str()))
	{
		Log("Could not connect to FTP server: " + address_+ ":" + std::to_string(port_));
		connection_ = nullptr;
		return;
	}

	if (!connection_->Login(username_.c_str(), password_.c_str()))
	{
		Log("Could not login to FTP server");
		connection_->Quit();
		connection_ = nullptr;
		return;
	}
}

void StorageConnectionFTP::Close()
{
	if (connection_ != nullptr)
	{
		connection_->Quit();
		delete connection_;
		connection_ = nullptr;
	}

	IStorageConnection::Close();
}

void StorageConnectionFTP::LoadSettings()
{
	address_ = GetSetting<std::string>("Address", "127.0.0.1");
	port_ = GetSetting<double>("Port", 21);
	target_dir_ = GetSetting<std::string>("Directory", "/");
	username_ = GetSetting<std::string>("Username", "");
	password_ = GetSetting<std::string>("Password", "");
}

void StorageConnectionFTP::SaveSettings()
{
	SetSetting<std::string>("Address", address_);
	SetSetting<double>("Port", port_);
	SetSetting<std::string>("Directory", target_dir_);
	SetSetting<std::string>("Username", username_);
	SetSetting<std::string>("Password", password_);
}

SessionInfo StorageConnectionFTP::StoreSession(Session& session)
{
	if (connection_ == nullptr)
	{
		return SessionInfo();
	}

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t time = std::chrono::system_clock::to_time_t(now);
	tm local_time = *localtime(&time);

	if (session.Name.empty()) 
	{
		std::stringstream session_name;
		session_name << std::setfill('0') << std::setw(4) << (local_time.tm_year + 1900) << "_" << std::setfill('0') << std::setw(2) << (local_time.tm_mon + 1) << "_" << std::setfill('0') << std::setw(2) << local_time.tm_mday << "-" << std::setfill('0') << std::setw(2) << local_time.tm_hour << "_" << std::setfill('0') << std::setw(2) << local_time.tm_min << "_" << std::setfill('0') << std::setw(2) << local_time.tm_sec;
		session.Name = session_name.str();
	}

	//TODO use proper filenames
	std::string filename = target_dir_ + session.Name + ".json";
	ftphandle* file = connection_->RawOpen(filename.c_str(), ftplib::filewrite, ftplib::ascii);
	if (file == nullptr)
	{
		Log("Failed to open file: " + filename);
		return SessionInfo();
	}

	picojson::array data_array;


	for (std::string& packet : session.Packets) 
	{
		picojson::value packet_data;
		std::string error = picojson::parse(packet_data, packet);

		if (!error.empty())
		{
			Log("Failed to parse packet\n" + packet); 
			break;
		}

		if(!packet_data.is<picojson::array>())
		{
			Log("Malformed packet\n" + packet);
			break;
		}

		data_array.push_back(picojson::value(packet_data.get<picojson::array>()));
	}

	std::string data = picojson::value(data_array).serialize();
	size_t cursor = 0;

	while(cursor < data.length())
	{
		size_t length = std::min((size_t)1024, data.length() - cursor);
		size_t written = connection_->RawWrite((char*)data.data() + cursor, length, file);
		cursor += written;
	}

	connection_->RawClose(file);


	// Meta data
	data_array = {};

	for (auto it = session.Meta.begin(); it != session.Meta.end(); it++)
	{
		picojson::value meta_value;
		picojson::parse(meta_value, it->second);

		if(meta_value.is<picojson::array>())
		{
			picojson::array& meta_array = meta_value.get<picojson::array>();
			if (meta_array.size() == 2)
			{
				picojson::array entry;
				entry.push_back(picojson::value(it->first));
				entry.push_back(meta_array[0]);
				entry.push_back(meta_array[1]);
				data_array.push_back(picojson::value(entry));
			}
			else
			{
				Log("Malformed meta data\n" + it->second);
			}
		} else
		{
			Log("Mistyped meta data\n" + it->second);
		}
	}

	if (!data_array.empty()) 
	{
		cursor = 0;
		filename = target_dir_ + session.Name + ".meta.json";
		file = connection_->RawOpen(filename.c_str(), ftplib::filewrite, ftplib::ascii);
		data = picojson::value(data_array).serialize();
		while (cursor < data.length())
		{
			size_t length = std::min((size_t)1024, data.length() - cursor);
			size_t written = connection_->RawWrite((char*)data.data() + cursor, length, file);
			cursor += written;
		}
		connection_->RawClose(file);
	}


	SessionInfo info;
	info.Name = session.Name;
	info.Database = GetDatabaseName();
	info.Meta = session.Meta;
	return info;
}

Session StorageConnectionFTP::RetrieveSession(SessionInfo& info)
{
	if (connection_ == nullptr)
	{
		return Session();
	}

	std::string filename = target_dir_ + info.Name + ".json";

	std::string data = ReadFtpFile(filename);
	if (!data.empty()) {

		picojson::value session_value;
		picojson::parse(session_value, data);

		if (!session_value.is<picojson::array>())
		{
			Log("Failed to parse session: " + info.Name);
			return Session();
		}

		picojson::array& session = session_value.get<picojson::array>();

		Session result_session;
		result_session.Name = info.Name;


		for (picojson::value& packet_value : session)
		{
			if (!packet_value.is<picojson::array>())
			{
				Log("Failed to parse packet: " + info.Name + " > " + packet_value.serialize());
			}

			result_session.Packets.push_back(packet_value.serialize());
		}



		// Meta data
		filename = target_dir_ + info.Name + ".meta.json";
		if (FtpFileExists(filename))
		{
			data = ReadFtpFile(filename);
			if (!data.empty())
			{
				picojson::value meta_value;
				picojson::parse(meta_value, data);
				if (meta_value.is<picojson::array>())
				{
					picojson::array& meta_array = meta_value.get<picojson::array>();
					result_session.Meta = ParseMetaData(meta_array);
				}
			}
		}

		return result_session;
	}

	return Session();
}

std::vector<SessionInfo> StorageConnectionFTP::GetSessionList()
{
	if (connection_ == nullptr)
	{
		return std::vector<SessionInfo>();
	}

	std::string file_list_cache = GetExecutableDirectory() + "\\ftp_cache.txt";

	if(connection_->Nlst(file_list_cache.c_str(), target_dir_.c_str())!=1)
	{
		Log("Could not retrieve file list");
		return std::vector<SessionInfo>();
	}

	std::vector<SessionInfo> result;

	std::ifstream cache_file(file_list_cache.c_str());
	std::string file;
	while (std::getline(cache_file, file))
	{
		std::string filename = ReplaceString(FormatPath(file), FormatPath(target_dir_), "");
		if (!MatchStringPattern(filename, "*.json")) continue;
		std::string session_name = ReplaceString(filename, ".json", "");

		if (filename != "." && filename != "..") 
		{
			SessionInfo info;
			info.Name = session_name;
			info.Database = GetDatabaseName();

			std::string meta_file = target_dir_ + info.Name + ".meta.json";
			if (FtpFileExists(meta_file))
			{
				std::string data = ReadFtpFile(meta_file);
				if (!data.empty())
				{
					picojson::value meta_value;
					picojson::parse(meta_value, data);
					if (meta_value.is<picojson::array>())
					{
						picojson::array& meta_array = meta_value.get<picojson::array>();
						info.Meta = ParseMetaData(meta_array);
					}
				}
			}

			result.push_back(info);
		}
	}
	cache_file.close();

	return result;
}

bool StorageConnectionFTP::FtpFileExists(std::string filename)
{
	int size;
	if (connection_->Size(filename.c_str(), &size, ftplib::image) != 1)
	{
		return false;
	}
	return size > 0;
}

std::map<std::string, std::string> StorageConnectionFTP::ParseMetaData(picojson::array& data)
{
	std::map<std::string, std::string> result;


	for(picojson::value& entry_value : data)
	{
		picojson::array& entry_array = entry_value.get<picojson::array>();
		picojson::array data;
		data.push_back(entry_array[1]);
		data.push_back(entry_array[2]);
		result.insert(std::pair<std::string, std::string>(entry_array[0].get <std::string>(), picojson::value(data).serialize()));
	}

	return result;
}

std::string StorageConnectionFTP::ReadFtpFile(std::string filename)
{
	int file_size;

	if (connection_->Size(filename.c_str(), &file_size, ftplib::image) != 1)
	{
		Log("Could not find file: " + filename);
		return std::string();
	}

	ftphandle* file = connection_->RawOpen(filename.c_str(), ftplib::fileread, ftplib::image);
	if (file == nullptr)
	{
		Log("Failed to open file: " + filename);
		return std::string();
	}


	std::string data;
	data.resize(file_size);

	int cursor = 0;

	while (cursor < file_size)
	{
		int length = std::min(1024, file_size - cursor);

		int read = connection_->RawRead((char*)data.data() + cursor, length, file);

		cursor += read;
	}

	connection_->RawClose(file);

	return data;
}
