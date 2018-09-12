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

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


PG_FUNCTION_INFO_V1(h3_ext_version);

/*
 * return the version of this extension (not h3 itself)
 * as text.
 */
Datum
h3_ext_version(PG_FUNCTION_ARGS)
{
#ifdef EXTVERSION
    /* EXTVERSION is only available on some compilers. see Makefile */
    PG_RETURN_TEXT_P(cstring_to_text(EXTVERSION));
#else
    PG_RETURN_TEXT_P(cstring_to_text("unknown"));
#endif
}

