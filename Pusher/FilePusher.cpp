/**
 * @file FilePusher.cpp
 * @author holylong (mrhlingchen@163.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <MultiMux.h>
#include <filenameio.h>

#ifndef USE_CMAKE_BUILD
#ifndef _DEBUG
#pragma comment(lib, "opencv_world400.lib")
#else
#pragma comment(lib, "opencv_world3416d.lib")
#pragma comment(lib, "StreamPusher.lib")
#endif

#pragma comment(lib, "DXGICapture.lib")
#endif //USE_CMAKE_BUILD

int main(int argc, char* argv[])
{
	
	CMultiMux* pMux = createMultiMux();
	
	pMux->LoadConfig("decodeconfig.ini");

	std::vector<std::string> m_arrImagePath;
	scanDir("./data/", m_arrImagePath, 1, false);

	
	std::vector<std::string>::iterator iterImage = m_arrImagePath.begin();
	IplImage *pImg = NULL;
	char chPath[40];
	for(; iterImage != m_arrImagePath.end(); iterImage++)
	{
		sprintf(chPath, "./data/%s", (*iterImage).c_str());
		pImg = cvLoadImage(chPath);
		//支持 opencv 3
		// pMux->PushStreamData(0, pImg);
		pMux->PushStreamData(0, (uint8_t*)pImg->imageData);
		if(iterImage == m_arrImagePath.end() - 1)
			iterImage = m_arrImagePath.begin();		
	}
	
	delete pMux;
	pMux = NULL;
	

	return 0;
}
