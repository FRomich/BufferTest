#pragma once

#include "FrameBuffer.h"

#include "MvCameraControl.h"

#include <cstdint>
#include <memory>
#include <string>

class Camera
{
public:
	explicit Camera(
        const MV_CC_DEVICE_INFO& deviceInfo,
		size_t bufferCapacity = 100);

	~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    bool open();
    bool close();

    bool start();
    bool stop();

    bool isOpen() const;
    bool isGrabbing() const;

    FrameBuffer& buffer();

    std::string serialNumber() const;
    std::string modelName() const;

private:
    static void __stdcall imageCallback(
        MV_FRAME_OUT* frame,
        void* user,
        bool autoFree);

    void onFrame(
        MV_FRAME_OUT* frame,
        bool autoFree);

private:
    void* m_handle = nullptr;

    MV_CC_DEVICE_INFO m_deviceInfo{};;

    FrameBuffer m_buffer;

    bool m_isOpen = false;
    bool m_isGrabbing = false;

};
