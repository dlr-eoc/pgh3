
# PostgreSQL H3 extension

Postgresql extension to wrap the hexagonal hierarchical geospatial indexing system of [h3 library](https://github.com/uber/h3).


## Functions

### h3_compact

Compacts the array of given H3 indexes as best as possible

__Synopsis:__ `h3_compact(h3indexes text[])`

__Returntype:__ `SETOF text`


### h3_edge_length_km

Average hexagon edge length in kilometers at the given resolution.

__Synopsis:__ `h3_edge_length_km(resolution integer)`

__Returntype:__ `double precision`


### h3_edge_length_m

Average hexagon edge length in meters at the given resolution.

__Synopsis:__ `h3_edge_length_m(resolution integer)`

__Returntype:__ `double precision`


### h3_ext_version

Returns the version number of the H3 extension. This is not the version number of the h3 library itself.

__Synopsis:__ `h3_ext_version()`

__Returntype:__ `text`


### h3_geo_to_h3index

Get the H3 index for the point at the given resolution.

__Synopsis:__ `h3_geo_to_h3index(p point, resolution integer)`

__Returntype:__ `text`


### h3_geo_to_h3index

Get the H3 index for the PostGIS point geometry at the given resolution.

__Synopsis:__ `h3_geo_to_h3index(g geometry, resolution integer)`

__Returntype:__ `text`


### h3_get_basecell

Get the base cell for a H3 index.

__Synopsis:__ `h3_get_basecell(h3index text)`

__Returntype:__ `integer`


### h3_get_resolution

Get the resolution for a H3 index.

__Synopsis:__ `h3_get_resolution(h3index text)`

__Returntype:__ `integer`


### h3_h3index_is_valid

Check if a H3 index is valid.

__Synopsis:__ `h3_h3index_is_valid(h3index text)`

__Returntype:__ `boolean`


### h3_h3index_to_geo

Convert a H3 index to coordinates. Returned as a PostGIS point geometry.

__Synopsis:__ `h3_h3index_to_geo(h3index text)`

__Returntype:__ `geometry`


### h3_h3index_to_geoboundary

Convert the boundary of H3 index to polygon coordinates. Returned as a PostGIS polygon geometry.

__Synopsis:__ `h3_h3index_to_geoboundary(h3index text)`

__Returntype:__ `geometry`


### h3_hexagon_area_km2

Average hexagon area in square kilometers at the given resolution.

__Synopsis:__ `h3_hexagon_area_km2(resolution integer)`

__Returntype:__ `double precision`


### h3_hexagon_area_m2

Average hexagon area in square meters at the given resolution.

__Synopsis:__ `h3_hexagon_area_m2(resolution integer)`

__Returntype:__ `double precision`


### h3_kring

Returns the neighbor indices within the given distance.

__Synopsis:__ `h3_kring(h3index text, distance integer)`

__Returntype:__ `SETOF text`


### h3_polyfill

Fills the given PostGIS polygon or multipolygon with hexagons at the given resolution. Holes in the polygon will be omitted.

The H3 `polyfill` function requires a preallocation of the memory for the generates indexes. Depending on the size of the
given polygon, its shape and resolution this may exhaust the memory given to this extension. In this case
this function will be terminated by the database server and a corresponding notice will be given.

The memory limit can be increased using the `pgh3.polyfill_mem` configuration parameter in the `postgresql.conf` file. The 
default value for this setting 1024MB (PostgreSQL internal `MaxAllocSize`). Syntax for the setting is

    pgh3.polyfill_mem = 1024MB

For values larger than `MaxAllocSize`, the PostgreSQL `MemoryContextAllocHuge` allocator will be used.

_This setting is only available when using a PostgreSQL version >= 10_. On earlier versions the memory limit is set to 1024MB.

If this does not resolve the issue, there is essentially one way to work around this issue: Cut the polygon into segments and run this function to each of them seperately. The PostGIS functions `ST_Subdivide`, `ST_Split` and `ST_Segmentize` may be helpful.


__Synopsis:__ `h3_polyfill(geom geometry, resolution integer)`

__Returntype:__ `SETOF text`


### h3_polyfill_estimate

Estimate the number of indexes required to fill the given PostGIS polygon or multipolygon with hexagons at the given resolution. Holes in the polygon will be omitted.

__Synopsis:__ `h3_polyfill_estimate(geom geometry, resolution integer)`

__Returntype:__ `integer`


### h3_to_children

Returns the children (finer) indexes contained the given index.

__Synopsis:__ `h3_to_children(h3index text, resolution integer)`

__Returntype:__ `SETOF text`


### h3_to_parent

Returns the parent (coarser) index containing the given index.

__Synopsis:__ `h3_to_parent(h3index text, resolution integer)`

__Returntype:__ `text`


### h3_uncompact

Uncompacts the array of given H3 indexes

__Synopsis:__ `h3_uncompact(h3indexes text[], resolution integer)`

__Returntype:__ `SETOF text`



## Internal functions

These are mostly function for which a more comfortable wrapper function exists.

### _h3_h3index_to_geo

Convert a H3 index to coordinates. Returned as a postgresql point type.

__Synopsis:__ `_h3_h3index_to_geo(h3index text)`

__Returntype:__ `point`


### _h3_h3index_to_geoboundary

Convert the boundary of H3 index to polygon coordinates. Returned as a postgresql native polygon type.

__Synopsis:__ `_h3_h3index_to_geoboundary(h3index text)`

__Returntype:__ `polygon`


### _h3_polyfill_polygon_c

Fills the given exterior ring with hexagons at the given resolution. The interior_ring polygons are understood as holes and will be omitted.

__Synopsis:__ `_h3_polyfill_polygon_c(exterior_ring polygon, interior_rings polygon[], resolution integer)`

__Returntype:__ `SETOF text`


### _h3_polyfill_polygon_estimate_c

Estimate the number of indexes required to fill the given exterior ring with hexagons at the given resolution. The interior_ring polygons are understood as holes and will be omitted.

__Synopsis:__ `_h3_polyfill_polygon_estimate_c(exterior_ring polygon, interior_rings polygon[], resolution integer)`

__Returntype:__ `integer`


