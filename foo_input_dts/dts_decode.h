#ifndef _DTS_DECODE_H_
#define _DTS_DECODE_H_

#include <foobar2000.h>

// parsers
#include "stream_parser.h"
#include "parsers/ac3/ac3_header.h"
#include "parsers/dts/dts_header.h"
#include "parsers/mpa/mpa_header.h"
#include "parsers/spdif/spdif_header.h"
#include "parsers/multi_header.h"

// filters
#include "filters/dvd_graph.h"

struct dts_config
{
	int format;
	int speakers;
	bool auto_matrix;
	bool normalize_matrix;
	bool voice_control;
	bool expand_stereo;
	bool auto_gain;
	bool normalize;
	bool drc;
};

class dts_decode
{
private:
    int delay_units;
    float delays[NCHANNELS];
    sample_t gains[NCHANNELS];

	int iformat;
    int imask;

    StreamParser *stream_parser;
	const HeaderParser *header_parser;
	MultiHeader *multi_parser;

	DVDGraph dvd_graph;

	Speakers user_spk;

	Chunk chunk;

	void reset_stats();

public:
	int length;
	int frames;
	float avg_frame_interval;
	float avg_bitrate;
	int sample_rate;
	char *format;

	dts_decode();
	~dts_decode();

	bool load(service_ptr_t<file> m_file, abort_callback & p_abort);
	void initialize(struct dts_config config);
    size_t decode_frame(pfc::array_t<t_uint8> *sample_buffer);
	void seek(double seconds);
};

#endif
