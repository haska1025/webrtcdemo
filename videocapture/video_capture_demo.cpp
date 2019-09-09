// pch.cpp: 与预编译标头对应的源文件；编译成功所必需的

#include "video_capture_demo.h"


#include <modules/video_capture/video_capture.h>
#include <modules/video_capture/video_capture_factory.h>
#include <media/engine/webrtcvideocapturerfactory.h>

#include <iostream>
#include <algorithm>

// 一般情况下，忽略此文件，但如果你使用的是预编译标头，请保留它。

VideoCaptureDemo::VideoCaptureDemo()
{
}

VideoCaptureDemo::~VideoCaptureDemo()
{
}


void VideoCaptureDemo::GetVideoDeviceInfo()
{
	std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
		webrtc::VideoCaptureFactory::CreateDeviceInfo());
	if (!info) {
		return;
	}
	int num_devices = info->NumberOfDevices();
	for (int i = 0; i < num_devices; ++i) {
		const uint32_t kSize = 256;
		char name[kSize] = { 0 };
		char id[kSize] = { 0 };
		char product[kSize] = { 0 };
		if (info->GetDeviceName(i, name, kSize, id, kSize, product, kSize) != -1) {
			VideoDeviceInfo vdi;
			vdi.deviceName = name;
			vdi.deviceId = id;
			vdi.productId = product;

			std::cout << "Name:" << name << ", id:" << id << ", product:" << product << std::endl;
			videos_.push_back(vdi);
		}
	}
}

void VideoCaptureDemo::StartCapture()
{
	cricket::WebRtcVideoDeviceCapturerFactory factory;
	std::unique_ptr<cricket::VideoCapturer> capturer;
	for (const auto& vdi : videos_) {
		capturer = factory.Create(cricket::Device(vdi.deviceName, 0));
		if (capturer) {
			break;
		}
	}

	this->capturer_ = std::move(capturer);

	cricket::VideoFormat format;

	format.height = 720;
	format.width = 1280;
	format.interval = FPS_TO_INTERVAL(30);
	format.fourcc = cricket::FOURCC_ANY;

	cricket::VideoFormat best_format;

	capturer_->GetBestCaptureFormat(format, &best_format);

	std::cout << "best format height:" << best_format.height << ", width:" << best_format.width 
		<< ", interval:" << best_format.interval << ", fourcc:" << best_format.fourcc << std::endl;

	capturer_->Start(best_format);
	capturer_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}
// VideoSinkInterface implementation
void VideoCaptureDemo::OnFrame(const webrtc::VideoFrame& frame)
{
	std::cout << "OnFrame size:" << frame.size() << ", width:" << frame.width() 
		<< ", height:" << frame.height() << ", timestamp:" << frame.timestamp() << std::endl;
}
