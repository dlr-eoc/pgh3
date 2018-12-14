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

#include <h3/h3api.h>


#define H3_RES_TO_FLOAT8_FUNC(FUNC_NAME, H3_FUNC_NAME) \
    Datum \
    FUNC_NAME(PG_FUNCTION_ARGS) \
    { \
        int resolution = PG_GETARG_INT32(0); \
        double value = H3_EXPORT(H3_FUNC_NAME)(resolution); \
        PG_RETURN_FLOAT8(value); \
    }


PG_FUNCTION_INFO_V1(h3_hexagon_area_km2);
H3_RES_TO_FLOAT8_FUNC(h3_hexagon_area_km2, hexAreaKm2);

PG_FUNCTION_INFO_V1(h3_hexagon_area_m2);
H3_RES_TO_FLOAT8_FUNC(h3_hexagon_area_m2, hexAreaM2);

PG_FUNCTION_INFO_V1(h3_edge_length_km);
H3_RES_TO_FLOAT8_FUNC(h3_edge_length_km, edgeLengthKm);

PG_FUNCTION_INFO_V1(h3_edge_length_m);
H3_RES_TO_FLOAT8_FUNC(h3_edge_length_m, edgeLengthM);
