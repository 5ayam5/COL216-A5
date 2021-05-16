make
> summary
for folder in Tests/*
do
	cores=$(ls -1 $folder/*.asm | wc -l)
	echo $folder $cores cores
	./main $folder $cores 500 10 2 > $folder/out
	echo $folder >> summary
	python3.8 overview.py $folder/out
	echo >> summary
done
