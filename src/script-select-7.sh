./euclid-cli \
"with member [measure].[MM001] as (measure.cost) + (measure.cost) * (measure.cost) - (measure.cost) " \
"select " \
"{ " \
"  ( measure.cost, [ending region].[Asia].[China] ), " \
"  ( measure.cost, [ending region].[Asia].[Japan] ), " \
"  ( measure.cost, [ending region].[Asia].[South Korea] ), " \
"  ( measure.cost, [ending region].[America].[U.S] ), " \
"  ( measure.cost, [ending region].[America].[Mexico] ), " \
"  ( measure.cost, [ending region].[America].[Chile] ), " \
"  ( measure.cost, [ending region].[Europe].[Greece] ), " \
"  ( measure.cost, [ending region].[Europe].[Italy] ), " \
"  ( measure.cost, [ending region].[Europe].[UK] ), " \
"  ( measure.cost, [ending region].[Asia] ), " \
"  ( measure.cost, [ending region].[America] ), " \
"  ( measure.cost, [ending region].[Europe] ), " \
"  ( measure.MM001, [ending region].[Europe] ), " \
"  ( measure.MM001, [ending region].[Asia] ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"