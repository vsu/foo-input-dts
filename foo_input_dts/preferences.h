#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <foobar2000.h>
#include "../ATLHelpers/ATLHelpers.h"
#include "resource.h"

#define inttobool(v)	(v != 0)

#define default_cfg_format				0
#define default_cfg_speakers			2
#define default_cfg_auto_matrix			0
#define default_cfg_normalize_matrix	0
#define default_cfg_voice_control		0
#define default_cfg_expand_stereo		0
#define default_cfg_auto_gain			0
#define default_cfg_normalize			0
#define default_cfg_drc					0

// {9A5EFB4D-3FE9-4FD0-91D1-9CA51CA9E52D}
static const GUID guid_cfg_format =
{ 0x9A5EFB4D, 0x3FE9, 0x4FD0, { 0x91, 0xD1, 0x9C, 0xA5, 0x1C, 0xA9, 0xE5, 0x2D } };

// {BB2E574A-C7FC-4029-A690-E8FD95F1C87F}
static const GUID guid_cfg_speakers =
{ 0xBB2E574A, 0xC7FC, 0x4029, { 0xA6, 0x90, 0xE8, 0xFD, 0x95, 0xF1, 0xC8, 0x7F } };

// {C6F56C3E-D730-4342-B040-BB0E7904C42E}
static const GUID guid_cfg_auto_matrix =
{ 0xC6F56C3E, 0xD730, 0x4342, { 0xB0, 0x40, 0xBB, 0x0E, 0x79, 0x04, 0xC4, 0x2E } };

// {948427E3-6B21-42B2-BF51-B687DD580B05}
static const GUID guid_cfg_normalize_matrix =
{ 0x948427E3, 0x6B21, 0x42B2, { 0xBF, 0x51, 0xB6, 0x87, 0xDD, 0x58, 0x0B, 0x05 } };

// {DD6F1187-997F-49E3-B908-46399FAE191D}
static const GUID guid_cfg_voice_control =
{ 0xDD6F1187, 0x997F, 0x49E3, { 0xB9, 0x08, 0x46, 0x39, 0x9F, 0xA3, 0x19, 0x1D } };

// {8E994059-274F-40E8-822B-E72A55CEA63A}
static const GUID guid_cfg_expand_stereo =
{ 0x8E994059, 0x274F, 0x40E8, { 0x82, 0x2B, 0xE7, 0x2A, 0x55, 0xCE, 0xA6, 0x3A } };

// {503EB9F3-739A-48F6-8C36-4C40B58F2BE2}
static const GUID guid_cfg_auto_gain =
{ 0x503EB9F3, 0x739A, 0x48F6, { 0x8C, 0x36, 0x4C, 0x40, 0xB5, 0x8F, 0x2B, 0xE2 } };

// {F1DD7C77-EB01-46D4-A504-28A626C89A0C}
static const GUID guid_cfg_normalize =
{ 0xF1DD7C77, 0xEB01, 0x46D4, { 0xA5, 0x04, 0x28, 0xA6, 0x26, 0xC8, 0x9A, 0x0C } };

// {A44282C9-2AA0-4821-804E-1A9371FA7C66}
static const GUID guid_cfg_drc =
{ 0xA44282C9, 0x2AA0, 0x4821, { 0x80, 0x4E, 0x1A, 0x93, 0x71, 0xFA, 0x7C, 0x66 } };

extern cfg_int cfg_format;
extern cfg_int cfg_speakers;
extern cfg_int cfg_auto_matrix;
extern cfg_int cfg_normalize_matrix;
extern cfg_int cfg_voice_control;
extern cfg_int cfg_expand_stereo;
extern cfg_int cfg_auto_gain;
extern cfg_int cfg_normalize;
extern cfg_int cfg_drc;

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance
{
public:
    //Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
    CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

    //Note that we don't bother doing anything regarding destruction of our class.
    //The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.

    //dialog resource ID
    enum { IDD = IDD_CONFIG };

    // preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
    t_uint32 get_state();
    void apply();
    void reset();

    //WTL message map
    BEGIN_MSG_MAP(CMyPreferences)
    MSG_WM_INITDIALOG(OnInitDialog)
    COMMAND_HANDLER_EX(IDC_FORMAT, CBN_SELCHANGE, OnSelectionChange)
    COMMAND_HANDLER_EX(IDC_SPEAKERS, CBN_SELCHANGE, OnSelectionChange)
    COMMAND_HANDLER_EX(IDC_AUTO_MATRIX, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_NORMALIZE_MATRIX, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_VOICE_CONTROL, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_EXPAND_STEREO, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_AUTO_GAIN, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_NORMALIZE, BN_CLICKED, OnButtonClick)
    COMMAND_HANDLER_EX(IDC_DRC, BN_CLICKED, OnButtonClick)
    END_MSG_MAP()
private:
    BOOL OnInitDialog(CWindow, LPARAM);
    void OnSelectionChange(UINT, int, CWindow);
    void OnButtonClick(UINT, int, CWindow);
    bool HasChanged();
    void OnChanged();

    const preferences_page_callback::ptr m_callback;
};

#endif