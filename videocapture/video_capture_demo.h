// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef _VIDEO_CAPTURE_H_
#define _VIDEO_CAPTURE_H_


#include <api/videosinkinterface.h>
#include <api/video/video_frame.h>

#include <media/base/videocapturer.h>

#include <vector>
#include <memory>

class VideoCaptureDemo : public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:

	struct VideoDeviceInfo
	{
		std::string deviceName;
		std::string deviceId;
		std::string productId;
	};

public:
	VideoCaptureDemo();
	~VideoCaptureDemo();

	void GetVideoDeviceInfo();

	void StartCapture();
	// VideoSinkInterface implementation
	void OnFrame(const webrtc::VideoFrame& frame) override;

private:
	std::vector<VideoDeviceInfo> videos_;

	std::unique_ptr<cricket::VideoCapturer> capturer_;
};


#endif //_VIDEO_CAPTURE_H_
