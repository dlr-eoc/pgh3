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
#include "utils/array.h"
#include "utils/builtins.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/lsyscache.h"
#include "access/tupmacs.h"
#include "funcapi.h"

#include <h3/h3api.h>


static void
__h3_free_geopolygon_internal_structs(GeoPolygon *h3polygon) {
    if (h3polygon->numHoles > 0) {

        for (int hi = 0; hi < h3polygon->numHoles; hi++) {
            pfree(h3polygon->holes[hi].verts);
            h3polygon->holes[hi].verts = NULL;
        }
        pfree(h3polygon->holes);
        h3polygon->holes = NULL;
    }
    pfree(h3polygon->geofence.verts);
    h3polygon->geofence.verts = NULL;
}

static void
__h3_pgpolygon_to_geofence(POLYGON *poly, Geofence *geofence)
{

    // initialize the Geofence structure
    geofence->verts = (GeoCoord*) palloc0(
                (sizeof(geofence->verts[0]) * poly->npts));

    geofence->numVerts = poly->npts;

    for (int i = 0; i < poly->npts; i++) {
        geofence->verts[i].lat = degsToRads(poly->p[i].y);
        geofence->verts[i].lon = degsToRads(poly->p[i].x);
    }
}

static void
__h3_polyfill_build_geopolygon(GeoPolygon *h3polygon, POLYGON *exterior_ring, ArrayType *interior_rings) 
{
    // initialize
    h3polygon->numHoles = 0; 
    h3polygon->holes = NULL;
    h3polygon->geofence.verts = NULL;

    // fill the geofence of the h3 struct with the values of the exterior ring.
    __h3_pgpolygon_to_geofence(exterior_ring, &(h3polygon->geofence));

    if (interior_rings != NULL) {
        int ndim = ARR_NDIM(interior_rings);
        int *dim = ARR_DIMS(interior_rings);

        if (ndim > 0) { // not an empty array
            if (ndim != 1) {
                fail_and_report("The interior rings argument must be Null, "
                                "an empty array or a 1-dimensional array");
            }

            Oid elemtype = ARR_ELEMTYPE(interior_rings);
            if (elemtype != POLYGONOID) {
                fail_and_report("the type of the interior_rings array must be "
                                "postgresqls polygon type");
            }

            int16       typlen;
            bool        typbyval;
            char        typalign;
            get_typlenbyvalalign(elemtype, &typlen, &typbyval, &typalign);

            h3polygon->numHoles = dim[0];
            h3polygon->holes = (Geofence *) palloc0(h3polygon->numHoles * (sizeof(Geofence)));

            bool isnull;
            Datum interior_rings_datum = PointerGetDatum(interior_rings);
            for (int i = 1; i <= dim[0]; i++) {

                Datum ring_datum = array_get_element(interior_rings_datum, ndim, &i, 
                            -1, typlen, typbyval, typalign, &isnull);
                if (isnull) {
                    fail_and_report("lat coordinate at position %d is null", i);
                }
                
                POLYGON *ring = DatumGetPolygonP(ring_datum);
                __h3_pgpolygon_to_geofence(ring, &(h3polygon->holes[i - 1]));
            }
        }
    }
}

PG_FUNCTION_INFO_V1(_h3_polyfill_polygon);

Datum
_h3_polyfill_polygon(PG_FUNCTION_ARGS)
{
    FuncCallContext *funcctx;
    int call_cntr = 0;
    int max_calls = 0;
    MemoryContext oldcontext;
    H3Index *hexagons = NULL;

    if (SRF_IS_FIRSTCALL()) {
        // early exit when exterior_ring is null
        if (PG_ARGISNULL(0)) {
            PG_RETURN_NULL();
        }

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        POLYGON *exterior_ring = PG_GETARG_POLYGON_P(0);
        ArrayType *interior_rings = NULL;
        if (!(PG_ARGISNULL(1))) {
            interior_rings = PG_GETARG_ARRAYTYPE_P(1);
        }
        int resolution = PG_GETARG_INT32(2);

        GeoPolygon h3polygon;
        __h3_polyfill_build_geopolygon(&h3polygon, exterior_ring, interior_rings);
        
        int numHexagons = maxPolyfillSize(&h3polygon, resolution);
        report_debug1("Generating an estimated number of %d H3 "
                        "hexagons at resolution %d", numHexagons, resolution);

        hexagons = palloc0(numHexagons * sizeof(H3Index));
        polyfill(&h3polygon, resolution, hexagons);
        __h3_free_geopolygon_internal_structs(&h3polygon);

        for (int i = 0; i < numHexagons; i++) {
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
        report_debug1("Generated exactly %d H3 hexagons at resolution %d", 
                    max_calls, resolution);

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


PG_FUNCTION_INFO_V1(_h3_polyfill_polygon_estimate);

Datum
_h3_polyfill_polygon_estimate(PG_FUNCTION_ARGS)
{
    // early exit when exterior_ring is null
    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    POLYGON *exterior_ring = PG_GETARG_POLYGON_P(0);
    ArrayType *interior_rings = NULL;
    if (!(PG_ARGISNULL(1))) {
        interior_rings = PG_GETARG_ARRAYTYPE_P(1);
    }
    int resolution = PG_GETARG_INT32(2);

    GeoPolygon h3polygon;
    __h3_polyfill_build_geopolygon(&h3polygon, exterior_ring, interior_rings);
    
    int numHexagons = maxPolyfillSize(&h3polygon, resolution);

    __h3_free_geopolygon_internal_structs(&h3polygon);

    PG_RETURN_INT32(numHexagons);
}
