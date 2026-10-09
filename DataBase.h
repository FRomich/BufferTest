#pragma once

#include <libpq-fe.h>
#include <string>
#include "Frame.h"

class DataBase
{
public:
    DataBase() = default;
    ~DataBase();

    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    bool connect(const char* connInfo);
    void disconnect();

    bool isConnected() const;

    bool execute(const std::string& sql);

    std::string lastError() const;

    bool insertFrame(
        int sessionId,
        int cameraId,
        const Frame& frame,
        const std::string& filePath);

    std::int64_t createSession(
        const std::string& sessionName,
        const std::string& directoryPath);

private:
    PGconn* m_connection = nullptr;
    std::string m_lastError;
};
