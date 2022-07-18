./euclid-cli \
"select " \
"{ " \
"  (Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[Europe].[UK]) " \
"} on 0, " \
"{ " \
"  ([ending region].[Europe].[Italy], [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3], [measure].[cost]) " \
"} on 1 " \
"from [logistics.test] "