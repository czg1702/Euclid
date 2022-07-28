./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( parent([Goods].[electronic product].computer) ) " \
"select " \
"children( parent( parent( [starting date].[2021].Q1.M3 ) ) ) on 0, " \
"QQQ on 111 " \
"from [logistics.test] " \
"where ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.income " \
")"