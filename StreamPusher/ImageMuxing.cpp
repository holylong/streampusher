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


#include "ImageMuxing.h"

#ifndef USE_CMAKE_BUILD
#ifdef _WIN32
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
//#pragma comment(lib, "ImageEncoder.lib")
//#pragma comment(lib, "iconv.lib")

#ifdef USE_OPENCV
#ifndef _DEBUG
#pragma comment(lib, "opencv_world3416.lib")
#else
#pragma comment(lib, "opencv_world3416d.lib")
#endif
//#pragma comment(lib, "opencv_core400.lib")
//#pragma comment(lib, "opencv_highgui400.lib")
//#pragma comment(lib, "opencv_imgproc400.lib")
//#pragma comment(lib, "opencv_imgcodecs400.lib")
#endif  //USE_OPENCV
//#pragma comment(lib, "iniparser.lib")
//#pragma comment(lib, "pthreadVC2.lib")
#endif //_WIN32

#endif //USE_CMAKE_BUILD

#define STREAM_DURATION 50000

int CImageMuxing::ChannelCount = 0;

CImageMuxing* createMuxer(int nWidth, int nHeight, const char* strFmt, const char* pOutUrl, int nFrameRate)
{
	CImageMuxing* pMux = new CImageMuxing(nWidth, nHeight, strFmt, nFrameRate, pOutUrl);
	return pMux;
}

CImageMuxing::CImageMuxing(int nWidth, int nHeight, const char* strFmt, int nFrameRate, const char* chOutputUrl) :m_nWidth(nWidth), m_nHeight(nHeight), m_strFmt(strFmt), m_nFrameRate(nFrameRate), m_strUrl(chOutputUrl)
{
	InitParam();
	InitVideo(nWidth, nHeight, strFmt, nFrameRate, chOutputUrl);
}

CImageMuxing::CImageMuxing()
{

}

CImageMuxing::~CImageMuxing()
{
	UnInit();
}

int CImageMuxing::InitParam()
{
	m_bInit = 0;
	m_nWidth = 0;
	m_nCanPush = false;
	m_nHeight = 0;
	m_strFmt = "";			//????????
	m_strUrl = "";			//??????????
	m_nFrameRate = 0;			//???
	m_nBitRate = 0;			//
	m_strListDir = "";		//???????????��?????????

	m_nHasVideo = 0;
	m_nHasAudio = 0;
	m_nEncodeVideo = 0;
	m_nEncodeAudio = 0;

	m_pOsmVideo = { 0 };
	m_pOsmAudio = { 0 };
	m_pFmt = NULL;			/*??????????*/
	m_pOc = NULL;			/*??????????????*/
	m_pAudioCodec = NULL;			/*???????*/
	m_pVideoCodec = NULL;
	return 0;
}

