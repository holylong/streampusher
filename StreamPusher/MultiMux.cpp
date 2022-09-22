// MIT License

// Copyright (c) 2022 hooy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "MultiMux.h"
#include "ImageMuxing.h"
#include "iniparser.h"
#include "dictionary.h"


EXPORT_DLLA void* CreateMuxHandle()
{
	std::cout << "Fuck" << std::endl;
	return nullptr;
}

EXPORT_DLL void* HelloWorld()
{
	std::cout << "Hello World End" << std::endl;
	return nullptr;
}

CMultiMux* createMultiMux()
{
	return new CMultiMux;
}

CMultiMux::CMultiMux()
{
	m_pLock = new MuxLocker;
	m_mapConfig.clear();
	m_mapMux.clear();
}

CMultiMux::~CMultiMux()
{
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.begin();
	for (; iterMux != m_mapMux.end(); iterMux++)
	{
		delete iterMux->second;
		iterMux->second = NULL;
	}

	delete m_pLock;
	m_pLock = NULL;
}

//int CMultiMuxing::AddStreamMux(CStreamMuxing* m_pMux)
//{
//	if (m_pMux == NULL)
//		return -1;
//	m_arrMux.push_back(m_pMux);
//	return m_arrMux.size();
//}

int CMultiMux::PushStreamData(int nChannel, uint8_t* pRgb)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(pRgb);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		fprintf(stderr, "Find Muxer Error!\n");
	}
	m_pLock->UnLock();
	return -1;
}

int CMultiMux::LoadConfig(const char* strPath)
{
	dictionary * ini;
	//char       * ini_name;
	//ini_name = "./config/decodeconfig.ini";
	ini = iniparser_load(strPath);
	
	int nrtmpcount = iniparser_getint(ini, "rtmp:count", 0);
	for (int i = 0; i < nrtmpcount; i++)
	{
		rtmpconfig *pConfig = new rtmpconfig;
		char chRtmp[20];
		memset(chRtmp, '\0', 20);
		sprintf(chRtmp, "rtmp%d:framerate", i);
		pConfig->nFrameRate = iniparser_getint(ini, chRtmp, 20);
		memset(chRtmp, '\0', 20);
		sprintf(chRtmp, "rtmp%d:fmt", i);
		pConfig->strFmt = iniparser_getstring(ini, chRtmp, "flv");
		memset(chRtmp, '\0', 20);
		sprintf(chRtmp, "rtmp%d:url", i);
		pConfig->strUrl = iniparser_getstring(ini, chRtmp, "rtsp://192.168.1.103/live");

		std::map<int, rtmpconfig*>::iterator iterConfig = m_mapConfig.find(i);
		if (iterConfig != m_mapConfig.end())
		{
			delete iterConfig->second;
			iterConfig->second = NULL;
			m_mapConfig[i] = pConfig;
		}
		else
			m_mapConfig[i] = pConfig;
	}

	iniparser_freedict(ini);
	return m_mapConfig.size();
}

int CMultiMux::PushStreamData(int nChannel, int nWidth, int nHeight, uint8_t* pRgb)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(pRgb);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		std::map<int, rtmpconfig*>::iterator iterConfig = m_mapConfig.find(nChannel);
		if (m_mapConfig.end() != iterConfig)
		{
			CImageMuxing *pMux = createMuxer(nWidth, nHeight, iterConfig->second->strFmt.c_str(), iterConfig->second->strUrl.c_str(), iterConfig->second->nFrameRate);
			printf("Channel:%d\n", nChannel);
			m_mapMux[nChannel] = pMux;
			ret = pMux->PushStream(pRgb);
			m_pLock->UnLock();
			return ret;
		}
		else
		{
			//fprintf(stderr, "Cannot Init RtmpStream, can't find config by this channel, please use PushStreamData(int, const char*, const char*, IplImage*, int) function\n");
		}
	}
	m_pLock->UnLock();
	return -1;
}

int CMultiMux::PushStreamData(int nChannel, int nWidth, int nHeight, const char* strFmt, const char* strUtl, uint8_t* pRgb, int nFrameRate)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(pRgb);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		CImageMuxing *pMux = createMuxer(nWidth, nHeight, strFmt, strUtl, nFrameRate);
		printf("Channel:%d\n", nChannel);
		m_mapMux[nChannel] = pMux;
		ret = pMux->PushStream(pRgb);
		m_pLock->UnLock();
		return ret;
	}
	m_pLock->UnLock();
	return -1;
}

