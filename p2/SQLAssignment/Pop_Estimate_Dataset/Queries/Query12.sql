SELECT 
	U.AGE,
	ABS(U.DIFF),
	CASE WHEN U.DIFF > 0 THEN "increased" WHEN U.DIFF = 0 THEN "same" WHEN U.DIFF < 0 THEN "decreased" END
FROM
	(
		SELECT 
			P.AGE AS AGE, 
			( SUM(P.POPESTIMATE2011) - SUM(P.POPESTIMATE2010) ) as DIFF
		FROM 
			POP_ESTIMATE_STATE_AGE_SEX_RACE_ORIGIN as P
		WHERE 
			P.SEX = 0 
			AND
			P.ORIGIN = 0
			AND
			P.STATE != 0
		GROUP BY
			P.AGE
	)as U
	--select AGE, and difference from the table which only counted the total sex and total origin, and excluded STATE code == 0.
;

/*
SELECT 
	state
FROM 
	POP_ESTIMATE_STATE_AGE_SEX_RACE_ORIGIN as P
WHERE 
	P.SEX = 0 
	AND
	P.ORIGIN = 0
	AND
	P.STATE != 0
GROUP BY 
	state
;
*/
