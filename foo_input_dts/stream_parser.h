/*
	Stream parser class
*/

#ifndef _STREAM_PARSER_H_
#define _STREAM_PARSER_H_

#include <foobar2000.h>
#include "parser.h"

class StreamParser // : public Source
{
protected:
	service_ptr_t<file> m_file;
	abort_callback & p_abort;

	StreamBuffer stream;

	uint8_t *buf;
	size_t buf_size;
	size_t buf_data;
	size_t buf_pos;

	size_t stat_size;			// number of measurments done by stat() call

public:
	float avg_frame_interval;	// average frame interval
	float avg_bitrate;			// average bitrate	size_t max_scan;
	size_t max_scan;
	enum units_t { bytes, relative, frames, time };
	inline double units_factor(units_t units) const;

	StreamParser(service_ptr_t<file> _m_file, abort_callback & _p_abort, const HeaderParser *_parser, size_t _max_scan);
	~StreamParser();

	/////////////////////////////////////////////////////////////////////////////
	// Stream operations

	bool probe();
	bool stats(unsigned max_measurments = 100, vtime_t precision = 0.5);

	bool eos();

	const HeaderParser *get_parser() const { return stream.get_parser(); }

	size_t file_info(char *buf, size_t size) const;

	/////////////////////////////////////////////////////////////////////////////
	// Positioning

	t_filesize get_pos() const;
	double get_pos(units_t units) const;

	t_filesize get_size() const;
	double get_size(units_t units) const;

	int seek(t_filesize pos);
	int seek(double pos, units_t units);

	/////////////////////////////////////////////////////////////////////////////
	// Frame-level interface (StreamBuffer interface wrapper)

	void reset();
	bool load_frame();

	bool is_in_sync()				const { return stream.is_in_sync(); }
	bool is_new_stream()			const { return stream.is_new_stream(); }
	bool is_frame_loaded()			const { return stream.is_frame_loaded(); }

	Speakers get_spk()				const { return stream.get_spk(); }
	uint8_t *get_frame()			const { return stream.get_frame(); }
	size_t get_frame_size()			const { return stream.get_frame_size(); }
	size_t get_frame_interval()		const { return stream.get_frame_interval(); }

	int get_frames()				const { return stream.get_frames(); }
	size_t stream_info(char *buf, size_t size) const { return stream.stream_info(buf, size); }
	HeaderInfo header_info()		const { return stream.header_info(); }
};

#endif
