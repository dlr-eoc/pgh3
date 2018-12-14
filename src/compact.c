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
#include "utils/lsyscache.h"
#include "funcapi.h"

#include <h3/h3api.h>

/**
 * returns an allocated array of H3Indexes
 */
static H3Index *
pg_array_to_h3index_array(ArrayType *indexarray, int * num_indexes) 
{
    *num_indexes = 0;
    if (indexarray != NULL) {
        int ndim = ARR_NDIM(indexarray);

        if (ndim != 1) {
            fail_and_report("The array if h3indexes must be an 1-dimensional array");
        }

        Oid elemtype = ARR_ELEMTYPE(indexarray);
        if (elemtype != TEXTOID) {
            fail_and_report("The type of the h3index array must be text");
        }

        int16       typlen;
        bool        typbyval;
        char        typalign;
        get_typlenbyvalalign(elemtype, &typlen, &typbyval, &typalign);

        int *dim = ARR_DIMS(indexarray);
        H3Index * h3indexes = palloc0(dim[0] * sizeof(H3Index));

        bool isnull;
        Datum indexarray_datum = PointerGetDatum(indexarray);
        for (int i = 1; i <= dim[0]; i++) {

            Datum index_datum = array_get_element(indexarray_datum, ndim, &i,
                        -1, typlen, typbyval, typalign, &isnull);
            if (isnull) {
                fail_and_report("h3 index at array position %d is null", i);
            }

            text *index_text = DatumGetTextP(index_datum);
            char * index_cstr = text_to_cstring(index_text);

            if (__h3_index_from_cstring(index_cstr, &(h3indexes[i-1])) == false) {
                pfree(h3indexes);
                fail_and_report("could not parse the h3 index at array position %d", i);
                return NULL;
            }
        }
        
        (*num_indexes) = dim[0];

        return h3indexes;
    }

    return NULL;
}


PG_FUNCTION_INFO_V1(h3_compact);

Datum
h3_compact(PG_FUNCTION_ARGS)
{
    FuncCallContext *funcctx;
    int call_cntr = 0;
    int max_calls = 0;
    MemoryContext oldcontext;
    H3Index *compacted_indexes = NULL;

    if (SRF_IS_FIRSTCALL()) {
        // early exit when no idexes are given
        if (PG_ARGISNULL(0)) {
            PG_RETURN_NULL();
        }

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        ArrayType * uncompacted_indexes_array = PG_GETARG_ARRAYTYPE_P(0);

        // convert the indexes to their native format
        int num_uncompacted_indexes = 0;
        H3Index * uncompacted_indexes = pg_array_to_h3index_array(uncompacted_indexes_array, &num_uncompacted_indexes);

        if (num_uncompacted_indexes == 0) {
            PG_RETURN_NULL(); // early exit - nothing to do
        }

        // allocate memory for the results
        compacted_indexes = __h3_polyfill_palloc0(num_uncompacted_indexes * sizeof(H3Index));

        if (H3_EXPORT(compact)(uncompacted_indexes, compacted_indexes, num_uncompacted_indexes) != 0) {
            pfree(uncompacted_indexes);
            pfree(compacted_indexes);
            fail_and_report("Error while compacting the h3 indexes");
        }
        pfree(uncompacted_indexes);

        for (int i = 0; i < num_uncompacted_indexes; i++) {
            if (compacted_indexes[i] != 0) {
                if (i > max_calls) {
                    // fill NULL "holes" in list of hexagons with the hexagons located
                    // after the NULL value. This allows us to use call_cntr and max_calls
                    // for iteration during SRF calls.
                    compacted_indexes[max_calls] = compacted_indexes[i];
                    compacted_indexes[i] = 0;
                }
                max_calls++;
            }
        }

        report_debug1("Compacted %d H3 hexagons to %d",
                    num_uncompacted_indexes, max_calls);

        if (max_calls > 0) {
            // keep track of the results
            funcctx->max_calls = max_calls;
            funcctx->user_fctx = compacted_indexes;
        }
        else {
            // fast track when no results
            pfree(compacted_indexes);
            compacted_indexes = NULL;

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
    compacted_indexes = funcctx->user_fctx;

    if (call_cntr < max_calls) {
        text * index_text = __h3_index_to_text(compacted_indexes[call_cntr]);
        SRF_RETURN_NEXT(funcctx, PointerGetDatum(index_text));
    }
    else {
        pfree(compacted_indexes);
        compacted_indexes = NULL;

        SRF_RETURN_DONE(funcctx);
    }

}
