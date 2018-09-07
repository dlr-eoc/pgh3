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

#define H3_INDEX_STR_LEN 17


/*
 * Convert an H3Index to a text*
 */
text *
__h3_index_to_text(H3Index index) 
{
    char *outstr = (char *) palloc0(H3_INDEX_STR_LEN * sizeof(char));
    SET_VARSIZE(outstr, H3_INDEX_STR_LEN * sizeof(char));

    h3ToString(index, outstr, H3_INDEX_STR_LEN);

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

