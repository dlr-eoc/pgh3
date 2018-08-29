create extension if not exists postgis;
create extension if not exists pgh3;


/* hierarchy */

select h3_to_parent('85639c63fffffff', 4);

select h3_to_parent('85639c63fffffff', 1);

select h3_to_children('82639ffffffffff', 3);
