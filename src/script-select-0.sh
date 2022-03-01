./euclid-cli \
"select " \
"{ ([starting region].[Asia].[China], Transport.railway), ([starting region].[Asia].[China], Transport.highway) } on 0, " \
"{ ([starting date].[2019].[Q4].[M10], Goods.[foods].[nut]), ([starting date].[2019].[Q3].[M9], Goods.[foods].[wine]) } on 1 " \
"from [logistics.test]"