/* Add an output stream. */
static void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, int nFrameRate, int nWidth, int nHeight)
{
	printf("Add Stream\n");
	AVCodecContext *c;
	int i;
	/* find the encoder */
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec))
	{
		fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
		//exit(1);
	}
	fprintf(stdout, "find encoder for '%s'\n", avcodec_get_name(codec_id));
	ost->st = avformat_new_stream(oc, NULL);
	if (!ost->st)
	{
		fprintf(stderr, "Could not allocate stream\n");
		//exit(1);
	}
	ost->st->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c)
	{
		fprintf(stderr, "Could not alloc an encoding context\n");
		//exit(1);
	}
	ost->enc = c;
	switch ((*codec)->type)
	{
	case AVMEDIA_TYPE_AUDIO:
		printf("Add Audio\n");
		c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 64000;
		c->sample_rate = 44100;
		if ((*codec)->supported_samplerates)
		{
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++)
			{
				if ((*codec)->supported_samplerates[i] == 44100)
					c->sample_rate = 44100;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((*codec)->channel_layouts)
		{
			c->channel_layout = (*codec)->channel_layouts[0];
			for (i = 0; (*codec)->channel_layouts[i]; i++)
			{
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		//ost->st->time_base = (AVRational) { 1, c->sample_rate };
		ost->st->time_base.num = 1;
		ost->st->time_base.den = c->sample_rate;
		break;
	case AVMEDIA_TYPE_VIDEO:
		printf("Add Video %d\n", codec_id);
		c->codec_id = codec_id;
		//c->rc_min_rate = 9000;
		//c->rc_max_rate = 20000000;
		//c->bit_rate = 512000;
		c->bit_rate = 4000000;
		/* Resolution must be a multiple of two. */
		c->width = nWidth;
		c->height = nHeight;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		* of which frame timestamps are represented. For fixed-fps content,
		* timebase should be 1/framerate and timestamp increments should be
		* identical to 1. */
		//ost->st->time_base = (AVRational) { 1, STREAM_FRAME_RATE };
		ost->st->time_base.num = 1;
		ost->st->time_base.den = nFrameRate;
		c->time_base = ost->st->time_base;
		c->gop_size = 1; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = AV_PIX_FMT_YUV420P;

		//h264 encoder profile
		c->profile = FF_PROFILE_H264_HIGH;
		c->level = 4.0;
		//c->pix_fmt = AV_PIX_FMT_RGB24;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
		{
			/* just for testing, we also add B-frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
		{
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			* This does not happen with normal video, it just happens here as
			* the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		break;
	default:
		break;
	}
	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	printf("Add Stream OK\n");
}

/* video output */
static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture;
	int ret;
	picture = av_frame_alloc();
	if (!picture)
		return NULL;
	picture->format = pix_fmt;
	picture->width = width;
	picture->height = height;
	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		//exit(1);
	}
	return picture;
}

static void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
	printf("Open Video\n");
	int ret;
	AVCodecContext *c = ost->enc;
	AVDictionary *opt = NULL;
	av_dict_copy(&opt, opt_arg, 0);
	printf("Open Video 1\n");

	/* open the codec */
	ret = avcodec_open2(c, codec, &opt);
	//ret = avcodec_open2(c, codec, NULL);
	if (ret < 0)
	{
		//fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
		fprintf(stderr, "Could not open video codec: %d\n", ret);
		//exit(1);
	}

#if 1
	printf("Open Video 2\n");
	av_dict_free(&opt);
	printf("Open Video 3\n");
#endif

	/* allocate and init a re-usable frame */
	printf("Open Video 4\n");
	ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
	if (!ost->frame)
	{
		fprintf(stderr, "Could not allocate video frame\n");
		//exit(1);
	}
	/* If the output format is not YUV420P, then a temporary YUV420P
	* picture is needed too. It is then converted to the required
	* output format. */
	ost->tmp_frame = NULL;
	printf("Open Video 5\n");
	if (c->pix_fmt != AV_PIX_FMT_YUV420P)
	{
		ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
		if (!ost->tmp_frame)
		{
			fprintf(stderr, "Could not allocate temporary picture\n");
			//exit(1);
		}
	}
	/* copy the stream parameters to the muxer */
	printf("Open Video 6\n");
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		//exit(1);
	}
	printf("Open Video OK\n");
}

/* audio output */
static AVFrame* alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
	AVFrame *frame = av_frame_alloc();
	int ret;
	if (!frame)
	{
		fprintf(stderr, "Error allocating an audio frame\n");
		//exit(1);
	}
	frame->format = sample_fmt;
	frame->channel_layout = channel_layout;
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;
	if (nb_samples)
	{
		ret = av_frame_get_buffer(frame, 0);
		if (ret < 0)
		{
			fprintf(stderr, "Error allocating an audio buffer\n");
			//exit(1);
		}
	}
	return frame;
}

static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
	AVCodecContext *c;
	int nb_samples;
	int ret;
	AVDictionary *opt = NULL;
	c = ost->enc;
	/* open it */
	av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0)
	{
		//fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
		fprintf(stderr, "Could not open audio codec: %d\n", ret);
		//exit(1);
	}
	/* init signal generator */
	ost->t = 0;
	ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
	/* increment frequency by 110 Hz per second */
	ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
	ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, nb_samples);
	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0)
	{
		fprintf(stderr, "Could not copy the stream parameters\n");
		//exit(1);
	}
	/* create resampler context */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx)
	{
		fprintf(stderr, "Could not allocate resampler context\n");
		//exit(1);
	}
	/* set options */
	av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);
	/* initialize the resampling context */
	if ((ret = swr_init(ost->swr_ctx)) < 0)
	{
		fprintf(stderr, "Failed to initialize the resampling context\n");
		//exit(1);
	}
}

