#include <foobar2000.h>
#include "../ATLHelpers/ATLHelpers.h"
#include "preferences.h"
#include "dts_decode.h"

#define COMPONENT_NAME		"AC3/DTS Decoder"
#define COMPONENT_VERSION	"0.1"

class input_dts
{
private:
	dts_decode * m_decoder;
	struct dts_config config;

	t_filestats m_stats;
	bool first_block;

	pfc::array_t<t_uint8> sample_buffer;

	unsigned int get_bps_for_format(int format)
	{
		if ((format == 0) || (format == 3))
		{
			return 16;
		}
		else if ((format == 1) || (format == 4))
		{
			return 24;
		}
		else if ((format == 2) || (format == 5))
		{
			return 32;
		}
		else if (format == 6)
		{
			return 32;
		}

		return 0;
	}

	unsigned int get_flags_for_format(int format)
	{
		if ((format == 0) || (format == 1) || (format == 2))
		{
			return audio_chunk::FLAG_SIGNED | audio_chunk::FLAG_LITTLE_ENDIAN;
		}
		else if ((format == 3) || (format == 4) || (format == 5))
		{
			return audio_chunk::FLAG_SIGNED | audio_chunk::FLAG_BIG_ENDIAN;
		}
		else if (format == 6)
		{
			return audio_chunk::FLAG_BIG_ENDIAN;
		}

		return 0;
	}

	unsigned int get_channel_config_for_speakers(int speakers)
	{
		unsigned int config = 0;

		switch (speakers)
		{
			case 1:
				config = audio_chunk::channel_config_mono;
				break;
			case 2:
				config = audio_chunk::channel_config_stereo;
				break;
			case 3:
				config = audio_chunk::channel_front_left | audio_chunk::channel_front_right |
						 audio_chunk::channel_front_center;
				break;
			case 4:
				config = audio_chunk::channel_front_left | audio_chunk::channel_front_right |
						 audio_chunk::channel_back_left | audio_chunk::channel_back_right;
				break;
			case 5:
				config = audio_chunk::channel_front_left | audio_chunk::channel_front_right |
						 audio_chunk::channel_back_left | audio_chunk::channel_back_right |
						 audio_chunk::channel_front_center;
				break;
			case 6:
				config = audio_chunk::channel_config_5point1;
				break;
		}

		return config;
	}

public:
	input_dts()
	{
		m_decoder = 0;

		config.format = cfg_format;
		config.speakers = cfg_speakers;
		config.auto_matrix = inttobool(cfg_auto_matrix);
		config.normalize_matrix = inttobool(cfg_normalize_matrix);
		config.voice_control = inttobool(cfg_voice_control);
		config.expand_stereo = inttobool(cfg_expand_stereo);
		config.auto_gain = inttobool(cfg_auto_gain);
		config.normalize = inttobool(cfg_normalize);
		config.drc = inttobool(cfg_drc);
	}

	~input_dts()
	{
		if (m_decoder != 0)
		{
			delete m_decoder;
			m_decoder = 0;
		}
	}

	void open(service_ptr_t<file> m_file, const char * p_path, t_input_open_reason p_reason, abort_callback & p_abort)
	{
		if (p_reason == input_open_info_write)
		{
			throw exception_io_data();
		}

		input_open_file_helper(m_file, p_path, p_reason, p_abort);
		m_stats = m_file->get_stats(p_abort);
		m_decoder = new dts_decode();

		if (!m_decoder->load( m_file, p_abort))
		{
			throw exception_io_data();
		}
	}

	void get_info(file_info & p_info, abort_callback & p_abort)
	{
		p_info.info_set_int("bitrate", (t_int64)m_decoder->avg_bitrate / 1000);
		p_info.info_set_int("samplerate", m_decoder->sample_rate);
		p_info.info_set_int("bitspersample", get_bps_for_format(config.format));
		p_info.info_set_int("channels", config.speakers);
		p_info.info_set("codec", m_decoder->format);
		p_info.set_length( (double) m_decoder->length);
	}

	t_filestats get_file_stats(abort_callback & p_abort)
	{
		return m_stats;
	}

