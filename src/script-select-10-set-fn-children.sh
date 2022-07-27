./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( [completion date].[2020].[Q1] ) " \
"select " \
"children([starting date].[2021].[Q2]) on 0, " \
"QQQ on 111 " \
"from [logistics.test] " \
"where ( " \
"Goods.[household appliances].[television], " \
"Transport.[ocean freight], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.quantity " \
")"