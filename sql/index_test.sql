create extension if not exists postgis;
create extension if not exists pgh3;


/* indexing */

select trunc(p[0]::numeric, 5) x, trunc(p[1]::numeric, 5) y
from (
    select h3_h3index_to_geo('85639c63fffffff')::point p
) f;

select trunc(st_x(p)::numeric, 5) x, trunc(st_y(p)::numeric, 5) y
from (
    select h3_h3index_to_geo('85639c63fffffff')::geometry p
) f;

select h3_geo_to_h3index('(9.40691761982618,52.1233617183044)'::point, 5);

select h3_h3index_is_valid('85639c63fffffff'); -- is valid

select h3_h3index_is_valid('85639cdddddddd'); -- is invalid

select h3_get_resolution('85639c63fffffff'); -- = 5

select h3_get_basecell('85639c63fffffff'); -- = 5
