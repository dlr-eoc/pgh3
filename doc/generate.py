#!/usr/bin/python
# Pulls documentation from the database schema

import sys
import psycopg2
import psycopg2.extras

HEADER='''
# PostgreSQL H3 extension

Postgresql extension to wrap the hexagonal hierarchical geospatial indexing system of [h3 library](https://github.com/uber/h3).

'''

def function_defs(cur, ffilter='h3\\_%'):
    cur.execute('''
            select proc.oid, 
                    proc.proname as name, 
                    pg_get_function_identity_arguments(proc.oid) as args, 
                    pg_get_function_result(proc.oid) as result, 
                    dsc.description as desc
            from pg_proc proc
            left join pg_description dsc on dsc.objoid = proc.oid
            where proname like %s escape '\\'
            order by proc.proname''', [ffilter])
    return cur.fetchall()


def print_function_def(fnc):
    sys.stdout.write("### {0}".format(fnc['name']))
    sys.stdout.write('\n\n{0}'.format(fnc['desc'] or ''))
    sys.stdout.write("\n\n__Synopsis:__ `{0}({1})`".format(fnc['name'], fnc['args']))
    sys.stdout.write("\n\n__Returntype:__ `{0}`".format(fnc['result']))

    sys.stdout.write('\n\n\n')


if __name__ == '__main__':
    dbname = sys.argv[1]
    conn = psycopg2.connect(dbname=dbname)
    cur = conn.cursor(cursor_factory = psycopg2.extras.DictCursor)
    
    sys.stdout.write(HEADER)
    sys.stdout.write('''
## Functions\n\n''')
    for fnc in function_defs(cur):
        print_function_def(fnc)
        
    sys.stdout.write('''
## Internal functions

These are mostly function for which a more comfortable wrapper function exists.

''')
    for fnc in function_defs(cur, ffilter='_h3_%'):
        print_function_def(fnc)

