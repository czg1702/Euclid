./euclid-cli \
"select	 " \
"{	 " \
"  ([Goods].[foods].[nut],[Transport].[railway],[starting region].[Europe].[UK]),	 " \
"  ([Transport].[highway],[starting region].[Asia].[Japan])  	 " \
"} on 0,	 " \
"{	 " \
"  ([starting region].[America].[Chile],[ending region].[Asia].[China]),	 " \
"  ([ending region].[Asia].[South Korea]),	 " \
"  ([starting date].[2019].[Q3].[M8],[completion date].[2020].[Q2],[Goods].[foods].[wine])	 " \
"} on 1,	 " \
"{	 " \
"  ([completion date].[2020].[Q4],[Goods].[foods].[beef],[Transport].[ocean freight]),	 " \
"  ([Goods].[foods].[nut],[Transport].[railway]),	 " \
"  ([Transport].[highway]),	 " \
"  ([starting region].[Europe].[Greece])	 " \
"} on 2	 " \
"from [logistics.test]"