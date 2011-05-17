#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <foobar2000.h>
#include "stream_parser.h"


#define FLOAT_THRESHOLD 1e-20
static const size_t max_buf_size = 65536;

#define SAFE_SEEK(pos)	if (m_file->can_seek()) m_file->seek(pos, p_abort)
#define GET_FILE_POS()	m_file->get_position(p_abort)
#define GET_FILE_SIZE()	m_file->get_size(p_abort)
#define IS_EOF()		m_file->is_eof(p_abort)

int compact_size(t_filesize size)
{
	int iter = 0;
	while (size >= 10000 && iter < 5)
	{
		size /= 1024;
		iter++;
	}

	return (int)size;
}

const char *compact_suffix(t_filesize size)
{
	static const char *suffixes[] = { "", "K", "M", "G", "T", "P" };

	int iter = 0;
	while (size >= 10000 && iter < 5)
	{
		size /= 1024;
		iter++;
	}

	return suffixes[iter];
}

StreamParser::StreamParser(service_ptr_t<file> _m_file, abort_callback & _p_abort, const HeaderParser *_parser, size_t _max_scan) 
	: m_file(_m_file), p_abort(_p_abort)
{
	SAFE_SEEK(0);

	buf = new uint8_t[max_buf_size];
	buf_size = buf? max_buf_size: 0;
	buf_data = 0;
	buf_pos = 0;

	stat_size = 0;
	avg_frame_interval = 0;
	avg_bitrate = 0;

	stream.set_parser(_parser);

	max_scan = _max_scan;

	reset();
}

StreamParser::~StreamParser()
{
	stream.release_parser();

	stat_size = 0;
	avg_frame_interval = 0;
	avg_bitrate = 0;

	max_scan = 0;

	safe_delete(buf);
}

///////////////////////////////////////////////////////////////////////////////
// Stream operations

bool 
StreamParser::probe()
{
	t_filesize old_pos = GET_FILE_POS();
	bool result = load_frame();
	SAFE_SEEK(old_pos);

	return result;
}

bool
StreamParser::stats(unsigned max_measurments, vtime_t precision)
{
	t_filesize old_pos = GET_FILE_POS();

	// If we cannot load a frame we will not gather any stats.
	// (If file format is unknown measurments may take much of time)
	if (!load_frame())
	{
		SAFE_SEEK(old_pos);
		return false;
	}

	stat_size = 0;
	avg_frame_interval = 0;
	avg_bitrate = 0;

	vtime_t old_length;
	vtime_t new_length;

	old_length = 0;
	for (unsigned i = 0; i < max_measurments; i++)
	{
		t_filesize file_pos = t_filesize((double)rand() * GET_FILE_SIZE() / RAND_MAX);
		SAFE_SEEK(file_pos);

		if (!load_frame())
			continue;

		///////////////////////////////////////////////////////
		// Update stats

		HeaderInfo hinfo = stream.header_info();

		stat_size++;
		avg_frame_interval += stream.get_frame_interval();
		avg_bitrate	+= float(stream.get_frame_interval() * 8 * hinfo.spk.sample_rate) / hinfo.nsamples;

		///////////////////////////////////////////////////////
		// Finish scanning if we have enough accuracy

		if (precision > FLOAT_THRESHOLD)
		{
			new_length = double(GET_FILE_SIZE()) * 8 * stat_size / avg_bitrate;
			if (stat_size > 10 && fabs(old_length - new_length) < precision)
				break;
			old_length = new_length;
		}
	}

	if (stat_size)
	{
		avg_frame_interval /= stat_size;
		avg_bitrate	/= stat_size;
	}

	seek(old_pos);
	return stat_size > 0;
}

bool 
StreamParser::eos() 
{ 
	return IS_EOF() && (buf_pos >= buf_data) && !stream.is_frame_loaded(); 
}

size_t 
StreamParser::file_info(char *buf, size_t size) const
{
	char info[1024];

	t_filesize file_size = GET_FILE_SIZE();

	size_t len = sprintf(info,
		"Size: %.0f (%i %sB)\n",
		(double)file_size, compact_size(file_size), compact_suffix(file_size));

	int file_size_time = int(get_size(time));
	int file_size_frames = int(get_size(frames));

	if (stat_size)
		len += sprintf(info + len,
			"Length: %i:%02i:%02i\n"
			"Frames: %i\n"
			"Frame interval: %i\n"
			"Bitrate: %ikbps\n",
			file_size_time / 3600, file_size_time % 3600 / 60, file_size_time % 60,
			file_size_frames, 
			int(avg_frame_interval),
			int(avg_bitrate / 1000));

	if (len + 1 > size) len = size - 1;
	memcpy(buf, info, len + 1);
	buf[len] = 0;
	return len;
}

///////////////////////////////////////////////////////////////////////////////
// Positioning

inline double
StreamParser::units_factor(units_t units) const
{
	switch (units)
	{
		case bytes:			return 1.0;
		case relative:		return 1.0 / GET_FILE_SIZE();
	}

	if (stat_size)
	{
		switch (units)
		{
			case frames:	return 1.0 / avg_frame_interval;
			case time:		return 8.0 / avg_bitrate;
		}
	}

	return 0.0;
}

t_filesize
StreamParser::get_pos() const
{
	return GET_FILE_POS() - (t_filesize)buf_data + (t_filesize)buf_pos;
}

double 
StreamParser::get_pos(units_t units) const
{
	return get_pos() * units_factor(units);
}

t_filesize
StreamParser::get_size() const
{
	return GET_FILE_SIZE();
}

double 
StreamParser::get_size(units_t units) const
{
	return get_size() * units_factor(units);
}

int
StreamParser::seek(t_filesize pos)
{
	SAFE_SEEK(pos);

	buf_data = 0;
	buf_pos = 0;

	stream.reset();

	return 0;
}

int
StreamParser::seek(double pos, units_t units)
{ 
	double factor = units_factor(units);
	if (factor > FLOAT_THRESHOLD)
	{
		return seek(t_filesize(pos / factor));
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Frame-level interface (StreamBuffer interface wrapper)

void
StreamParser::reset()
{
	SAFE_SEEK(0);

	buf_data = 0;
	buf_pos = 0;

	stream.reset();
}

bool
StreamParser::load_frame()
{
	size_t sync_size = 0;

	while (1)
	{
		///////////////////////////////////////////////////////
		// Load a frame

		uint8_t *pos = buf + buf_pos;
		uint8_t *end = buf + buf_data;
		if (stream.load_frame(&pos, end))
		{
			buf_pos = pos - buf;
			return true;
		}

		///////////////////////////////////////////////////////
		// Stop file scanning if scanned too much

		sync_size += (pos - buf) - buf_pos;
		buf_pos = pos - buf;
		if (max_scan > 0) // do limiting
		{
			if ((sync_size > stream.get_parser()->max_frame_size() * 3) && // minimum required to sync and load a frame
					(sync_size > max_scan))																		// limit scanning
				return false;
		}

		///////////////////////////////////////////////////////
		// Fill the buffer

		if (!buf_data || buf_pos >= buf_data)
		{
			/* Move the data
			if (buf_data && buf_pos)
				memmove(buf, buf + buf_pos, buf_data - buf_pos);
			buf_pos = 0;
			buf_data -= buf_pos;
			*/

			buf_pos = 0;
			buf_data = m_file->read(buf, max_buf_size, p_abort);

			if (!buf_data) return false;
		}
	}

	// never be here
	return false;
}
