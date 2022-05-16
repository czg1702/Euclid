./euclid-cli \
"select	 " \
"{ " \
"    ( " \
"        Goods.[foods].[nut], " \
"        Transport.[highway], " \
"        [starting region].[Europe].[UK], " \
"        [ending region].[Asia].[South Korea], " \
"        [starting date].[2019].[Q3].[M8], " \
"        [completion date].[2020].[Q4] " \
"    ), " \
"    ( " \
"        [ending region].[Asia].[South Korea], " \
"        [starting date].[2019].[Q3].[M8], " \
"        [completion date].[2020].[Q4], " \
"        Goods.[foods].[nut], " \
"        Transport.[highway], " \
"        [starting region].[Europe].[UK] " \
"    ) " \
"} on 0 " \
"from [logistics.test] "