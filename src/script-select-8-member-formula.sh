./euclid-cli \
"with " \
"member [ending region].CCCCCC as ([ending region].[Asia].[China]) * ([ending region].[America].[U.S]) / ([ending region].[Europe].[Greece]) + ([ending region].[Europe].[Italy]) " \
"member [ending region].AAAAAA as ([ending region].[Asia].[China]) * 1000000000 + ([ending region].[America].[U.S]) " \
"member [ending region].X as (([ending region].[Europe].[Greece]) * ((([ending region].[Asia].[China]) * ([ending region].[America].[U.S]) / ([ending region].[Europe].[Greece]) + ([ending region].[Europe].[Italy])) + 1000) - ([ending region].[Europe].[Italy])) / 0.5 " \
"member [ending region].XX as ((((((((((((((((((([ending region].[Asia]))))))))))))))))))) + ((([ending region].[Asia].[China]))) * 10000" \
"select " \
"{ " \
"  ( measure.cost, [ending region].[Asia].[China] ), " \
"  ( measure.cost, [ending region].[America].[U.S] ), " \
"  ( measure.cost, [ending region].[Europe].[Greece] ), " \
"  ( measure.cost, [ending region].[Europe].[Italy] ), " \
"  ( measure.cost, [ending region].CCCCCC ), " \
"  ( measure.cost, [ending region].AAAAAA ), " \
"  ( measure.cost, [ending region].X ), " \
"  ( measure.cost, [ending region].XX ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"