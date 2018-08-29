create extension if not exists postgis;
create extension if not exists pgh3;


/* test data */

create table test_geometries(name text primary key, geom geometry);
insert into test_geometries (name, geom) values
        ('polygon with hole', 'POLYGON((51.0708433295253 -23.5514097581317,61.0708433295253 11.4485902418683,31.0708433295253 6.44859024186827,26.0708433295253 -13.5514097581317,51.0708433295253 -23.5514097581317),(36.0708433295253 -3.55140975813173,51.0708433295253 1.44859024186827,46.0708433295253 -13.5514097581317,36.0708433295253 -3.55140975813173))'::geometry),
        ('multipolygon with hole', 'MULTIPOLYGON(((1.42997600913934 32.3311063409985,-18.5700239908607 37.3311063409985,6.42997600913934 22.3311063409985,1.42997600913934 32.3311063409985)),((-18.5700239908607 27.3311063409985,-28.5700239908607 22.3311063409985,-28.5700239908607 2.33110634099847,-8.57002399086066 -2.66889365900153,6.42997600913934 12.3311063409985,-18.5700239908607 27.3311063409985),(-8.57002399086066 12.3311063409985,-18.5700239908607 7.33110634099847,-18.5700239908607 17.3311063409985,-8.57002399086066 12.3311063409985)))');


/* region */

select h3_polyfill_estimate(geom, 1) 
    from test_geometries where name = 'polygon with hole';

select array_length(array_agg(i), 1)
    from ( select h3_polyfill(geom, 1) i
        from test_geometries 
        where name = 'polygon with hole' ) f;

select i 
    from (
    select h3_polyfill(geom, 1) i
        from test_geometries where name = 'polygon with hole'
) f
order by i;

select i 
    from (
    select h3_polyfill(geom, 1) i
        from test_geometries where name = 'multipolygon with hole'
) f
order by i;
