#include <string>
#include <array>

#include "DataBase.h"
#include "Frame.h"


DataBase::~DataBase()
{
    disconnect();
}

bool DataBase::connect(const char* connInfo)
{
    disconnect();
    m_lastError.clear();

    m_connection = PQconnectdb(connInfo);

    if (!m_connection)
    {
        m_lastError = "Failed to allocate connection";
        return false;
    }

    if (PQstatus(m_connection) != CONNECTION_OK)
    {
        m_lastError = PQerrorMessage(m_connection);
        disconnect();
        return false;
    }

    return true;
}


void DataBase::disconnect()
{
    if (m_connection)
    {
        PQfinish(m_connection);
        m_connection = nullptr;
    }
}

bool DataBase::isConnected() const
{
    return m_connection &&
        PQstatus(m_connection) == CONNECTION_OK;
}

bool DataBase::execute(const std::string& sql)
{
    if (!isConnected())
    {
        m_lastError = "Database is not connected";
        return false;
    }

    PGresult* result = PQexec(
        m_connection,
        sql.c_str()
    );

    if (!result)
    {
        m_lastError = PQerrorMessage(m_connection);
        return false;
    }

    const auto status = PQresultStatus(result);

    const bool success =
        status == PGRES_COMMAND_OK ||
        status == PGRES_TUPLES_OK;

    if (!success)
        m_lastError = PQresultErrorMessage(result);
    else
        m_lastError.clear();

    PQclear(result);

    return success;
}

std::string DataBase::lastError() const
{
    return m_lastError;
}

bool DataBase::insertFrame(
    int sessionId,
    int cameraId,
    const Frame& frame,
    const std::string& filePath)
{
    if (!isConnected())
    {
        m_lastError = "Database is not connected";
        return false;
    }

    const char* sql = R"(
        INSERT INTO public.frames
        (
            session_id,
            camera_id,
            frame_number,
            trigger_number,
            width,
            height,
            device_timestamp,
            host_timestamp,
            exposure_time,
            file_path
        )
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
    )";

    const std::array<std::string, 10> values {
        std::to_string(sessionId),
        std::to_string(cameraId),
        std::to_string(frame.frameNumber),
        std::to_string(frame.trigNumber),
        std::to_string(frame.width),
        std::to_string(frame.height),
        std::to_string(frame.deviceTimeStamp),
        std::to_string(frame.hostTimeStamp),
        std::to_string(frame.exposureTime),
        filePath
    };

    std::array<const char*, 10> params{};

    for (size_t i = 0; i < values.size(); ++i)
        params[i] = values[i].c_str();

    PGresult* result = PQexecParams(
        m_connection,
        sql,
        static_cast<int>(params.size()),
        nullptr,
        params.data(),
        nullptr,
        nullptr,
        0);

    if (!result)
    {
        m_lastError = PQerrorMessage(m_connection);
        return false;
    }

    const bool success =
        PQresultStatus(result) == PGRES_COMMAND_OK;

    if (!success)
        m_lastError = PQresultErrorMessage(result);
    else
        m_lastError.clear();

    PQclear(result);
    return success;
}

std::int64_t DataBase::createSession(
    const std::string& sessionName,
    const std::string& directoryPath)
{
    if (!isConnected())
    {
        m_lastError = "Database is not connected";
        return 0;
    }

    const char* sql = R"(
        INSERT INTO public.frame_sessions
            (session_name, directory_path, started_at)
        VALUES ($1, $2, now())
        RETURNING id
    )";

    const std::array<const char*, 2> params = {
        sessionName.c_str(),
        directoryPath.c_str()
    };

    PGresult* result = PQexecParams(
        m_connection,
        sql,
        2,
        nullptr,
        params.data(),
        nullptr,
        nullptr,
        0);

    if (!result)
    {
        m_lastError = PQerrorMessage(m_connection);
        return 0;
    }

    if (PQresultStatus(result) != PGRES_TUPLES_OK ||
        PQntuples(result) != 1)
    {
        m_lastError = PQresultErrorMessage(result);
        PQclear(result);
        return 0;
    }

    const auto sessionId =
        std::stoll(PQgetvalue(result, 0, 0));

    PQclear(result);
    m_lastError.clear();

    return sessionId;
}