./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( [Goods].[electronic product] ) " \
"select " \
"children([starting date].[2021]) on 0, " \
"QQQ on 111 " \
"from [logistics.test] " \
"where ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.cost " \
")"