#pragma once
#include "StorageConnectionBase.h"
#include "../Dependencies/ftplibpp/ftplib.h"

class StorageConnectionFTP : public IStorageConnection
{
public:
	void Open() override;
	void Close() override;
	void LoadSettings() override;
	void SaveSettings() override;
	SessionInfo StoreSession(Session&) override;
	Session RetrieveSession(SessionInfo&) override;
	std::vector<SessionInfo> GetSessionList() override;

private:
	ftplib* connection_;

	std::string address_;
	unsigned int port_;
	std::string username_;
	std::string password_;
	std::string target_dir_;

	bool FtpFileExists(std::string);

	std::map<std::string, std::string> ParseMetaData(picojson::array&);
	std::string ReadFtpFile(std::string);
};

REGISTER_STORAGECONNECTION_CLASS(StorageConnectionFTP, "FTP")