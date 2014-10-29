SELECT 
	T.stname, COUNT(DISTINCT T.county)
FROM 	
	pop_estimate_cities_towns T
WHERE 
	county != 0
GROUP BY	 
	T.stname
;
--exclude the county code ==0. select the counties for each state

