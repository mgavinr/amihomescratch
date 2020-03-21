/* WeekLS.rexx */
parse arg z
call date('n', date('i') - 7)
parse var result d m y .
datewithquotes = '22'x||m d '00:00' y||'22'x
'ls -N' datewithquotes z
