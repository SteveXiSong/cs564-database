SELECT 
	NAME, NETMIG2012
FROM 
	POP_ESTIMATE_NATION_STATE_PR 
WHERE 	
	NETMIG2012 >0 
	AND STATE >0 
	AND STATE != 72
;	
	--exclude PR and State code == 0 , NETMIG greater than 0 means "migration into"
