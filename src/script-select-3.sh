./euclid-cli \
"select " \
"{ " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[quantity] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[income] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[cost] ), " \
"  ( [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[quantity] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12] ) " \
"} on 0, " \
"{ " \
"  ( [completion date].[2020].[Q1].[M1], [measure].quantity ), " \
"  ( [completion date].[2020].[Q1].[M1], [measure].income ), " \
"  ( [completion date].[2020].[Q1].[M1], [measure].cost ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3] ), " \
"  ( [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3], Goods.[household appliances].[television], Transport.[ocean freight] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight] ), " \
"  ( [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[income] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[cost] ) " \
"} on 1 " \
"from [logistics.test] "