	void decode_initialize(unsigned p_flags, abort_callback & p_abort)
	{
		first_block = false;

		config.format = cfg_format;
		config.speakers = cfg_speakers;
		config.auto_matrix = inttobool(cfg_auto_matrix);
		config.normalize_matrix = inttobool(cfg_normalize_matrix);
		config.voice_control = inttobool(cfg_voice_control);
		config.expand_stereo = inttobool(cfg_expand_stereo);
		config.auto_gain = inttobool(cfg_auto_gain);
		config.normalize = inttobool(cfg_normalize);
		config.drc = inttobool(cfg_drc);

		m_decoder->initialize(config);
	}

	bool decode_run(audio_chunk & p_chunk, abort_callback & p_abort)
	{
		size_t data_size = m_decoder->decode_frame(&sample_buffer);

		if (config.format == 6)
		{
			p_chunk.set_data_floatingpoint_ex(
				sample_buffer.get_ptr(),
				data_size,
				m_decoder->sample_rate,
				config.speakers,
				get_bps_for_format(config.format),
				get_flags_for_format(config.format),
				get_channel_config_for_speakers(config.speakers));
		}
		else
		{
			p_chunk.set_data_fixedpoint_ex(
				sample_buffer.get_ptr(),
				data_size,
				m_decoder->sample_rate,
				config.speakers,
				get_bps_for_format(config.format),
				get_flags_for_format(config.format),
				get_channel_config_for_speakers(config.speakers));
		}

		return (data_size > 0);
	}

	void decode_seek(double p_seconds, abort_callback & p_abort)
	{
		m_decoder->seek(p_seconds);

		first_block = true;
	}

	bool decode_can_seek()
	{
		return true;
	}

	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta)
	{
		if (first_block)
		{
			p_out.info_set_int("samplerate", m_decoder->sample_rate);
			first_block = false;
			return true;
		}

		return false;
	}

	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta)
	{
		return false;
	}

	void decode_on_idle(abort_callback & p_abort)
	{
	}

	void retag(const file_info & p_info, abort_callback & p_abort)
	{
		throw exception_io_data();
	}

	static bool g_is_our_content_type(const char * p_content_type)
	{
		return false;
	}

	static bool g_is_our_path(const char * p_path, const char * p_extension)
	{
		return !stricmp(p_extension, "ac3") || !stricmp(p_extension, "dts") || !stricmp(p_extension, "dtswav");
	}
};


class preferences_page_myimpl : public preferences_page_impl<CMyPreferences>
{
	// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
	const char * get_name() { return COMPONENT_NAME; }
	GUID get_guid()
	{
		// {9EF0B336-91D1-4E03-AFE8-E514D180D057}
		static const GUID guid = { 0x9EF0B336, 0x91D1, 0x4E03, { 0xAF, 0xE8, 0xE5, 0x14, 0xD1, 0x80, 0xD0, 0x57 } };
		return guid;
	}

	GUID get_parent_guid() { return guid_input; }
};

class dts_file_types : public input_file_type
{
	virtual unsigned get_count()
	{
		return 3;
	}

	virtual bool get_name(unsigned idx, pfc::string_base & out)
	{
		static const char * names[] = { "AC3 files", "DTS files", "WAV encapsulated DTS files" };
		if (idx > 1) return false;
		out = names[ idx ];
		return true;
	}

	virtual bool get_mask(unsigned idx, pfc::string_base & out)
	{
		static const char * extensions[] = { "ac3", "dts", "dtswav" };
		out = "*.";
		if (idx > 1) return false;
		out += extensions[ idx ];
		return true;
	}

	virtual bool is_associatable(unsigned idx)
	{
		return true;
	}
};

static input_singletrack_factory_t<input_dts> g_input_dts_factory;
static preferences_page_factory_t<preferences_page_myimpl> g_config_mod_factory;
static service_factory_single_t<dts_file_types> g_input_file_type_dts_factory;

DECLARE_COMPONENT_VERSION(COMPONENT_NAME, COMPONENT_VERSION, COMPONENT_NAME" v"COMPONENT_VERSION);
VALIDATE_COMPONENT_FILENAME("foo_input_dts.dll");
