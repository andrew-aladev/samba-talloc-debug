/*
   Unix SMB/CIFS implementation.
   SMB torture tester
   Copyright (C) Andrew Bartlett 2012

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"
#include "torture/smbtorture.h"
#include "dlz_minimal.h"
#include <talloc_debug.h>
#include <ldb.h>
#include "lib/param/param.h"
#include "dsdb/samdb/samdb.h"
#include "dsdb/common/util.h"
#include "auth/session.h"

struct torture_context *tctx_static;

static void dlz_bind9_log_wrapper(int level, const char *fmt, ...)
{
	va_list ap;
	char *msg;
	va_start(ap, fmt);
	msg = talloc_vasprintf(NULL, fmt, ap);
	torture_comment(tctx_static, "%s\n", msg);
	TALLOC_FREE(msg);
	va_end(ap);
}

static bool test_dlz_bind9_version(struct torture_context *tctx)
{
	unsigned int flags = 0;
	torture_assert_int_equal(tctx, dlz_version(&flags),
				 DLZ_DLOPEN_VERSION, "got wrong DLZ version");
	return true;
}

static bool test_dlz_bind9_create(struct torture_context *tctx)
{
	void *dbdata;
	const char *argv[] = {
		"samba_dlz",
		"-H",
		lpcfg_private_path(tctx, tctx->lp_ctx, "dns/sam.ldb"),
		NULL
	};
	tctx_static = tctx;
	torture_assert_int_equal(tctx, dlz_create("samba_dlz", 3, discard_const_p(char *, argv), &dbdata,
						  "log", dlz_bind9_log_wrapper, NULL), ISC_R_SUCCESS,
		"Failed to create samba_dlz");

	dlz_destroy(dbdata);

	return true;
}

static isc_result_t dlz_bind9_writeable_zone_hook(dns_view_t *view,
					   const char *zone_name)
{
	struct torture_context *tctx = talloc_get_type((void *)view, struct torture_context);
	struct ldb_context *samdb = samdb_connect_url(tctx, NULL, tctx->lp_ctx,
						      system_session(tctx->lp_ctx),
						      0, lpcfg_private_path(tctx, tctx->lp_ctx, "dns/sam.ldb"));
	struct ldb_message *msg;
	int ret;
	const char *attrs[] = {
		NULL
	};
	if (!samdb) {
		torture_fail(tctx, "Failed to connect to samdb");
		return ISC_R_FAILURE;
	}

	ret = dsdb_search_one(samdb, tctx, &msg, NULL,
			      LDB_SCOPE_SUBTREE, attrs, DSDB_SEARCH_SEARCH_ALL_PARTITIONS,
			      "(&(objectClass=dnsZone)(name=%s))", zone_name);
	if (ret != LDB_SUCCESS) {
		torture_fail(tctx, talloc_asprintf(tctx, "Failed to search for %s: %s", zone_name, ldb_errstring(samdb)));
		return ISC_R_FAILURE;
	}
	talloc_free(msg);

	return ISC_R_SUCCESS;
}

static bool test_dlz_bind9_configure(struct torture_context *tctx)
{
	void *dbdata;
	const char *argv[] = {
		"samba_dlz",
		"-H",
		lpcfg_private_path(tctx, tctx->lp_ctx, "dns/sam.ldb"),
		NULL
	};
	tctx_static = tctx;
	torture_assert_int_equal(tctx, dlz_create("samba_dlz", 3, discard_const_p(char *, argv), &dbdata,
						  "log", dlz_bind9_log_wrapper,
						  "writeable_zone", dlz_bind9_writeable_zone_hook, NULL),
				 ISC_R_SUCCESS,
				 "Failed to create samba_dlz");

	torture_assert_int_equal(tctx, dlz_configure((void*)tctx, dbdata),
						     ISC_R_SUCCESS,
				 "Failed to configure samba_dlz");

	dlz_destroy(dbdata);

	return true;
}



static struct torture_suite *dlz_bind9_suite(TALLOC_CTX *ctx)
{
	struct torture_suite *suite = torture_suite_create(ctx, "dlz_bind9");

	suite->description = talloc_strdup(suite,
	                                   "Tests for the BIND 9 DLZ module");
	torture_suite_add_simple_test(suite, "version", test_dlz_bind9_version);
	torture_suite_add_simple_test(suite, "create", test_dlz_bind9_create);
	torture_suite_add_simple_test(suite, "configure", test_dlz_bind9_configure);
	return suite;
}

/**
 * DNS torture module initialization
 */
NTSTATUS torture_bind_dns_init(void)
{
	struct torture_suite *suite;
	TALLOC_CTX *mem_ctx = talloc_autofree_context();

	/* register DNS related test cases */
	suite = dlz_bind9_suite(mem_ctx);
	if (!suite) return NT_STATUS_NO_MEMORY;
	torture_register_suite(suite);

	return NT_STATUS_OK;
}
