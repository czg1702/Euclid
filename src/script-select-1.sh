./euclid-cli \
"select	 " \
"{	 " \
"  ([Goods].[foods],[Transport].[railway],[starting region].[Europe].[UK]),	 " \
"  ([Transport].[highway],[starting region].[Asia].[Japan])  	 " \
"} on 0,	 " \
"{	 " \
"  ([starting region].[America].[Chile],[ending region].[Asia].[China]),	 " \
"  ([ending region].[Asia].[South Korea]),	 " \
"  ([starting date].[2019].[Q3].[M8],[completion date].[2020].[Q2],[Goods].[foods])	 " \
"} on 1,	 " \
"{	 " \
"  ([completion date].[2020].[Q4],[Goods].[foods],[Transport].[ocean freight]),	 " \
"  ([Goods].[foods],[Transport].[railway]),	 " \
"  ([Transport].[highway]),	 " \
"  ([starting region].[Europe].[Greece])	 " \
"} on 2	 " \
"from [logistics.test]"