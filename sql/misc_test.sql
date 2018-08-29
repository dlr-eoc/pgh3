create extension if not exists postgis;
create extension if not exists pgh3;

/******* misc functions *********************************/

select h3_hexagon_area_km2(4);

select h3_hexagon_area_m2(4);

select h3_edge_length_km(4);

select h3_edge_length_m(4);
