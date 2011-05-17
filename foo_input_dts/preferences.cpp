#include "preferences.h"

cfg_int cfg_format(guid_cfg_format, default_cfg_format);
cfg_int cfg_speakers(guid_cfg_speakers, default_cfg_speakers);
cfg_int cfg_auto_matrix(guid_cfg_auto_matrix, default_cfg_auto_matrix);
cfg_int cfg_normalize_matrix(guid_cfg_normalize_matrix, default_cfg_normalize_matrix);
cfg_int cfg_voice_control(guid_cfg_voice_control, default_cfg_voice_control);
cfg_int cfg_expand_stereo(guid_cfg_expand_stereo, default_cfg_expand_stereo);
cfg_int cfg_auto_gain(guid_cfg_auto_gain, default_cfg_auto_gain);
cfg_int cfg_normalize(guid_cfg_normalize, default_cfg_normalize);
cfg_int cfg_drc(guid_cfg_drc, default_cfg_drc);

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) 
{
	CWindow w;

	pfc::ptr_list_t<char> items;

	items.add_item((char *)"PCM 16 LE");
	items.add_item((char *)"PCM 24 LE");
	items.add_item((char *)"PCM 32 LE");
	items.add_item((char *)"PCM 16 BE");
	items.add_item((char *)"PCM 24 BE");
	items.add_item((char *)"PCM 32 BE");
	items.add_item((char *)"PCM Float");

	w = GetDlgItem(IDC_FORMAT);

	uSendMessage(w, CB_RESETCONTENT, 0, 0);

	t_size i;
	for (i = 0; i < items.get_count(); i++) 
	{
		uSendMessageText(w, CB_ADDSTRING, 0, items[i]);
	}

	::SendMessage(w, CB_SETCURSEL, cfg_format, 0);
	
	items.remove_all();
	items.add_item((char *)"1/0 (mono)");
	items.add_item((char *)"2/0 (stereo)");
	items.add_item((char *)"3/0 (surround)");
	items.add_item((char *)"2/2 (quadro)");
	items.add_item((char *)"3/2 (5 ch)");
	items.add_item((char *)"3/2+SW (5.1)");

	w = GetDlgItem(IDC_SPEAKERS);

	uSendMessage(w, CB_RESETCONTENT, 0, 0);

	for (i = 0; i < items.get_count(); i++) 
	{
		uSendMessageText(w, CB_ADDSTRING, 0, items[i]);
	}

	::SendMessage(w, CB_SETCURSEL, cfg_speakers - 1, 0);

	CheckDlgButton(IDC_AUTO_MATRIX, cfg_auto_matrix);
	CheckDlgButton(IDC_NORMALIZE_MATRIX, cfg_normalize_matrix);
	CheckDlgButton(IDC_VOICE_CONTROL, cfg_voice_control);
	CheckDlgButton(IDC_EXPAND_STEREO, cfg_expand_stereo);
	CheckDlgButton(IDC_AUTO_GAIN, cfg_auto_gain);
	CheckDlgButton(IDC_NORMALIZE, cfg_normalize);
	CheckDlgButton(IDC_DRC, cfg_drc);

	return FALSE;
}

void CMyPreferences::OnSelectionChange(UINT, int, CWindow) 
{
	OnChanged();
}

void CMyPreferences::OnButtonClick(UINT, int, CWindow) 
{
	OnChanged();
}

t_uint32 CMyPreferences::get_state() 
{
	t_uint32 state = preferences_state::resettable;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() 
{
	::SendMessage(GetDlgItem(IDC_FORMAT), CB_SETCURSEL, default_cfg_format, 0);
	::SendMessage(GetDlgItem(IDC_SPEAKERS), CB_SETCURSEL, default_cfg_speakers - 1, 0);

	CheckDlgButton(IDC_AUTO_MATRIX, default_cfg_auto_matrix);
	CheckDlgButton(IDC_NORMALIZE_MATRIX, default_cfg_normalize_matrix);
	CheckDlgButton(IDC_VOICE_CONTROL, default_cfg_voice_control);
	CheckDlgButton(IDC_EXPAND_STEREO, default_cfg_expand_stereo);
	CheckDlgButton(IDC_AUTO_GAIN, default_cfg_auto_gain);
	CheckDlgButton(IDC_NORMALIZE, default_cfg_normalize);
	CheckDlgButton(IDC_DRC, default_cfg_drc);

	OnChanged();
}

void CMyPreferences::apply() 
{
	cfg_format = ::SendMessage(GetDlgItem(IDC_FORMAT), CB_GETCURSEL, 0, 0);
	cfg_speakers = ::SendMessage(GetDlgItem(IDC_SPEAKERS), CB_GETCURSEL, 0, 0) + 1;

	cfg_auto_matrix = IsDlgButtonChecked(IDC_AUTO_MATRIX);
	cfg_normalize_matrix = IsDlgButtonChecked(IDC_NORMALIZE_MATRIX);
	cfg_voice_control = IsDlgButtonChecked(IDC_VOICE_CONTROL);
	cfg_expand_stereo = IsDlgButtonChecked(IDC_EXPAND_STEREO);
	cfg_auto_gain = IsDlgButtonChecked(IDC_AUTO_GAIN);
	cfg_normalize = IsDlgButtonChecked(IDC_NORMALIZE);
	cfg_drc = IsDlgButtonChecked(IDC_DRC);

	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged() 
{
	return (::SendMessage(GetDlgItem(IDC_FORMAT), CB_GETCURSEL, 0, 0) != cfg_format) ||
		   (::SendMessage(GetDlgItem(IDC_SPEAKERS), CB_GETCURSEL, 0, 0) + 1 != cfg_speakers) ||
		   (IsDlgButtonChecked(IDC_AUTO_MATRIX) != cfg_auto_matrix) ||
		   (IsDlgButtonChecked(IDC_NORMALIZE_MATRIX) != cfg_normalize_matrix) ||
		   (IsDlgButtonChecked(IDC_VOICE_CONTROL) != cfg_voice_control) ||
		   (IsDlgButtonChecked(IDC_EXPAND_STEREO) != cfg_expand_stereo) ||
		   (IsDlgButtonChecked(IDC_AUTO_GAIN) != cfg_auto_gain) ||
		   (IsDlgButtonChecked(IDC_NORMALIZE) != cfg_normalize) ||
		   (IsDlgButtonChecked(IDC_DRC) != cfg_drc);
}

void CMyPreferences::OnChanged() 
{
	//tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}
