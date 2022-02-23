#!/bin/bash

./euclid-cli " insert [logistics.test] " \
" ( [Goods].[foods].[nut], Transport.[ocean freight], [starting region].[Asia].China, [ending region].Asia.[China], " \
" [starting date].[2019].Q3.M8, [completion date].[2021].[Q1].[M3] measures quantity 333.333 income 222.222 cost 111.111 ), " \
" ( Goods.foods.wine, Transport.[ocean freight], [starting region].[Asia].China, [ending region].Asia.[China], " \
" [starting date].[2019].Q3.M8, [completion date].[2021].[Q1].[M3] measures [income] 234 [cost] 123 ) "