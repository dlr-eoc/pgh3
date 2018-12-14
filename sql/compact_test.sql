create extension if not exists postgis;
create extension if not exists pgh3;

/*
Indexes are taken from the testCompact.c "sunnyvale" unittests

generated with:

    echo '89283470c27ffff' | kRing 9 | sed -E "s/^(.*)$/'\1',/g" | sed 'N;s/\n/ /;' | sed 'N;s/\n/ /;'
*/
create table sunnyvale (h3index text);
insert into sunnyvale (h3index) values
('89283470c27ffff'), ('89283470c23ffff'), ('89283470c2fffff'), ('89283470d5bffff'), ('89283470d53ffff'), ('89283470dcbffff'), ('89283470c37ffff'), ('89283470c33ffff'),
('89283470c3bffff'), ('89283470c2bffff'), ('89283470893ffff'), ('89283470897ffff'), ('89283470d4bffff'), ('89283470d43ffff'), ('89283470d57ffff'), ('89283470dcfffff'),
('89283470dc3ffff'), ('89283470ddbffff'), ('89283470cafffff'), ('89283470cabffff'), ('89283470c07ffff'), ('89283470c0fffff'), ('89283470c77ffff'), ('89283470c67ffff'),
('8928347089bffff'), ('89283470883ffff'), ('89283470887ffff'), ('892834708b3ffff'), ('89283470d4fffff'), ('89283470d47ffff'), ('89283470d0bffff'), ('89283470d1bffff'),
('89283470dc7ffff'), ('89283470dd7ffff'), ('89283470dd3ffff'), ('89283470ca7ffff'), ('89283470ca3ffff'), ('89283470cbbffff'), ('89283470c17ffff'), ('89283470c03ffff'),
('89283470c0bffff'), ('89283470c73ffff'), ('89283470c63ffff'), ('89283470c6fffff'), ('892834708d7ffff'), ('8928347088bffff'), ('8928347088fffff'), ('892834708bbffff'),
('892834708a3ffff'), ('892834708b7ffff'), ('89283470d7bffff'), ('89283470d73ffff'), ('89283470d0fffff'), ('89283470d03ffff'), ('89283470d13ffff'), ('89283470d8bffff'),
('89283470d9bffff'), ('89283472b6fffff'), ('89283472b6bffff'), ('89283470cb7ffff'), ('89283470cb3ffff'), ('89283470c87ffff'), ('89283470c8fffff'), ('89283470c13ffff'),
('89283470c1bffff'), ('89283470c57ffff'), ('89283470c47ffff'), ('89283470c7bffff'), ('89283470c6bffff'), ('892834708d3ffff'), ('892834708c3ffff'), ('892834708c7ffff'),
('89283470813ffff'), ('89283470817ffff'), ('892834708abffff'), ('892834708afffff'), ('892834708a7ffff'), ('89283470d6bffff'), ('89283470d63ffff'), ('89283470d77ffff'),
('89283470d3bffff'), ('89283470d07ffff'), ('89283470d17ffff'), ('89283470d8fffff'), ('89283470d83ffff'), ('89283470d93ffff'), ('89283472b67ffff'), ('89283472b63ffff'),
('89283472b7bffff'), ('89283472b4fffff'), ('89283472b4bffff'), ('89283470c97ffff'), ('89283470c83ffff'), ('89283470c8bffff'), ('89283470cc7ffff'), ('89283470ccfffff'),
('89283470c53ffff'), ('89283470c43ffff'), ('89283470c4fffff'), ('89283470eb7ffff'), ('89283470ea7ffff'), ('892834708dbffff'), ('892834708cbffff'), ('892834708cfffff'),
('8928347081bffff'), ('89283470803ffff'), ('89283470807ffff'), ('89283470833ffff'), ('89283470837ffff'), ('892834709dbffff'), ('892834709d3ffff'), ('89283470d6fffff'),
('89283470d67ffff'), ('89283470d2bffff'), ('89283470d23ffff'), ('89283470d33ffff'), ('89283470dabffff'), ('89283470dbbffff'), ('89283470d87ffff'), ('89283470d97ffff'),
('89283472b2fffff'), ('89283472b2bffff'), ('89283472b77ffff'), ('89283472b73ffff'), ('89283472b47ffff'), ('89283472b43ffff'), ('89283472b5bffff'), ('89283472a2fffff'),
('89283470c93ffff'), ('89283470c9bffff'), ('89283470cd7ffff'), ('89283470cc3ffff'), ('89283470ccbffff'), ('89283470127ffff'), ('89283470c5bffff'), ('89283470c4bffff'),
('89283470eb3ffff'), ('89283470ea3ffff'), ('89283470eafffff'), ('89283470e37ffff'), ('89283470e27ffff'), ('89283470853ffff'), ('89283470857ffff'), ('8928347080bffff'),
('8928347080fffff'), ('8928347083bffff'), ('89283470823ffff'), ('89283470827ffff'), ('892834709cbffff'), ('892834709c3ffff'), ('892834709d7ffff'), ('8928347099bffff'),
('89283470993ffff'), ('89283470d2fffff'), ('89283470d27ffff'), ('89283470d37ffff'), ('89283470dafffff'), ('89283470da3ffff'), ('89283470db3ffff'), ('8928347764bffff'),
('8928347765bffff'), ('89283472b27ffff'), ('89283472b23ffff'), ('89283472b3bffff'), ('89283472b0fffff'), ('89283472b0bffff'), ('89283472b57ffff'), ('89283472b53ffff'),
('89283472a27ffff'), ('89283472a23ffff'), ('89283472a2bffff'), ('89283472a67ffff'), ('89283472a6fffff'), ('89283470cd3ffff'), ('89283470cdbffff'), ('89283470137ffff'),
('89283470123ffff'), ('8928347012fffff'), ('89283470e97ffff'), ('89283470e87ffff'), ('89283470ebbffff'), ('89283470eabffff'), ('89283470e33ffff'), ('89283470e23ffff'),
('89283470e2fffff'), ('8928347085bffff'), ('89283470843ffff'), ('89283470847ffff'), ('89283470873ffff'), ('89283470877ffff'), ('8928347082bffff'), ('8928347082fffff'),
('8928347095bffff'), ('89283470953ffff'), ('892834709cfffff'), ('892834709c7ffff'), ('8928347098bffff'), ('89283470983ffff'), ('89283470997ffff'), ('8928347725bffff'),
('89283477253ffff'), ('892834772cbffff'), ('892834772dbffff'), ('89283470da7ffff'), ('89283470db7ffff'), ('8928347764fffff'), ('89283477643ffff'), ('89283477653ffff'),
('892834776cbffff'), ('89283472b37ffff'), ('89283472b33ffff'), ('89283472b07ffff'), ('89283472b03ffff'), ('89283472b1bffff'), ('89283472bcfffff'), ('89283472bcbffff'),
('89283472a37ffff'), ('89283472a33ffff'), ('89283472a3bffff'), ('89283472a77ffff'), ('89283472a63ffff'), ('89283472a6bffff'), ('892834701a7ffff'), ('892834701afffff'),
('89283470133ffff'), ('8928347013bffff'), ('8928347012bffff'), ('89283470e93ffff'), ('89283470e83ffff'), ('89283470e8fffff'), ('89283470e17ffff'), ('89283470e07ffff'),
('89283470e3bffff'), ('89283470e2bffff'), ('89283470a93ffff'), ('89283470a97ffff'), ('8928347084bffff'), ('8928347084fffff'), ('8928347087bffff'), ('89283470863ffff'),
('89283470867ffff'), ('89283470b93ffff'), ('89283470b97ffff'), ('8928347094bffff'), ('89283470943ffff'), ('89283470957ffff'), ('8928347091bffff'), ('89283470913ffff'),
('8928347098fffff'), ('89283470987ffff'), ('8928347724bffff'), ('89283477243ffff'), ('89283477257ffff'), ('892834772cfffff'), ('892834772c3ffff'), ('892834772d3ffff'),
('8928347766bffff'), ('8928347767bffff'), ('89283477647ffff'), ('89283477657ffff'), ('892834776cfffff'), ('892834776c3ffff'), ('892834776dbffff'), ('89283472bafffff'),
('89283472babffff'), ('89283472b17ffff'), ('89283472b13ffff'), ('89283472bc7ffff'), ('89283472bc3ffff'), ('89283472bdbffff'), ('89283472aafffff');


select count(*) from h3_compact(array(select h3index from sunnyvale));

select h3_compact(array((select h3_to_children('89283470c27ffff', 10))));
