#include "CameraManager.h"

CameraManager::~CameraManager()
{
    shutdown();
}

bool CameraManager::initialize()
{
    m_cameras.clear();

    MV_CC_DEVICE_INFO_LIST deviceList{};

    int ret = MV_CC_EnumDevices(
        MV_GIGE_DEVICE,
        &deviceList);

    if (ret != MV_OK)
        return false;

    for (unsigned int i = 0; i < deviceList.nDeviceNum; ++i)
    {
        if (deviceList.pDeviceInfo[i] == nullptr)
            continue;

        m_cameras.push_back(
            std::make_unique<Camera>(
                *deviceList.pDeviceInfo[i]));
    }

    return true;
}

size_t CameraManager::count() const
{
    return m_cameras.size();
}

Camera* CameraManager::camera(size_t index)
{
    if (index >= m_cameras.size())
        return nullptr;

    return m_cameras[index].get();
}

const Camera* CameraManager::camera(size_t index) const
{
    if (index >= m_cameras.size())
        return nullptr;

    return m_cameras[index].get();
}

bool CameraManager::openAll()
{
    for (auto& camera : m_cameras)
    {
        if (!camera->open())
            return false;
    }

    return true;
}

void CameraManager::closeAll()
{
    for (auto& camera : m_cameras)
    {
        camera->close();
    }
}

bool CameraManager::startAll()
{
    for (auto& camera : m_cameras)
    {
        if (!camera->start())
            return false;
    }

    return true;
}

void CameraManager::stopAll()
{
    for (auto& camera : m_cameras)
    {
        camera->stop();
    }
}

void CameraManager::shutdown()
{
    stopAll();
    closeAll();

    m_cameras.clear();
}