//??????????????????
int CImageMuxing::InitVideo(int nWidth, int nHeight, const char* strFmt, int nFrameRate, const char* chOutputUrl)
{
	if (m_bInit == 1)
		return 0;

	int ret = 0;

	printf("width:%d==height:%d==fmt:%s\n", nWidth, nHeight, strFmt);

	if (m_bInit == 0 && ChannelCount == 0)
	{
		av_register_all();
		avcodec_register_all();
	}
	ChannelCount++;
	AVDictionary *opt = NULL;
	AVDictionaryEntry *t = NULL;
	while ((t = av_dict_get(opt, "", t, AV_DICT_IGNORE_SUFFIX)))
	{
		av_log(NULL, AV_LOG_DEBUG, "%s: %s", t->key, t->value);
	}

	/* allocate the output media context */
	avformat_alloc_output_context2(&m_pOc, NULL, NULL, chOutputUrl);
	if (!m_pOc)
	{
		printf("Could not deduce output format from file extension: using MPEG.\n");
		printf("FrameRate:%d\n", nFrameRate);
		printf("Url:%s\n", chOutputUrl);
		avformat_alloc_output_context2(&m_pOc, NULL, strFmt, chOutputUrl);
	}

	if (!m_pOc)
	{
		printf("OutputStream Failed\n");
		return 1;
	}
	printf("OutputStream OK\n");
	m_pFmt = m_pOc->oformat;

	/* Add the audio and video streams using the default format codecs and initialize the codecs. */
	if (m_pFmt->video_codec != AV_CODEC_ID_NONE)
	{
		add_stream(&m_pOsmVideo, m_pOc, &m_pVideoCodec, m_pFmt->video_codec, nFrameRate, nWidth, nHeight);
		printf("add Stream OK 2\n");
		m_nHasVideo = 1;
		m_nEncodeVideo = 1;
	}
	printf("add Stream OK 3\n");

#ifdef USE_AUDIO
	if (m_pFmt->audio_codec != AV_CODEC_ID_NONE)
	{
		add_stream(&m_pOsmAudio, m_pOc, &m_pAudioCodec, m_pFmt->audio_codec, 0, 0, 0);
		m_nHasAudio = 1;
		m_nEncodeAudio = 1;
	}
#endif
	/* Now that all the parameters are set, we can open the audio and
	* video codecs and allocate the necessary encode buffers. */
	if (m_nHasVideo)
		open_video(m_pOc, m_pVideoCodec, &m_pOsmVideo, opt);

#ifdef USE_AUDIO
	if (m_nHasAudio)
		open_audio(m_pOc, m_pAudioCodec, &m_pOsmAudio, opt);
#endif

	printf("debug destinatin\n");
	av_dump_format(m_pOc, 0, chOutputUrl, 1);
	/* open the output file, if needed */
	if (!(m_pFmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&m_pOc->pb, chOutputUrl, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			//fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(ret));
			fprintf(stderr, "Could not open '%s': %d\n", chOutputUrl, ret);
			return 1;
		}
	}
	/* Write the stream header, if any. */
	ret = avformat_write_header(m_pOc, &opt);
	if (ret < 0)
	{
		//fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
		fprintf(stderr, "Error occurred when opening output file: %d\n", ret);
		return 1;
	}

	m_bInit = 1;
	m_nCanPush = true;
	return 0;
}

int CImageMuxing::InitVideo(const char* strListDir, const char* strFmt, int nFrameRate, const char* chOutputUrl)
{
	return 0;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
	avcodec_free_context(&ost->enc);
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}

int CImageMuxing::UnInit()
{
	/*av_codec_close()*/
	av_write_trailer(m_pOc);
	/* Close each codec. */
	if (m_nHasVideo)
		close_stream(m_pOc, &m_pOsmVideo);
	if (m_nHasAudio)
		close_stream(m_pOc, &m_pOsmAudio);
	if (!(m_pFmt->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_closep(&m_pOc->pb);
	/* free the stream */
	avformat_free_context(m_pOc);

	ChannelCount--;

	if (ChannelCount == 0)
	{
		printf("Has No Channel\n");
	}
	return 0;
}

/* Prepare a dummy image. */
static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
	int x, y, i;
	i = frame_index;
	/* Y */
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
	/* Cb and Cr */
	for (y = 0; y < height / 2; y++)
	{
		for (x = 0; x < width / 2; x++)
		{
			pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
			pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
		}
	}
}

static AVFrame *get_video_frame(OutputStream *ost, uint8_t* pRgb)
{
	AVCodecContext *c = ost->enc;
	/* check if we want to generate more frames */
	AVRational avrat;
	avrat.num = 1;
	avrat.den = 1;
#if 0
	if (av_compare_ts(ost->next_pts, c->time_base, STREAM_DURATION, avrat) >= 0)
		return NULL;
#endif
	/* when we pass a frame to the encoder, it may keep a reference to it
	* internally; make sure we do not overwrite it here */
	if (av_frame_make_writable(ost->frame) < 0)
	{
		fprintf(stderr, "av_frame_make_writable error\n");
		//exit(1);
	}
	//if (c->pix_fmt != AV_PIX_FMT_YUV420P)
	if (1)
	{
		if (ost->sws_ctx == NULL)
		{
			ost->sws_ctx = sws_getContext(ost->enc->width, ost->enc->height, AV_PIX_FMT_BGR24, ost->enc->width, ost->enc->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		}

		if (ost->sws_ctx != NULL)
		{
			AVPicture avSrc;
			avpicture_fill(&avSrc, pRgb, AV_PIX_FMT_RGB24, ost->enc->width, ost->enc->height);
			sws_scale(ost->sws_ctx, avSrc.data, avSrc.linesize, 0, c->height, ost->frame->data, ost->frame->linesize);
		}
	}
	else {
		fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
	}
	ost->frame->pts = ost->next_pts++;
	return ost->frame;
}

char* MyPrintTS(char* buf, uint64_t ts)
{
	if (ts == AV_NOPTS_VALUE) snprintf(buf, AV_TS_MAX_STRING_SIZE, "NOPTS");
	else                      snprintf(buf, AV_TS_MAX_STRING_SIZE, "%ld", ts);
	return buf;
}

char *MyPrintTimeStr(char *buf, int64_t ts, AVRational *tb)
{
	if (ts == AV_NOPTS_VALUE) snprintf(buf, AV_TS_MAX_STRING_SIZE, "NOPTS");
	else                      snprintf(buf, AV_TS_MAX_STRING_SIZE, "%.6lf", av_q2d(*tb) * ts);
	return buf;
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
	char buf[AV_TS_MAX_STRING_SIZE];
	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
#if 0
	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
		av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
		av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
		pkt->stream_index);
#endif

#if 0
	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		MyPrintTS(buf, pkt->pts), MyPrintTimeStr(buf, pkt->pts, time_base),
		MyPrintTS(buf, pkt->dts), MyPrintTimeStr(buf, pkt->dts, time_base),
		MyPrintTS(buf, pkt->duration), MyPrintTimeStr(buf, pkt->duration, time_base),
		pkt->stream_index);
#endif
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, st->time_base);
	pkt->stream_index = st->index;

	/* Write the compressed frame to the media file. */
	log_packet(fmt_ctx, pkt);
	//printf("Write Frame Begin\n");
	int ret = av_interleaved_write_frame(fmt_ctx, pkt);
	//printf("Write Frame End\n");
	return ret;
}

