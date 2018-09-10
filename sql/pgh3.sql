

CREATE FUNCTION h3_ext_version() RETURNS text
AS 'pgh3', 'h3_ext_version'
IMMUTABLE LANGUAGE C STRICT;
comment on function h3_ext_version() is 'Returns the version number of the H3 extension. This is not the version number of the h3 library itself.';


/******* Indexing functions *********************************/

CREATE FUNCTION h3_geo_to_h3index(p point, resolution integer) RETURNS text
AS 'pgh3', 'h3_geo_to_h3index'
IMMUTABLE LANGUAGE C STRICT;
comment on function h3_geo_to_h3index(p point, integer) is 'Get the H3 index for the point at the given resolution.';

/*
comment on function byteamagic_mime(bytea) is
    'return the mimetype of the bytea data as text';

*/

create function h3_geo_to_h3index(g geometry, resolution integer) returns text as $$
begin
    if st_geometrytype(g) != 'ST_Point' then
        raise exception 'h3 only supports point geometries';
    end if;
    return h3_geo_to_h3index(g::point, resolution);
end;
$$ language plpgsql immutable strict;
comment on function h3_geo_to_h3index(g geometry, integer) is 'Get the H3 index for the PostGIS point geometry at the given resolution.';


create function _h3_h3index_to_geo(h3index text) returns point
as 'pgh3', '_h3_h3index_to_geo'
IMMUTABLE LANGUAGE C STRICT ;
comment on function _h3_h3index_to_geo(h3index text) is 'Convert a H3 index to coordinates. Returned as a postgresql point type.';

create function h3_h3index_to_geo(h3index text) returns geometry
as $$
declare
    g geometry;
begin
    select st_makepoint(p[0], p[1]) into g
    from (
        select _h3_h3index_to_geo(h3index) p
    ) foo;

    return g;
end;
$$ language plpgsql immutable strict;
comment on function h3_h3index_to_geo(h3index text) is 'Convert a H3 index to coordinates. Returned as a PostGIS point geometrry.';



create function _h3_h3index_to_geoboundary(h3index text) returns polygon
as 'pgh3', '_h3_h3index_to_geoboundary'
IMMUTABLE LANGUAGE C STRICT ;
comment on function _h3_h3index_to_geoboundary(h3index text) is 'Convert the boundary of H3 index to polygon coordinates. Returned as a postgresql native polygon type.';

create function h3_h3index_to_geoboundary(h3index text) returns geometry
as $$
declare
    g geometry;
    point_geoms geometry[];
begin
    select _h3_h3index_to_geoboundary(h3index)::geometry into g;

    return g;
end;
$$ language plpgsql immutable strict;
comment on function h3_h3index_to_geoboundary(h3index text) is 'Convert the boundary of H3 index to polygon coordinates. Returned as a PostGIS polygon geometrry.';


create function h3_h3index_is_valid(h3index text) returns boolean
as 'pgh3', 'h3_h3index_is_valid'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_h3index_is_valid(h3index text) is 'Check if a H3 index is valid.';

create function h3_get_resolution(h3index text) returns integer
as 'pgh3', 'h3_get_resolution'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_get_resolution(h3index text) is 'Get the resolution for a H3 index.';

create function h3_get_basecell(h3index text) returns integer
as 'pgh3', 'h3_get_basecell'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_get_basecell(h3index text) is 'Get the base cell for a H3 index.';


/******* hierarchy functions *********************************/


create function h3_to_parent(h3index text, resolution integer) returns text
as 'pgh3', 'h3_to_parent'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_to_parent(h3index text, resolution integer) is 'Returns the parent (coarser) index containing the given index.';

create function h3_to_children(h3index text, resolution integer) returns setof text
as 'pgh3', 'h3_to_children'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_to_children(h3index text, resolution integer) is 'Returns the children (finer) indeesx contained the given index.';

/******* neigbor functions *********************************/

create function h3_kring(h3index text, distance integer) returns setof text
as 'pgh3', 'h3_kring'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_kring(h3index text, distance integer) is 'Returns the neigbor indices within the given distance.';

/******* misc functions *********************************/

create function h3_hexagon_area_km2(resolution integer) returns double precision
as 'pgh3', 'h3_hexagon_area_km2'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_hexagon_area_km2(resolution integer) is 'Average hexagon area in square kilometers at the given resolution.';

create function h3_hexagon_area_m2(resolution integer) returns double precision
as 'pgh3', 'h3_hexagon_area_m2'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_hexagon_area_m2(resolution integer) is 'Average hexagon area in square meters at the given resolution.';

create function h3_edge_length_km(resolution integer) returns double precision
as 'pgh3', 'h3_edge_length_km'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_edge_length_km(resolution integer) is 'Average hexagon edge length in kilometers at the given resolution.';

create function h3_edge_length_m(resolution integer) returns double precision
as 'pgh3', 'h3_edge_length_m'
IMMUTABLE LANGUAGE C STRICT ;
comment on function h3_edge_length_m(resolution integer) is 'Average hexagon edge length in meters at the given resolution.';

/******* region functions *********************************/

CREATE FUNCTION _h3_polyfill_polygon_c(exterior_ring polygon, interior_rings polygon[],  
                            resolution integer) RETURNS SETOF text
