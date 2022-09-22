/**
 * @file CapturePusher.cpp
 * @author holylong (mrhlingchen@163.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <Windows.h>

#include <MultiMux.h>
#include <filenameio.h>
#include <opencv2/opencv.hpp>
#include <DXGIManager.hpp>

#ifndef USE_CMAKE_BUILD
#ifndef _DEBUG
#pragma comment(lib, "opencv_world400.lib")
#else
#pragma comment(lib, "opencv_world3416d.lib")
#pragma comment(lib, "StreamPusher.lib")
#endif

#pragma comment(lib, "DXGICapture.lib")
#endif //USE_CMAKE_BUILD

typedef void*(*lpFun)(void);

using namespace std;
using namespace cv;
//test
//server use srs
//test 1
//streampusher rtmp://172.19.0.228/livevideo/ddd
//ffplay rtmp://172.19.0.228/livevideo/ddd
// Stream #0:0 : Data : none
// Stream #0:1 : Video : flv1, yuv420p, 1920x1080, 4000 kb / s, 25 fps, 25 tbr, 1k tbn
//test 2
// obs push rtmp://172.19.0.228/livevideo/ddd/123456
//ffplay rtmp://172.19.0.228/livevideo/ddd/123456
// Stream #0:0: Data: none
// Stream #0:1: Audio: aac (LC), 48000 Hz, stereo, fltp, 163 kb/s
// Stream #0:2: Video: h264 (High), yuv420p(tv, bt709, progressive), 1920x1080 [SAR 1:1 DAR 16:9], 2560 kb/s, 30 fps, 30 tbr, 1k tbn, 60 tbc
int main() 
{
	HINSTANCE hDll; //DLL句柄	
	//hDll = LoadLibrary("E:\\AIHtml\\RemoteNet\\DXGI\\DXGCap\\x64\\Debug\\iconv.dll");
	//hDll = LoadLibrary("E:\\AIHtml\\RemoteNet\\DXGI\\DXGCap\\x64\\Debug\\RtmpPusher.dll");
	hDll = LoadLibraryEx("..\\x64\\Debug\\StreamPusher.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (NULL == hDll)
	{
		hDll = LoadLibraryEx("StreamPusher.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (NULL == hDll) std::cout << "Load Library Failed" << std::endl;
	}

	//lpFun addFun; //函数指针
	lpFun CreateMuxHandle = (lpFun)GetProcAddress(hDll, "CreateMuxHandle");//指的是ShowDlg的返回为void 参数为void
	if (NULL == CreateMuxHandle)
	{
		std::cout << "LoadFunction Failed" << std::endl;
	}
	CreateMuxHandle();

	//不加extern "C" C++格式导出的函数名称加了前后缀 可用这个程序查看dumpbin -exports StreamPusher.dll

	//@@YAPEAXXZ
	lpFun hw = (lpFun)GetProcAddress(hDll, "?HelloWorld@@YAPEAXXZ");//!!!!获取函数地址
	if (NULL == hw)
	{
		std::cout << "LoadFunction Failed" << std::endl;
	}
	hw();
	
	FreeLibrary(hDll);//卸载.dll文件；

	CMultiMux* pMux = createMultiMux();
	pMux->LoadConfig("decodeconfig.ini");
	auto pManager = std::make_shared<DXGIManager>();
	pManager->setup();
	RECT rect = pManager->get_output_rect();
	BYTE * pRgb = (BYTE*)malloc((rect.right - rect.left) * 4 * (rect.bottom - rect.top));
	size_t pSize = 0;

	Mat mat;
	Mat matRGB;
	mat = cv::Mat::zeros(rect.bottom - rect.top, rect.right - rect.left, CV_8UC4);
	int inx = 300;
	//namedWindow("img", 0);
	while (1)
	{
				//string strTime = getCurrentSystemTime();
		//cout << strTime << endl;
		double time = 0;
		double counts = 0;
		LARGE_INTEGER nFreq;
		LARGE_INTEGER nBeginTime;
		LARGE_INTEGER nEndTime;
		QueryPerformanceFrequency(&nFreq);
		QueryPerformanceCounter(&nBeginTime);//开始计时
		pManager->get_output_data(&pRgb, &pSize);
		mat.data = pRgb;
		//cvtColor(mat, matRGB, COLOR_RGBA2BGR);
		cvtColor(mat, matRGB, COLOR_RGBA2RGB);

		//FILE *fp = fopen("rgb24.jpg", "wb");
		//fwrite(matRGB.data, 1, matRGB.cols*matRGB.rows * 3, fp);
		//fflush(fp);
		//fclose(fp);

		//imshow("img", mat);
		//waitKey(1);
		pMux->PushStreamData(0, matRGB);
		inx--;

		QueryPerformanceCounter(&nEndTime);//停止计时

		time = (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;//计算程序执行时间单位为s
		cout << "程序执行时间：" << time * 1000 << "ms" << endl;

		if (time*1000 < 40)
		{
			std::chrono::milliseconds dura((int)(40 - (time * 1000)));
			std::this_thread::sleep_for(dura);
		}
	}
	delete pMux;
	pMux = NULL;
	//std::cin.get();
	return 0;
}