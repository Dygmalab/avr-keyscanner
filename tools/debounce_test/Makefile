DEBOUNCERS := $(shell ls ../../firmware/debounce-*.h |cut -d \/ -f 4 |cut -d \. -f 1 )

test:
	perl run_tests.pl

all: debouncers

debouncers: $(DEBOUNCERS)

debounce-%:
	$(CC) debounce_test.c -include debounce_test.h -include ../../firmware/$(@).h -o $(@)

clean: 
	-rm $(DEBOUNCERS)
