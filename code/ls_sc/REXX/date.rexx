/* ljr */
date = date('n')
parse var date d m y .
say left(date('w'),3) m right(' '||d+0,2) time() y