int CMultiMux::RemoveChannel(int nChannel)
{
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		delete iterMux->second;
		iterMux->second = NULL;
		m_mapMux.erase(iterMux);
	}
	else
	{
		fprintf(stderr, "can't find channel:%d\n", nChannel);
	}
	return m_mapMux.size();
}

#ifdef USE_OPENCV

#ifdef CV_THREE
int CMultiMux::PushStreamData(int nChannel, IplImage* pImg)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(pImg);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		//CImageMuxing *pMux = createMuxer(pImg->width, pImg->height, "flv", "rtmp://192.168.1.103/live", 20);
		//CImageMuxing *pMux = createMuxer(pImg->width, pImg->height, "flv", "1.mp4", 20);
		std::map<int, rtmpconfig*>::iterator iterConfig = m_mapConfig.find(nChannel);
		if (m_mapConfig.end() != iterConfig)
		{
			CImageMuxing *pMux = createMuxer(pImg->width, pImg->height, iterConfig->second->strFmt.c_str(), iterConfig->second->strUrl.c_str(), iterConfig->second->nFrameRate);
			printf("Channel:%d\n", nChannel);
			m_mapMux[nChannel] = pMux;
			ret = pMux->PushStream(pImg);
			m_pLock->UnLock();
			return ret;
		}
		else
		{
			//fprintf(stderr, "Cannot Init RtmpStream, can't find config by this channel, please use PushStreamData(in, const char*, const char*, IplImage*, int) function\n");
		}
	}
	m_pLock->UnLock();
	return -1;
}
#endif

int CMultiMux::PushStreamData(int nChannel, cv::Mat mat)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(mat);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		//CImageMuxing *pMux = createMuxer(mat.rows, mat.cols, "flv", "rtmp://192.168.1.103/live", nFrameRate);
		std::map<int, rtmpconfig*>::iterator iterConfig = m_mapConfig.find(nChannel);
		if (m_mapConfig.end() != iterConfig)
		{
			CImageMuxing *pMux = createMuxer(mat.cols, mat.rows, iterConfig->second->strFmt.c_str(), iterConfig->second->strUrl.c_str(), iterConfig->second->nFrameRate);
			m_mapMux[nChannel] = pMux;
			ret = pMux->PushStream(mat);
			m_pLock->UnLock();
			return ret;
		}
		else
		{
			//fprintf(stderr, "Cannot Init RtmpStream, can't find config by this channel, please use PushStreamData(in, const char*, const char*, mat, int) function\n");
		}
	}
	m_pLock->UnLock();
	return -1;
}

#ifdef CV_THREE
int CMultiMux::PushStreamData(int nChannel, const char* strFmt, const char* strUtl, IplImage* pImg, int nFrameRate)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(pImg);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		CImageMuxing *pMux = createMuxer(pImg->width, pImg->height, strFmt, strUtl, nFrameRate);
		printf("Channel:%d\n", nChannel);
		m_mapMux[nChannel] = pMux;
		ret = pMux->PushStream(pImg);
		m_pLock->UnLock();
		return ret;
	}
	m_pLock->UnLock();
	return -1;
}
#endif

int CMultiMux::PushStreamData(int nChannel, const char* strFmt, const char* strUtl, cv::Mat mat, int nFrameRate)
{
	m_pLock->Lock();
	int ret = 0;
	std::map<int, CImageMuxing*>::iterator iterMux = m_mapMux.find(nChannel);
	if (iterMux != m_mapMux.end())
	{
		ret = iterMux->second->PushStream(mat);
		m_pLock->UnLock();
		return ret;
	}
	else
	{
		//CImageMuxing *pMux = createMuxer(mat.rows, mat.cols, "flv", "rtmp://192.168.1.103/live", nFrameRate);
		CImageMuxing *pMux = createMuxer(mat.rows, mat.cols, strFmt, strUtl, nFrameRate);
		m_mapMux[nChannel] = pMux;
		ret = pMux->PushStream(mat);
		m_pLock->UnLock();
		return ret;
		fprintf(stderr, "Cannot Init RtmpStream, Please check your parameters\n");
	}
	m_pLock->UnLock();
	return -1;
}
#endif
