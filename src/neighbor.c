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

#include "postgres.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/geo_decls.h"
#include "funcapi.h"

#include <h3/h3api.h>


PG_FUNCTION_INFO_V1(h3_kring);

/*
 * Returns the neigbor indices within the given distance.
 *
 * The indexex are returned in thier string representations
 */
Datum
h3_kring(PG_FUNCTION_ARGS)
{
    FuncCallContext *funcctx;
    int call_cntr = 0;
    int max_calls = 0;
    MemoryContext oldcontext;
    H3Index *hexagons = NULL;

    if (SRF_IS_FIRSTCALL()) {

        text *center_index_text = PG_GETARG_TEXT_P(0);
        char * center_index_cstr = text_to_cstring(center_index_text);
        H3Index center_index;
        __h3_index_from_cstring(center_index_cstr, &center_index);

        int distance = PG_GETARG_INT32(1);

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        int max_num_neighbors = maxKringSize(distance);

        report_debug1("Generating %d H3 child hexagons within distance %d", 
                        max_num_neighbors, distance);

        hexagons = palloc0(max_num_neighbors * sizeof(H3Index));
        kRing(center_index, distance, hexagons);

        for (int i = 0; i < max_num_neighbors; i++) {
            if (hexagons[i] != 0) {
                if (i > max_calls) {
                    // fill NULL "holes" in list of hexagons with the hexagons located
                    // after the NULL value. This allows us to use call_cntr and max_calls
                    // for iteration during SRF calls.
                    hexagons[max_calls] = hexagons[i];
                    hexagons[i] = 0;
                }
                max_calls++;
            }
        }

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
