./euclid-cli \
"select " \
"{ " \
"  (  [starting region].[Asia].[China],  Transport.railway  ), " \
"  (  [starting region].[Asia].[China],  Transport.highway  ), " \
"  ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China] )  " \
"} on 0, " \
"{ " \
"  (  [starting date].[2019].[Q4].[M10],  Goods.[foods].[nut]   ), " \
"  (  [starting date].[2019].[Q3].[M9],   Goods.[foods].[wine]  ), " \
"  ( [ending region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )  " \
"} on 1 " \
"from [logistics.test] "