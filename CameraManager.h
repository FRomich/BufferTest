#pragma once

#include "Camera.h"

class CameraManager
{
public:
    CameraManager() = default;
    ~CameraManager();

    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    bool initialize();
    void shutdown();

    size_t count() const;

    Camera* camera(size_t index);
    const Camera* camera(size_t index) const;

    bool openAll();
    void closeAll();

    bool startAll();
    void stopAll();

private:
    std::vector<std::unique_ptr<Camera>> m_cameras;
};