AS 'pgh3', '_h3_polyfill_polygon'
IMMUTABLE LANGUAGE C;
comment on function _h3_polyfill_polygon_c(exterior_ring polygon, interior_rings polygon[], resolution integer) is
    'Fills the given exterior ring with hexagons at the given resolution. The interior_ring polygons are understood as holes and will be omitted.';


create function _h3_polyfill_polygon(polygong geometry, resolution integer) returns setof text as $$
declare
    exterior_ring polygon;
    interior_rings polygon[];
begin

    if st_geometrytype(polygong) != 'ST_Polygon' then
        raise exception 'h3_polyfill_polygon only supports polygon geometries';
    end if;

    select st_makepolygon(st_exteriorring(polygong))::polygon into exterior_ring;

    select array_agg(ring) into interior_rings
    from (
        select i as ring_i,
            st_makepolygon(st_interiorringn(polygong, i))::polygon ring
        from (
            select generate_series(0, st_numinteriorrings(polygong)) i
        ) gs
        where gs.i > 0 /* index starts with 1 */
    ) rings;

    return query select _h3_polyfill_polygon_c(exterior_ring, interior_rings, resolution);

    return;
end;
$$ language plpgsql immutable strict;
comment on function _h3_polyfill_polygon(polygong geometry, resolution integer) is 
    'Fills the given PostGIS polygon with hexagons at the given resolution. Holes in the polygon will be omitted.';


create function h3_polyfill(geom geometry, resolution integer) returns setof text as $$
begin

    if st_geometrytype(geom) = 'ST_Polygon' then
        return query select _h3_polyfill_polygon(geom, resolution);
    elsif st_geometrytype(geom) = 'ST_MultiPolygon' then
        return query select distinct _h3_polyfill_polygon(g, resolution)
            from (
                select (st_dump(geom)).geom as g
            ) d;
    else
        raise exception 'h3_polyfill only supports polygon and multipolygon geometries';
    end if;

    return;
end;
$$ language plpgsql immutable strict;
comment on function h3_polyfill(polygong geometry, resolution integer) is 
    'Fills the given PostGIS polygon or multipolygon with hexagons at the given resolution. Holes in the polygon will be omitted.

The H3 `polyfill` function requires a preallocation of the memory for the generates indexes. Depending of the size of the
given polygon, its shape and the resolution this may exhaust the memory given to PostgreSQL in its configuration. In this case
this function will be terminated by the database server and a corresponding notice will be given.

There are essentially two ways to work around this issue:

* Increase PostgreSQLs memory
* Cut the polygon into segments and run this function to each of them seperately. The PostGIS functions `ST_Subdivide`, `ST_Split` and `ST_Segmentize` may be helpful.
';


CREATE FUNCTION _h3_polyfill_polygon_estimate_c(exterior_ring polygon, interior_rings polygon[],  
            resolution integer) RETURNS integer
AS 'pgh3', '_h3_polyfill_polygon_estimate'
IMMUTABLE LANGUAGE C;
comment on function _h3_polyfill_polygon_estimate_c(exterior_ring polygon, interior_rings polygon[], resolution integer) is
    'Estimate the number of indexes required to fill the given exterior ring with hexagons at the given resolution. The interior_ring polygons are understood as holes and will be omitted.';


create function _h3_polyfill_polygon_estimate(polygong geometry, resolution integer) returns integer as $$
declare
    exterior_ring polygon;
    interior_rings polygon[];
    result integer;
begin

    if st_geometrytype(polygong) != 'ST_Polygon' then
        raise exception 'h3_polyfill_polygon only supports polygon geometries';
    end if;

    select st_makepolygon(st_exteriorring(polygong))::polygon into exterior_ring;

    select array_agg(ring) into interior_rings
    from (
        select i as ring_i,
            st_makepolygon(st_interiorringn(polygong, i))::polygon ring
        from (
            select generate_series(0, st_numinteriorrings(polygong)) i
        ) gs
        where gs.i > 0 /* index starts with 1 */
    ) rings;

    select _h3_polyfill_polygon_estimate_c(exterior_ring, interior_rings, resolution)
            into result;

    return result;
end;
$$ language plpgsql immutable strict;
comment on function _h3_polyfill_polygon_estimate(polygong geometry, resolution integer) is 
    'Estimate the number of indexes required to fill the given PostGIS polygon with hexagons at the given resolution. Holes in the polygon will be omitted.';


create function h3_polyfill_estimate(geom geometry, resolution integer) returns integer as $$
declare
    result integer;
begin

    if st_geometrytype(geom) = 'ST_Polygon' then
        select _h3_polyfill_polygon_estimate(geom, resolution) into result;
    elsif st_geometrytype(geom) = 'ST_MultiPolygon' then
        select sum(u) into result
            from (
            select distinct _h3_polyfill_polygon_estimate(g, resolution) u
                from (
                    select (st_dump(geom)).geom as g
                ) d
            ) f;
    else
        raise exception 'h3_polyfill only supports polygon and multipolygon geometries';
    end if;

    return result;
end;
$$ language plpgsql immutable strict;
comment on function h3_polyfill_estimate(polygong geometry, resolution integer) is 
    'Estimate the number of indexes required to fill the given PostGIS polygon or multipolygon with hexagons at the given resolution. Holes in the polygon will be omitted.';
