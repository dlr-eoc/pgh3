/*
 * Copyright 2018 Deutsches Zentrum für Luft- und Raumfahrt e.V. 
 *         (German Aerospace Center), German Remote Sensing Data Center
 *         Department: Geo-Risks and Civil Security
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

#include "postgres.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/geo_decls.h"
#include "funcapi.h"

#include <h3/h3api.h>


PG_FUNCTION_INFO_V1(h3_to_parent);

/*
 * Return the parent (coarser) index containing the index in the parameter.
 *
 * The Index is returned in its string representation
 */
Datum
h3_to_parent(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    int resolution = PG_GETARG_INT32(1);

    H3Index parent = H3_EXPORT(h3ToParent)(index, resolution);

    text * parent_text = __h3_index_to_text(parent);
    PG_RETURN_TEXT_P(parent_text);
}


PG_FUNCTION_INFO_V1(h3_to_children);

/*
 * Return the child (finer) index contained the index in the given 
 * resolution.
 *
 * The indexex are returned in thier string representations
 */
Datum
h3_to_children(PG_FUNCTION_ARGS)
{
    FuncCallContext *funcctx;
    int call_cntr = 0;
    int max_calls = 0;
    MemoryContext oldcontext;
    H3Index *hexagons = NULL;

    if (SRF_IS_FIRSTCALL()) {

        text *parent_index_text = PG_GETARG_TEXT_P(0);
        char * parent_index_cstr = text_to_cstring(parent_index_text);
        H3Index parent_index;
        __h3_index_from_cstring(parent_index_cstr, &parent_index);

        int child_resolution = PG_GETARG_INT32(1);

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        int num_children = H3_EXPORT(maxH3ToChildrenSize)(parent_index, child_resolution);
        max_calls = num_children;

        report_debug1("Generating %d H3 child hexagons at resolution %d", 
                        num_children, child_resolution);

        hexagons = palloc0(num_children * sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(parent_index, child_resolution, hexagons);

        if (max_calls > 0) {
            // keep track of the results
            funcctx->max_calls = max_calls;
            funcctx->user_fctx = hexagons;
        }
        else {
            // fast track when no results
            pfree(hexagons);
            hexagons = NULL;

            MemoryContextSwitchTo(oldcontext);
            SRF_RETURN_DONE(funcctx);
        }
        MemoryContextSwitchTo(oldcontext);
    }

    // stuff done on every call of the function
    funcctx = SRF_PERCALL_SETUP();

    // Initialize per-call variables
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    hexagons = funcctx->user_fctx;

    if (call_cntr < max_calls) {
        text * index_text = __h3_index_to_text(hexagons[call_cntr]);
        SRF_RETURN_NEXT(funcctx, PointerGetDatum(index_text));
    }
    else {
        pfree(hexagons);
        hexagons = NULL;

        SRF_RETURN_DONE(funcctx);
    }
}
