#include "Camera.h"
#include <cassert>

Camera::Camera(
    const MV_CC_DEVICE_INFO& deviceInfo,
    size_t bufferCapacity)
    : m_deviceInfo(deviceInfo)
    , m_buffer(bufferCapacity)
{
}

Camera::~Camera()
{
    stop();
    close();
}

bool Camera::open()
{
    if (m_isOpen)
        return true;

    int ret = MV_CC_CreateHandle(
        &m_handle,
        &m_deviceInfo);

    if (ret != MV_OK)
        return false;

    ret = MV_CC_OpenDevice(
        m_handle,
        MV_ACCESS_Exclusive,
        0);

    if (ret != MV_OK)
    {
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;

        return false;
    }

//   ret = MV_CC_SetBoolValue(
//       m_handle,
//       "GevIEEE1588",
//       true);
//
//
//   if (ret != MV_OK)
//   {
//       MV_CC_CloseDevice(m_handle);
//       MV_CC_DestroyHandle(m_handle);
//       m_handle = nullptr;
//       assert(true);
//       return false;
//   }

    m_isOpen = true;

    return true;
}

bool Camera::close()
{
    if (!m_handle)
        return true;

    if (m_isGrabbing)
        stop();

    int ret = MV_CC_CloseDevice(m_handle);

    if (ret != MV_OK)
        return false;

    MV_CC_DestroyHandle(m_handle);

    m_handle = nullptr;
    m_isOpen = false;

    return true;
}

bool Camera::start()
{
    if (!m_isOpen || m_isGrabbing)
        return false;

    int ret = MV_CC_RegisterImageCallBackEx2(
        m_handle,
        &Camera::imageCallback,
        this,
        true);

    if (ret != MV_OK)
        return false;

    ret = MV_CC_StartGrabbing(m_handle);

    if (ret != MV_OK)
        return false;

    m_isGrabbing = true;

    return true;
}

bool Camera::stop()
{
    if (!m_handle || !m_isGrabbing)
        return true;

    int ret = MV_CC_StopGrabbing(m_handle);

    if (ret != MV_OK)
        return false;

    m_isGrabbing = false;

    return true;
}

void __stdcall Camera::imageCallback(
    MV_FRAME_OUT* frame,
    void* user,
    bool autoFree)
{
    if (!frame || !user)
        return;

    auto* camera = static_cast<Camera*>(user);

    camera->onFrame(frame, autoFree);
}

void Camera::onFrame(
    MV_FRAME_OUT* frame,
    bool autoFree)
{
    const auto& info = frame->stFrameInfo;

    Frame output;

//    output.frameNumber = info.nTriggerIndex
//        ? info.nTriggerIndex
//        : info.nFrameNum;

    output.frameNumber = info.nFrameNum;

    output.trigNumber = info.nTriggerIndex;

    output.height =
        info.nExtendHeight;

    output.hostTimeStamp =
        static_cast<uint64_t>(
            info.nHostTimeStamp);

    output.deviceTimeStamp =
        (static_cast<uint64_t>(
            info.nDevTimeStampHigh) << 32) |
        static_cast<uint64_t>(
            info.nDevTimeStampLow);

    output.exposureTime =
        static_cast<double>(
            info.fExposureTime);

    output.dataSize =
        info.nFrameLenEx;

    output.data.resize(output.dataSize);

    std::memcpy(
        output.data.data(),
        frame->pBufAddr,
        output.dataSize);

    m_buffer.push(std::move(output));

    if (!autoFree)
    {
        MV_CC_FreeImageBuffer(
            m_handle,
            frame);
    }
}

FrameBuffer& Camera::buffer()
{
    return m_buffer;
}

bool Camera::isOpen() const
{
    return m_isOpen;
}

bool Camera::isGrabbing() const
{
    return m_isGrabbing;
}

std::string Camera::modelName() const
{
    return reinterpret_cast<const char*>(
        m_deviceInfo.SpecialInfo.stGigEInfo.chModelName);
}

std::string Camera::serialNumber() const
{
    return reinterpret_cast<const char*>(
        m_deviceInfo.SpecialInfo.stGigEInfo.chSerialNumber);
}
