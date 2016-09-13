#!/bin/sh

# Get all the function names from op1.h and spit them out as a JSON dict

functions=(`grep 'int EM' include/op1.h | cut -d ' ' -f3 | cut -d '(' -f1`)

/bin/echo -n "["
/bin/echo -n "\"_"
/bin/echo -n "${functions[0]}"
/bin/echo -n "\""

for i in "${functions[@]:1}"
do
  /bin/echo -n ", "
  /bin/echo -n "\"_"
  /bin/echo -n $i
  /bin/echo -n "\""
done

/bin/echo "]"
