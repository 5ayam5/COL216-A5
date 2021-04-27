make
for folder in Tests/*
do
	echo
	echo
	echo $folder:
	echo
	cores=$(ls $folder | wc -l)
	./main $folder $cores 500 10 2
done
