#SRC = Query1.sql Query2.sql Query3.sql
#RESULT = results1.tst results2.txt result3.txt
#
#all:$(SRC) $(RESULT)
#	mv $(RESULT) $(REULT).old
#	sqlite3 census.db < $(SRC) > $(RESULT)
#$(RESULT):
#	sqlite3 census.db < $(SRC) > $(RESULT)
ALLTARGETS = Q1 Q2 Q3

QUERY = Query1 Query2 Query3 Query4 Query5 
RESULT = results1 results2 results3

all: $(QUERY)
	echo "run all sql query"

Query1: results1.txt 
	mv $< $<.old
	sqlite3 census.db < $@.sql > $<
	less $<
results1.txt: 
	sqlite3 census.db < $@.sql > $@

Query2: results2.txt 
	mv $< $<.old
	sqlite3 census.db < $@.sql > $<
	less $<
results2.txt: 
	sqlite3 census.db < $< > $@

Query3: results3.txt
	mv $< $<.old
	sqlite3 census.db < $@.sql > $<
	less $<
results3.txt: Query3.sql
	sqlite3 census.db < $< > $@

Query4: results4.txt
	mv $< $<.old
	sqlite3 census.db < $@.sql > $<
	less $<
results4.txt: Query4.sql
	sqlite3 census.db < $< > $@

Query5: results5.txt
	mv $< $<.old
	sqlite3 census.db < $@.sql > $<
	less $<
results5.txt: Query5.sql
	sqlite3 census.db < $< > $@

Query6: results6.txt
	rm $< 
	sqlite3 census.db < $@.sql > $<
	less $<
results6.txt: Query6.sql
	sqlite3 census.db < $< > $@

Query7: results7.txt
	rm $< 
	sqlite3 census.db < $@.sql > $<
	less $<
results7.txt: Query7.sql
	sqlite3 census.db < $< > $@

Query8: results8.txt
	rm $< 
	sqlite3 census.db < $@.sql > $<
	less $<
results8.txt: Query8.sql
	sqlite3 census.db < $< > $@

Query9:
	rm results9.txt
	sqlite3 census.db < $@.sql > results9.txt
	less results9.txt

Query10:
	rm results10.txt
	sqlite3 census.db < $@.sql > results10.txt
	less results10.txt

Query11:
	rm results11.txt
	sqlite3 census.db < $@.sql > results11.txt
	less results11.txt

Query12:
	rm results12.txt
	sqlite3 census.db < $@.sql > results12.txt
	less results12.txt

clean: 
	rm -f *.txt 
