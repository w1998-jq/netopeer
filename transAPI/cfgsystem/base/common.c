/**
 * \file common.c
 * \brief Internal functions for cfgsystem module
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \date 2014
 *
 * Copyright (C) 2014 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */
#define _GNU_SOURCE

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <augeas.h>

#include "common.h"

augeas *sysaugeas = NULL;

int augeas_init(char** msg)
{
	assert(msg);

	if (sysaugeas != NULL) {
		/* already initiated */
		return EXIT_SUCCESS;
	}

	sysaugeas = aug_init(NULL, NULL, AUG_NO_MODL_AUTOLOAD | AUG_NO_ERR_CLOSE | AUG_SAVE_NEWFILE);
	if (aug_error(sysaugeas) != AUG_NOERROR) {
		asprintf(msg, "Augeas NTP initialization failed (%s)", aug_error_message(sysaugeas));
		return EXIT_FAILURE;
	}
	/* NTP */
	aug_set(sysaugeas, "/augeas/load/Ntp/lens", "Ntp.lns");
	aug_set(sysaugeas, "/augeas/load/Ntp/incl", AUGEAS_NTP_CONF);
	/* DNS resolver */
	aug_set(sysaugeas, "/augeas/load/Resolv/lens", "Resolv.lns");
	aug_set(sysaugeas, "/augeas/load/Resolv/incl", AUGEAS_DNS_CONF);
	/* authentication */
	aug_set(sysaugeas, "/augeas/load/Pam/lens", "Pam.lns");
	aug_set(sysaugeas, "/augeas/load/Pam/incl", AUGEAS_PAM_DIR"/*");
	/* /etc/login.defs */
	aug_set(sysaugeas, "/augeas/load/Login_defs/lens", "Login_defs.lns");
	aug_set(sysaugeas, "/augeas/load/Login_defs/incl", "/etc/login.defs");

	aug_load(sysaugeas);

	if (aug_match(sysaugeas, "/augeas//error", NULL) != 0) {
		aug_get(sysaugeas, "/augeas//error[1]/message", (const char**) msg);
		asprintf(msg, "Initiating augeas failed (%s)", *msg);
		augeas_close();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int augeas_save(char** msg)
{
	if (aug_save(sysaugeas) != 0) {
		asprintf(msg, "Saving configuration failed (%s)", aug_error_message(sysaugeas));
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

void augeas_close(void)
{
	aug_close(sysaugeas);
	sysaugeas = NULL;
}