/*
 * Copyright 2018 Deutsches Zentrum für Luft- und Raumfahrt e.V.
 *         (German Aerospace Center), Earth Observation Center
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "util.h"

#include "utils/memutils.h"
#include "utils/guc.h" // for GetConfigOption*

#define H3_INDEX_STR_LEN 17


/*
 * Convert an H3Index to a text*
 */
text *
__h3_index_to_text(H3Index index)
{
    char *outstr = (char *) palloc0(H3_INDEX_STR_LEN * sizeof(char));
    SET_VARSIZE(outstr, H3_INDEX_STR_LEN * sizeof(char));

    H3_EXPORT(h3ToString)(index, outstr, H3_INDEX_STR_LEN);

    text *iout = cstring_to_text(outstr);
    pfree(outstr);
    return iout;
}

/**
 * convert an cstring to an h3index
 */
bool
__h3_index_from_cstring(const char *cstr, H3Index *index)
{
    (*index) = stringToH3(cstr);
    if ((*index) == 0) {
        fail_and_report("Could not convert the value '%s' to a H3 index", cstr);
        return false;
    }
    return true;
}

/*
 * Make the smallest bounding box for the given polygon.
 *
 * This funciton is taken from postgresql
 */
void
__h3_make_bound_box(POLYGON *poly)
{
    int         i;
    double      x1,
                y1,
                x2,
                y2;

    Assert(poly->npts > 0);

    x1 = x2 = poly->p[0].x;
    y2 = y1 = poly->p[0].y;
    for (i = 1; i < poly->npts; i++)
    {
        if (poly->p[i].x < x1)
            x1 = poly->p[i].x;
        if (poly->p[i].x > x2)
            x2 = poly->p[i].x;
        if (poly->p[i].y < y1)
            y1 = poly->p[i].y;
        if (poly->p[i].y > y2)
            y2 = poly->p[i].y;
    }

    poly->boundbox.low.x = x1;
    poly->boundbox.high.x = x2;
    poly->boundbox.low.y = y1;
    poly->boundbox.high.y = y2;
}

static const char *
human_byte_size(size_t size)
{
    static char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    static char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;

    double size_dbl = size;

	if (size > 1024) {
		for (i = 0; (size / 1024) > 0 && i<length-1; i++, size /= 1024) {
			size_dbl = size / 1024.0;
        }
	}

	static char output[200];
	snprintf(output, 200, "%.02lf%s", size_dbl, suffix[i]);
    return output;
}

/**
 * wrapper for the postgresql allocators, as polyfill
 * may require large amounts of memory.
 *
 */
void *
__h3_polyfill_palloc0(size_t size)
{
    static size_t max_polyfill_mem = 0;

#if PG_VERSION_NUM > 100000 // GetConfigOptionByName(const char *name, const char **varname, bool missing_ok)
                            // exists with PG 9.6+, GUC_UNIT_MB with PG 10
    if (max_polyfill_mem <= 0) {
        const char * max_polyfill_mem_str = GetConfigOptionByName(PGH3_POLYFILL_MEM_SETTING_NAME, NULL, true);
        if (max_polyfill_mem_str != NULL) {

            int max_polyfill_mem_mb = 0;

            if (!parse_int(max_polyfill_mem_str, &max_polyfill_mem_mb, GUC_UNIT_MB, NULL)) {

                fail_and_report_with_code(
                        ERRCODE_CONFIG_FILE_ERROR,
                        PGH3_POLYFILL_MEM_SETTING_NAME ": could not parse value \"%s\"",
                        max_polyfill_mem_str);

            }
            max_polyfill_mem = (size_t)max_polyfill_mem_mb * 1024 * 1024;
        }
    }
#endif

    // not set, so set max. possible allocation using the standard allocator
    if (max_polyfill_mem == 0) {
        max_polyfill_mem = MaxAllocSize;
    }

    report_debug1(PGH3_POLYFILL_MEM_SETTING_NAME ": using %s of the possible %ldMB.",
            human_byte_size(size),
            max_polyfill_mem / 1024 /1024);

    if (size > max_polyfill_mem) {

        fail_and_report_with_code(
                ERRCODE_CONFIGURATION_LIMIT_EXCEEDED,
                PGH3_POLYFILL_MEM_SETTING_NAME ": requested memory allocation (%s) exceeds the upper limit (%ldMB).",
                human_byte_size(size),
                max_polyfill_mem / 1024 /1024);

    }

    int flags = 0;
    if (!AllocSizeIsValid(size)) {
        // the huge allocator is required
        flags |= MCXT_ALLOC_HUGE;
    }
    flags |= MCXT_ALLOC_ZERO;

    return palloc_extended(size, flags);
}
