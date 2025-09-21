# Define as subpastas que possuem makefiles
SUBDIRS = hello simple_driver simple_driver_atividade1 simple_driver_atividade2 simple_driver_desafio sstf-iosched-skeleton-master

.PHONY: clean

clean:
	@for dir in $(SUBDIRS); do \
		echo "Limpando pasta $${dir}..."; \
		$(MAKE) -C $${dir} clean; \
	done