#include "dts_decode.h"

const char *_ch_names[NCHANNELS] = { "Left", "Center", "Right", "Left surround", "Right surround", "LFE" };

const int mask_tbl[] =
{
    0,
    MODE_MONO,
    MODE_STEREO,
    MODE_3_0,
    MODE_2_2,
    MODE_3_2,
    MODE_5_1
};

const int format_tbl[] =
{
    FORMAT_PCM16,
    FORMAT_PCM24,
    FORMAT_PCM32,
    FORMAT_PCM16_BE,
    FORMAT_PCM24_BE,
    FORMAT_PCM32_BE,
    FORMAT_PCMFLOAT,
};

const sample_t level_tbl[] =
{
    32767,
    8388607,
    2147483647,
    1.0,
    32767,
    8388607,
    2147483647,
    1.0
};

const HeaderParser *parser_list[] =
{
    &spdif_header,
    &ac3_header,
    &dts_header,
    &mpa_header
};

dts_decode::dts_decode()
{
    /////////////////////////////////////////////////////////
    // Parsers

    stream_parser = 0;
    header_parser = 0;
    multi_parser = new MultiHeader(parser_list, array_size(parser_list));

    /////////////////////////////////////////////////////////
    // Arrays

    delay_units = DELAY_SP;

    int i;
    for (i = 0; i < NCHANNELS; i++)
    {
        delays[i] = 0;
    }

    for (i = 0; i < NCHANNELS; i++)
    {
        gains[i] = 1.0;
    }

    reset_stats();
}

dts_decode::~dts_decode()
{
    /////////////////////////////////////////////////////
    // Flushing

    chunk.set_empty(dvd_graph.get_input());
    chunk.eos = true;

    delete multi_parser;
    if (stream_parser != 0)
    {
        delete stream_parser;
        stream_parser = 0;
    }
}

bool dts_decode::load(service_ptr_t<file> m_file, abort_callback & p_abort)
{
    reset_stats();

    if (!header_parser)
    {
        header_parser = multi_parser;
    }

    stream_parser = new StreamParser(m_file, p_abort, header_parser, 1000000);

    if (!stream_parser->stats())
    {
        return false;
    }

    while (!stream_parser->eos())
    {
        if (stream_parser->load_frame())
        {
            break;
        }
    }

    if (!stream_parser->is_frame_loaded())
    {
        return false;
    }

    // Set stream info
    length = int(stream_parser->get_size(StreamParser::units_t::time));
    frames = int(stream_parser->get_size(StreamParser::units_t::frames));
    avg_frame_interval = stream_parser->avg_frame_interval;
    avg_bitrate = stream_parser->avg_bitrate;
    sample_rate = stream_parser->get_spk().sample_rate;

    format = "";
    switch (stream_parser->get_spk().format)
    {
    case FORMAT_DTS:
        format = "DTS";
        break;
    case FORMAT_AC3:
        format = "AC3";
        break;
    }

    return true;
}

void dts_decode::initialize(struct dts_config config)
{
    /////////////////////////////////////////////////////////
    // Setup processing
    /////////////////////////////////////////////////////////

    dvd_graph.proc.set_auto_matrix(config.auto_matrix);
    dvd_graph.proc.set_normalize_matrix(config.normalize_matrix);
    dvd_graph.proc.set_voice_control(config.voice_control);
    dvd_graph.proc.set_expand_stereo(config.expand_stereo);
    dvd_graph.proc.set_auto_gain(config.auto_gain);
    dvd_graph.proc.set_normalize(config.normalize);
    dvd_graph.proc.set_drc(config.drc);

    dvd_graph.proc.set_delay_units(delay_units);
    dvd_graph.proc.set_delays(delays);

    dvd_graph.proc.set_output_gains(gains);

    dvd_graph.proc.set_input_order(std_order);
    dvd_graph.proc.set_output_order(win_order);

    user_spk.set(format_tbl[config.format], mask_tbl[config.speakers], 0, level_tbl[config.format]);
    dvd_graph.set_user(user_spk);

    dvd_graph.set_input(stream_parser->get_spk());

    stream_parser->reset();
}

size_t dts_decode::decode_frame(pfc::array_t<t_uint8> *sample_buffer)
{
    /////////////////////////////////////////////////////////
    // Process
    /////////////////////////////////////////////////////////

    while (!stream_parser->eos())
    {
        stream_parser->load_frame();

        if (stream_parser->is_frame_loaded())
        {
            /////////////////////////////////////////////////////
            // Switch to a new stream

            if (stream_parser->is_new_stream())
            {
            }

            /////////////////////////////////////////////////////
            // Process data

            chunk.set_rawdata(stream_parser->get_spk(), stream_parser->get_frame(), stream_parser->get_frame_size());
            if (!dvd_graph.process(&chunk))
            {
                return 0;
            }

            while (!dvd_graph.is_empty())
            {
                if (!dvd_graph.get_chunk(&chunk))
                {
                    return 0;
                }

                /////////////////////////////////////////////////////
                // Do audio output

                if (!chunk.is_dummy())
                {
                    sample_buffer->set_size(chunk.size);
                    memcpy(sample_buffer->get_ptr(), chunk.rawdata, chunk.size);
                    return chunk.size;
                }
            } // while (!dvd_graph.is_empty())
        } // if(stream_parser->is_frame_loaded())
    } // while (!stream_parser->eos())

    return 0;
}

void dts_decode::seek(double seconds)
{
    stream_parser->seek(seconds, StreamParser::units_t::time);
}

void dts_decode::reset_stats()
{
    length = 0;
    frames = 0;
    avg_frame_interval = 0;
    avg_bitrate = 0;
    sample_rate = 0;
}