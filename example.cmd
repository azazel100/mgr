set app=GraphLayout


rem generate some small graphs using method describen the the thesis
set graphs=jagmesh1 add32 data commanche_dual G67 barth barth5 finance256 finan512
for %%g in (%graphs%) do (
	GraphLayout graphs\%%g.mtx drawings /drawFinal 2000 springaprox  /SkipManyLevels /AproxNodeProperties /AproxNodePropertiesPersistent /AlternativeUpperLevelsRandom
	)

rem generate same graphs using spring exact method
for %%g in (%graphs%) do (
	GraphLayout graphs\%%g.mtx drawings /drawFinal 2000 springexact  /SkipManyLevels /AproxNodeProperties /AproxNodePropertiesPersistent /AlternativeUpperLevelsRandom
	)



