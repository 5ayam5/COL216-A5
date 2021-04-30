make
for folder in Tests/*
do
	cores=$(ls -1 $folder/*.asm | wc -l)
	echo $folder $cores cores
	./main $folder $cores 500 10 2 > $folder/out
done
