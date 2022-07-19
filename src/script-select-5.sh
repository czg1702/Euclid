./euclid-cli \
"select " \
"{ " \
"  ([ending region].[Europe].[Italy], [completion date].[2020].[Q1].[M3], measure.cost ) " \
"} on 0, " \
"{ " \
"  (Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[Europe].[UK] ) " \
"} on 1 " \
"from [logistics.test] where ( [starting date].[2021].[Q2].[M5] )"