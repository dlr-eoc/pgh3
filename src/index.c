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

#include <h3/h3api.h>


PG_FUNCTION_INFO_V1(h3_geo_to_h3index);

/*
 * Find the H3 index for a coordinate pair.
 *
 * The Index is returned in its string representation
 */
Datum
h3_geo_to_h3index(PG_FUNCTION_ARGS)
{
    Point *p = PG_GETARG_POINT_P(0);

    GeoCoord location;
    location.lat = degsToRads(p->y);
    location.lon = degsToRads(p->x);

    int resolution = PG_GETARG_INT32(1);

    H3Index index = H3_EXPORT(geoToH3)(&location, resolution);
    if (index == 0) {
        fail_and_report("Could not convert the coordinates (%f %f) to a H3 index", p->x, p->y);
    }

    text * index_text = __h3_index_to_text(index);
    PG_RETURN_TEXT_P(index_text);
}



PG_FUNCTION_INFO_V1(_h3_h3index_to_geo);

/*
 * Return the centroid coordinates for the given h3 index.
 *
 * The index is expected in its string serialized representation.
 */
Datum
_h3_h3index_to_geo(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    GeoCoord coord;
    H3_EXPORT(h3ToGeo)(index, &coord);

    // return as a postgresql native point
    Point *p = palloc0(sizeof(Point));
    p->y = radsToDegs(coord.lat);
    p->x = radsToDegs(coord.lon);

    PG_RETURN_POINT_P(p);
}



PG_FUNCTION_INFO_V1(_h3_h3index_to_geoboundary);

/*
 * Return the boundary coordinates for the given h3 index as a
 * native postgresql polygon
 *
 * The index is expected in its string serialized representation.
 */
Datum
_h3_h3index_to_geoboundary(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    GeoBoundary gp;
    H3_EXPORT(h3ToGeoBoundary)(index, &gp);

    // return as an polygon
    //
    // initialize the POLYGON structure
    POLYGON *poly = NULL;
    int size = offsetof(POLYGON, p) + (sizeof(poly->p[0]) * gp.numVerts);
    poly = (POLYGON*) palloc0(size);
    SET_VARSIZE(poly, size);

    poly->npts = gp.numVerts;

    for(int i = 0; i < gp.numVerts; i++) {
        poly->p[i].y = radsToDegs(gp.verts[i].lat);
        poly->p[i].x = radsToDegs(gp.verts[i].lon);
    }

    __h3_make_bound_box(poly);

    PG_RETURN_POLYGON_P(poly);
}


PG_FUNCTION_INFO_V1(h3_h3index_is_valid);

/*
 * Return True when an H3 index is valid.
 *
 * The index is expected in its string serialized representation.
 */
Datum
h3_h3index_is_valid(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    int isvalid = H3_EXPORT(h3IsValid)(index);

    PG_RETURN_BOOL(isvalid != 0);
}


PG_FUNCTION_INFO_V1(h3_get_resolution);

/*
 * Get the H3 resolution.
 *
 * The index is expected in its string serialized representation.
 */
Datum
h3_get_resolution(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    int res = H3_EXPORT(h3GetResolution)(index); 

    PG_RETURN_INT32(res);
}


PG_FUNCTION_INFO_V1(h3_get_basecell);

/*
 * Get the H3 basecell.
 *
 * The index is expected in its string serialized representation.
 */
Datum
h3_get_basecell(PG_FUNCTION_ARGS)
{
    text *index_text = PG_GETARG_TEXT_P(0);
    
    char * index_cstr = text_to_cstring(index_text);
    H3Index index;
    __h3_index_from_cstring(index_cstr, &index);

    int basecell = H3_EXPORT(h3GetBaseCell)(index); 

    PG_RETURN_INT32(basecell);
}