static int write_audio_frame(AVFormatContext *oc, OutputStream *ost, uint8_t* pcm)
{
	return 0;
}

/*
* encode one video frame and send it to the muxer
* return 1 when encoding is finished, 0 otherwise
*/
static int write_video_frame(AVFormatContext *oc, OutputStream *ost, uint8_t* pRgb)
{
	int ret;
	AVCodecContext *c;
	AVFrame *frame;
	int got_packet = 0;
	AVPacket pkt = { 0 };
	c = ost->enc;
	frame = get_video_frame(ost, pRgb);
	av_init_packet(&pkt);
	/* encode the image */
	ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
	if (ret < 0)
	{
		//fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
		fprintf(stderr, "Error encoding video frame: %d\n", ret);
		//exit(1);
	}
	if (got_packet)
	{
		//fprintf(stdout, "lll Error while writing video frame: %d\n", ret);
		ret = write_frame(oc, &c->time_base, ost->st, &pkt);
		//fprintf(stdout, "llll Error while writing video frame: %d\n", ret);
	}
	else
	{
		ret = 0;
	}
	if (ret < 0)
	{
		//fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));
		//fprintf(stderr, "Error while writing video frame: %d\n", ret);
		//exit(1);
	}
	frame->pts++;
	return (frame || got_packet) ? 0 : 1;
}

int CImageMuxing::PushStream(uint8_t* pRgb)
{
	if (!m_nCanPush)
		return -1;
	if (m_bInit)
		return write_video_frame(m_pOc, &m_pOsmVideo, pRgb);
	else
		return -1;
}

int CImageMuxing::PushAudioStream(uint8_t* pcm)
{
	if (!m_nCanPush)
		return -1;
	if (m_bInit)
		return write_audio_frame(m_pOc, &m_pOsmAudio, pcm);
	else
		return -1;
}

#ifdef USE_OPENCV
int CImageMuxing::PushStream(cv::Mat  mat)
{
	if (!m_nCanPush)
		return -1;
	if (m_bInit)
		return write_video_frame(m_pOc, &m_pOsmVideo, mat.data);
	else
	{
		InitVideo(mat.cols, mat.rows, m_strFmt.c_str(), 25, m_strUrl.c_str());
		return write_video_frame(m_pOc, &m_pOsmVideo, mat.data);
	}
	return 0;
}

#ifdef CV_THREE
int CImageMuxing::PushStream(IplImage* pImg)
{
	if (!m_nCanPush)
		return -1;
	if (m_bInit)
		return write_video_frame(m_pOc, &m_pOsmVideo, (uint8_t*)pImg->imageData);
	else
	{
		InitVideo(pImg->width, pImg->height, m_strFmt.c_str(), 25, m_strUrl.c_str());
		return write_video_frame(m_pOc, &m_pOsmVideo, (uint8_t*)pImg->imageData);
	}
	return 0;
}
#endif
#endif
