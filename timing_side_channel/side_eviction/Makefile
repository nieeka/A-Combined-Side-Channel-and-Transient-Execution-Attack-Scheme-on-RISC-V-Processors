all: eviction-Thead eviction-other

eviction-Thead: hist.c
	gcc eviction.c -DC906 -Os -o eviction-Thead

eviction-other: hist.c
	gcc eviction.c -DU74 -Os -o eviction-other

clean:
	rm -f eviction-Thead eviction-other

