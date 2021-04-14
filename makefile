all: cleaner detector

cleaner:
	cd data_cleaner; make all

detector:
	cd top-k; make all