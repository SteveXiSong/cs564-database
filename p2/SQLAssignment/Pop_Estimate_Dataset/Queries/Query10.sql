SELECT 
	P.STNAME
	/*,P.sum_state_pop,
	 H.HUEST_2011,
	 CAST(P.sum_state_pop AS REAL)/CAST(H.HUEST_2011 AS REAL),
	 N.nation_ratio*/
FROM
	(
		SELECT 
			STNAME, SUM(POPESTIMATE2011) AS sum_state_pop
		FROM 
			POP_ESTIMATE_STATE_COUNTY
		WHERE 
			SUMLEV = 40
		GROUP BY 
			STATE	
	) AS P
	NATURAL JOIN
	(
		SELECT 
			STNAME, HUEST_2011
		FROM 
			HOUSING_UNITS_STATE_LEVEL
		GROUP BY
			STATE
	) AS H 
	--select each state's sum of population
	CROSS JOIN
	(
		SELECT  
			( CAST( P2.sum_nation_pop AS REAL)/CAST( H2.sum_nation_hu AS REAL) ) 
			AS nation_ratio
		FROM 
			(
				SELECT 
					STNAME, TOTAL(POPESTIMATE2011) as sum_nation_pop
				FROM 
					POP_ESTIMATE_STATE_COUNTY
				WHERE 
					SUMLEV = 40
			) AS P2
			NATURAL JOIN
			(
				SELECT 
					STNAME, TOTAL(HUEST_2011) as sum_nation_hu
				FROM 
					HOUSING_UNITS_STATE_LEVEL
			) AS H2
	)AS N
	-- select the national ration and join with the former selected table for each state
WHERE
	CAST(P.sum_state_pop AS REAL)/CAST(H.HUEST_2011 AS REAL)
	<
	N.nation_ratio
;

/*
SELECT "---------------------------------------"
;

SELECT 
	P.STNAME,
	P.sum_state_pop,
	H.HUEST_2011,
	CAST(P.sum_state_pop AS REAL)/CAST(H.HUEST_2011 AS REAL),
	N.nation_ratio
FROM
	(
		SELECT 
			STNAME, SUM(POPESTIMATE2011) AS sum_state_pop
		FROM 
			POP_ESTIMATE_STATE_COUNTY
		WHERE 
			SUMLEV = 40
		GROUP BY 
			STATE	
	) AS P
	NATURAL JOIN
	(
		SELECT 
			STNAME, HUEST_2011
		FROM 
			HOUSING_UNITS_STATE_LEVEL
		GROUP BY
			STATE
	) AS H 
	--select each state's sum of population
	CROSS JOIN
	(
		SELECT  
			( CAST( P2.sum_nation_pop AS REAL)/CAST( H2.sum_nation_hu AS REAL) ) 
			AS nation_ratio
		FROM 
			(
				SELECT 
					STNAME, TOTAL(POPESTIMATE2011) as sum_nation_pop
				FROM 
					POP_ESTIMATE_STATE_COUNTY
				WHERE 
					SUMLEV = 40
			) AS P2
			NATURAL JOIN
			(
				SELECT 
					STNAME, TOTAL(HUEST_2011) as sum_nation_hu
				FROM 
					HOUSING_UNITS_STATE_LEVEL
			) AS H2
	)AS N
	-- select the national ration and join with the former selected table for each state
WHERE
	CAST(P.sum_state_pop AS REAL)/CAST(H.HUEST_2011 AS REAL)
	>=
	N.nation_ratio
;
*/

