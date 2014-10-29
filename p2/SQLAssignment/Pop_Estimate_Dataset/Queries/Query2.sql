SELECT 
	st_data.STATE, st_data.sum_st_births 
FROM 
	(
		SELECT
			STATE, SUM(BIRTHS2012) as sum_st_births, NAME
		FROM
			POP_ESTIMATE_NATION_STATE_PR
		WHERE
			STATE != 72
		GROUP BY
			STATE
		HAVING 
			STATE != 0
	) AS st_data
	--select a table with state, sum of birth and name from the table, which excluded the PR and State code == 0.
WHERE
	st_data.sum_st_births > 80000	
	--select the rows which sum of birth greater than 80000
;

/*
SELECT 
	STATE, NAME, BIRTHS2012
FROM 
	POP_ESTIMATE_NATION_STATE_PR
WHERE
	STATE = 0
*/
;
