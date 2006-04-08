/*==========================================================
 * editmap.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.2 - 09 Nov 1994
 *========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "feedback.h"
#include "gedcomi.h"
#include "zstr.h"


extern STRING map_keys[];
extern STRING qScmperr,qSaredit,qSronlye,qSdataerr,qSbadttnum;
extern STRING qSsepch;

/*==============================================
 * edit_mapping -- Edit character mapping record
 *  code: [in] which translation table (see defn of map_keys)
 *============================================*/
BOOLEAN
edit_mapping (INT trnum)
{
	BOOLEAN rtn=FALSE;
	ZSTR zstr=zs_new();
	if (trnum < 0 || trnum >= NUM_TT_MAPS) {
		msg_error(_(qSbadttnum));
		goto end_edit_mapping;
	}
	if (readonly) {
		msg_error(_(qSronlye));
		goto end_edit_mapping;
	}

	unlink(editfile);

	if (transl_get_legacy_tt(trnum)) {
		if (!save_tt_to_file(trnum, editfile)) {
			msg_error(_(qSdataerr));
			goto end_edit_mapping;
		}
	}
	do_edit();
	while (TRUE) {
		if (load_new_tt(editfile, trnum)) {
			rtn = TRUE;
			goto end_edit_mapping;
		}
		zs_apps(zstr,_(qScmperr));
		zs_appc(zstr, ' ');
		zs_appf(zstr, _(qSsepch), "<tab>"); /* (separator is %s) */
		if (ask_yes_or_no_msg(zs_str(zstr), _(qSaredit)))
			do_edit();
		else {
			goto end_edit_mapping;
		}
	}
end_edit_mapping:
	zs_free(&zstr);
	return rtn;
}
/*==============================================
 * save_tt_to_file -- Save one translation table
 *  to external file
 * returns TRUE if successful
 * Created: 2001/12/24, Perry Rapp
 *============================================*/
BOOLEAN
save_tt_to_file (INT ttnum, STRING filename)
{
	INT rtn;
	TRANSLFNC translfnc = NULL; /* Must not translate the translation table! */
	ASSERT(ttnum>=0);
	ASSERT(ttnum<NUM_TT_MAPS);
	rtn = retrieve_to_textfile(map_keys[ttnum], filename, translfnc);
	return (rtn != RECORD_ERROR);
}
/*==============================================
 * load_new_tt -- Load translation table from file
 *  also update permanent record in database
 *  filepath:  [IN]  file whence to get new table
 *  ttnum:     [IN]  which translation table to load
 * Created: 2001/12/26, Perry Rapp
 *============================================*/
BOOLEAN
load_new_tt (CNSTRING filepath, INT trnum)
{
	TRANSLFNC transfnc = NULL; /* don't translate translation tables ! */
	TRANTABLE tt=0;
	CNSTRING mapname = transl_get_map_name(trnum);
	ZSTR zerr=zs_new();
	if (!init_map_from_file(filepath, mapname, &tt, zerr)) {
		llwprintf(zs_str(zerr));
		zs_free(&zerr);
		if (tt)
			remove_trantable(tt);
		return FALSE;
	}
	zs_free(&zerr);
	/* change from old one to new one */
	transl_set_legacy_tt(trnum, tt);
	/* store new one in permanent record in database */
	store_text_file_to_db(map_keys[trnum], filepath, transfnc);
	return TRUE;
}
