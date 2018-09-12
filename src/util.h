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


#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/geo_decls.h"

#include <h3/h3api.h>

#define ARRNELEMS(x)  ArrayGetNItems(ARR_NDIM(x), ARR_DIMS(x))

#define fail_and_report(msg, ...) \
             ereport(ERROR, \
                (errcode(ERRCODE_EXTERNAL_ROUTINE_EXCEPTION), errmsg(msg, ##__VA_ARGS__)));

#define report_notice(msg, ...) \
            ereport(NOTICE, \
                (errmsg(msg, ##__VA_ARGS__)));

#define report_debug1(msg, ...) \
            ereport(DEBUG1, \
                (errmsg(msg, ##__VA_ARGS__)));


text * __h3_index_to_text(H3Index);
void __h3_make_bound_box(POLYGON *poly);
bool __h3_index_from_cstring(const char *str, H3Index *index);
